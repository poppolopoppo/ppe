#pragma once

#include "Serialize.h"

#include "Parser/Parser.h"
#include "Parser/ParseList.h"
#include "Parser/ParseResult.h"

#include "Container/Tuple.h"
#include "Container/TupleHelpers.h"
#include "Container/Vector.h"
#include "Diagnostic/StackMarker.h"
#include "Misc/Function.h"
#include "Meta/Optional.h"

#include <functional>
#include <type_traits>

// /!\ Using TProduction<> everywhere is actually wrapping every static lambdas in
// a TFunction<> which will allocate when combining productions (TFunction<> won't fit in TFunction<>'s in situ)
// #TODO : a better implementation would use only TFunction<> to store the final expression, and use static native C++11 lambdas internally
// this could yield to much better optimization of this code

namespace PPE {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
#if 1 // in situ storage is bloating the stack
using TEnumerable = VECTORINSITU(Parser, T, 3);
#else
using TEnumerable = VECTOR(Parser, T);
#endif
//----------------------------------------------------------------------------
static CONSTEXPR size_t GProductionInSituSize = sizeof(intptr_t);
//----------------------------------------------------------------------------
template <typename T>
class TProduction {
public:
    typedef T value_type;
    typedef TFunction< FParseResult(FParseList&,T*), GProductionInSituSize > lambda_type;

private:
    lambda_type _lambda;

public:
    CONSTEXPR TProduction(lambda_type&& lambda) NOEXCEPT
    :   _lambda(std::move(lambda))
    {}

    TProduction(const TProduction&) = default;
    TProduction& operator =(const TProduction&) = default;

    TProduction(TProduction&&) = default;
    TProduction& operator =(TProduction&&) = default;

    FORCE_INLINE FParseResult operator ()(FParseList& input, T* result) const {
        return _lambda(input, result);
    }

    FParseResult Invoke(FParseList& input, T* result) const {
        return _lambda(input, result);
    }
    FParseResult TryParse(FParseList& input, T* result) const {
        const FParseMatch* offset = input.Peek();
        const FParseResult r = _lambda(input, result);

        if (not r)
            input.Seek(offset);

        return r;
    }

    T Parse(FParseList& input) const {
        T value;
        const FParseResult result = _lambda(input, &value);
        if (result)
            return value;

        input.Error(result);
    }

    CONSTEXPR TProduction Ref() const NOEXCEPT {
        return TProduction{ lambda_type::Bind<&TProduction::Invoke>(this) };
    }

    static TProduction Return(T&& rvalue) NOEXCEPT {
        return TProduction{ { Meta::ForceInit, [value{ std::move(rvalue) }](FParseList& input, T* result) -> FParseResult {
            PPE_STACKMARKER("return");

            *result = value;
            return FParseResult::Success(input.Site());
        }}};
    }

    template <typename U>
    CONSTEXPR TProduction<U> Then(TFunction< TProduction<U>(T*) >&& rthen) const NOEXCEPT {
        return TProduction<U>{ { Meta::ForceInit, [first{ *this }, then{ std::move(rthen) }](FParseList& input, T* result) -> FParseResult {
            PPE_STACKMARKER("then");

            const FParseResult r = first(input, result);
            return (r ? then(result) : r );
        }}};
    }

    template <typename U>
    CONSTEXPR TProduction<U> Select(TFunction< void(U*, const Lexer::FSpan&, T&&) >&& rconvert) const NOEXCEPT {
        return TProduction<U>{ { Meta::ForceInit, [first{ *this }, convert{ std::move(rconvert) }](FParseList& input, U* result) -> FParseResult {
            PPE_STACKMARKER("select");

            T tmp;
            const FParseResult r = first(input, &tmp);
            if (r)
                convert(result, r.Site, std::move(tmp));

            return r;
        }}};
    }

    CONSTEXPR TProduction< TEnumerable<T> > Many() const NOEXCEPT {
        return TProduction< TEnumerable<T> >{ { Meta::ForceInit, [parser{ *this }](FParseList& input, TEnumerable<T>* result) -> FParseResult {
            PPE_STACKMARKER("many");
            const FParseMatch* const offset = input.Peek();

            T tmp;
            for (;;) {
                const FParseResult r = parser.TryParse(input, &tmp);
                if (r)
                    result->push_back(std::move(tmp));
                else
                    break;;
            }

            return FParseResult::Success(offset
                ? Lexer::FSpan::FromSpan(offset->Site(), input.Site())
                : input.Site());
        }}};
    }

    CONSTEXPR TProduction< TEnumerable<T> > AtLeastOnce() const NOEXCEPT {
        return TProduction< TEnumerable<T> >{ { Meta::ForceInit, [parser{ *this }](FParseList& input, TEnumerable<T>* result) -> FParseResult {
            PPE_STACKMARKER("atleastonce");
            const FParseMatch* const offset = input.Peek();

            T tmp;
            FParseResult r;
            for (;;) {
                r = parser.TryParse(input, &tmp);
                if (r)
                    result->push_back(std::move(tmp));
                else
                    break;
            }

            if (result->empty())
                return r;
            else
                return FParseResult::Success(Lexer::FSpan::FromSpan(offset->Site(), input.Site()));
        }}};
    }

    template <typename U>
    CONSTEXPR TProduction< TEnumerable<T> > Join(TProduction<U>&& rdelimiter) const NOEXCEPT {
        return TProduction < TEnumerable<T> >{ { Meta::ForceInit, [parser{ *this }, delimiter{ std::move(rdelimiter) }]
        (FParseList& input, TEnumerable<T>* result) -> FParseResult {
            PPE_STACKMARKER("join");
            const FParseMatch* const offset = input.Peek();

            T tmp;
            const FParseMatch* sep = nullptr;
            for (;;) {
                FParseResult r = parser.TryParse(input, &tmp);
                if (r)
                    result->push_back(std::move(tmp));
                else
                    break;

                r = delimiter.TryParse(input, &sep);
                if (not r)
                    break;
            }

            return FParseResult::Success(offset
                ? Lexer::FSpan::FromSpan(offset->Site(), input.Site())
                : input.Site());
        }}};
    }

    template <typename U>
    CONSTEXPR auto Optional(TProduction<U>&& rother) const NOEXCEPT {
        using tuple_type = decltype(MergeTuple(std::declval<T>(), Meta::MakeOptional(std::declval<U>())));
        return TProduction<tuple_type>{ { Meta::ForceInit, [first{ *this }, other{ std::move(rother) }](FParseList& input, tuple_type* result) -> FParseResult {
            PPE_STACKMARKER("optional");

            FParseResult r = first.TryParse(input, &std::get<0>(*result));
            if (r) {
                U opt;
                r = r && other.TryParse(input, &opt);
                if (r)
                    std::get<1>(*result) = Meta::MakeOptional(std::move(opt));

                return FParseResult::Success(r.Site);
            }
            else {
                return r;
            }
        }}};
    }

    CONSTEXPR TProduction Except(TProduction&& rother) const NOEXCEPT {
        return TProduction{ [other{ std::move(rother) }, second{ *this }](FParseList& input, T* result) -> FParseResult {
            PPE_STACKMARKER("except");
            FParseResult r = other.TryParse(input, &result);
            if (r)
                return FParseResult::Failure(r.Site);
            else
                return second(input, result);
        }};
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename... _Args>
CONSTEXPR auto And(TProduction<_Args>&&... rexprs) NOEXCEPT -> TProduction<TTuple<_Args...>> {
    STATIC_ASSERT(sizeof...(_Args) >= 2);
    return { Meta::ForceInit, [exprs{ MakeTuple(std::move(rexprs)...) }]
        (FParseList& input, TTuple<_Args...>* value) -> FParseResult {
            PPE_STACKMARKER("and");
            FParseResult result = FParseResult::Success(input.Site());
            Meta::static_for<sizeof...(_Args)>([&](auto... idx) {
                return (... && (result = result && std::get<idx>(exprs)(input, &std::get<idx>(*value))).Succeed());
            });
            return result;
        }};
}
//----------------------------------------------------------------------------
template <typename... _Args>
CONSTEXPR auto Or(TProduction<_Args>&&... rexprs) NOEXCEPT -> TProduction<std::common_type_t<_Args...>> {
    STATIC_ASSERT(sizeof...(_Args) >= 2);
    return { Meta::ForceInit, [exprs{ MakeTuple(std::move(rexprs)...) }]
        (FParseList& input, std::common_type_t<_Args...>* value) -> FParseResult {
            PPE_STACKMARKER("or");
            FParseResult result;
            Meta::static_for<sizeof...(_Args)>([&](auto... idx) {
                return (... || (result = std::get<idx>(exprs).TryParse(input, value)).Succeed());
            });
            return result;
        }};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <Lexer::FSymbol::ETypeId _Symbol>
FParseResult Expect_(FParseList& input, const FParseMatch** result) {
    PPE_STACKMARKER("expect");
    return (input.Expect<_Symbol>(result)
        ? FParseResult::Success((*result)->Site())
        : FParseResult::Unexpected(_Symbol, *result, input) );
}
template <Lexer::FSymbol::ETypeId _Symbol>
FParseResult Optional_(FParseList& input, const FParseMatch** result) {
    PPE_STACKMARKER("optional");
    *result = input.Peek();
    if (not (*result) || not ((*result)->Symbol()->Type() ^ _Symbol)) {
        return FParseResult::Success((*result) ? (*result)->Site() : input.Site());
    }
    else {
        Verify(input.Read()); // consumes match
        return FParseResult::Success((*result)->Site());
    }
}
} //!details
//----------------------------------------------------------------------------
template <Lexer::FSymbol::ETypeId _Symbol>
TProduction<const FParseMatch *> Expect() NOEXCEPT {
    using lambda_type = typename TProduction<const FParseMatch *>::lambda_type;
    return TProduction<const FParseMatch *>{ lambda_type::template Bind<&details::Expect_<_Symbol>>() };
}
//----------------------------------------------------------------------------
template <Lexer::FSymbol::ETypeId _Symbol, typename T>
TProduction<T> Expect(T (*select)(const FParseMatch*)) NOEXCEPT {
    return TProduction<T>{ [select](FParseList& input, T* value) {
        PPE_STACKMARKER("expect_select");
        const FParseMatch* m = nullptr;
        if (input.Expect<_Symbol>(&m)) {
            *value = select(m);
            return FParseResult::Success(m->Site());
        }
        else {
            return FParseResult::Unexpected(_Symbol, m, input);
        }
    }};
}
//----------------------------------------------------------------------------
template <Lexer::FSymbol::ETypeId _Symbol>
TProduction<const FParseMatch *> Optional() NOEXCEPT {
    using lambda_type = typename TProduction<const FParseMatch *>::lambda_type;
    return TProduction<const FParseMatch *>{ lambda_type::template Bind<&details::Optional_<_Symbol>>() };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T>
using TSequenceResult_ = const FParseMatch*;
template <Lexer::FSymbol::ETypeId... _Symbols>
using TSequenceTuple_ = TTuple<TSequenceResult_<decltype(_Symbols)>...>;
template <Lexer::FSymbol::ETypeId... _Symbols>
FParseResult Sequence_(FParseList& input, TSequenceTuple_<_Symbols...>* value) {
    PPE_STACKMARKER("sequence");
    FParseResult result = FParseResult::Success(input.Site());
    Meta::static_for<sizeof...(_Symbols)>([&](auto... idx) {
        return (... && (result = result && Expect_<_Symbols>(input, &std::get<idx>(*value))).Succeed());
    });
    return result;
}
} //!details
//----------------------------------------------------------------------------
// Better using :
//  Parser::Sequence<Lexer::FSymbol::Identifier, Lexer::FSymbol::Is, Lexer::FSymbol::Number>
// Than :
//  Parser::And(
//      Parser::Expect<Lexer::FSymbol::Identifier>(),
//      Parser::Expect<Lexer::FSymbol::Is>(),
//      Parser::Expect<Lexer::FSymbol::Number> )
// Since the later will produce 4 TFunction<> instead of one for the first.
template <Lexer::FSymbol::ETypeId... _Symbols>
auto Sequence() NOEXCEPT {
    using tuple_type = details::TSequenceTuple_<_Symbols...>;
    using lambda_type = typename TProduction<tuple_type>::lambda_type;
    return TProduction<tuple_type>{ lambda_type::template Bind<&details::Sequence_<_Symbols...>>() };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// This helpers won't copy the production passed as arguments but will keep a ref on them instead.
// When available it's much cheaper than using Or() which will copy everything, more care is needed though
template <typename... T>
auto Switch(const TProduction<T>&... exprs) NOEXCEPT {
    using common_t = std::common_type_t<T...>;
    return TProduction<common_t>({ Meta::ForceInit,
        [&](FParseList& input, common_t* value) -> FParseResult {
            PPE_STACKMARKER("switch");

            FParseResult result;
            (... || (result = exprs.TryParse(input, value)).Succeed());
            return result;
        }});
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Better using :
//  Parser::BinaryOp<op>(expr)
// Than :
//  Parser::And(op.Many(), expr)
// Also this helper brings better error detection
//----------------------------------------------------------------------------
template <Lexer::FSymbol::ETypeId _Operator, typename T>
TProduction<T> UnaryOp(const TProduction<T>& operand, T(*make_op)(const FParseMatch*, const T&)) NOEXCEPT {
    return TProduction<T>{
        [&operand, make_op](FParseList& input, T* value) -> FParseResult {
            PPE_STACKMARKER("unaryop");

            TEnumerable<const FParseMatch*> ops;
            while (input.PeekType() ^ _Operator)
                ops.push_back(input.Read());

            if (Likely(ops.empty())) {
                return operand(input, value);
            }
            else {
                *value = operand.Parse(input);

                reverseforeachitem(op, ops)
                    *value = make_op(*op, *value);

                return FParseResult::Success(ops.front()->Site(), input.Site() );
            }
        }};
}
//----------------------------------------------------------------------------
template <Lexer::FSymbol::ETypeId _Operator, typename T>
TProduction<T> BinaryOp(const TProduction<T>& operand, T (*make_op)(const T&, const FParseMatch*, const T&)) NOEXCEPT {
    return TProduction<T>{
        [&operand, make_op](FParseList& input, T* value) -> FParseResult {
            PPE_STACKMARKER("binaryop");

            FParseResult result = operand(input, value);

            if (result) {
                while (input.PeekType() ^ _Operator) {
                    const FParseMatch* op = input.Read();
                    *value = make_op(*value, op, operand.Parse(input));
                }

                result = FParseResult::Success(result.Site, input.Site());
            }

            return result;
        }};
}
//----------------------------------------------------------------------------
template <Lexer::FSymbol::ETypeId _OpA, Lexer::FSymbol::ETypeId _OpB, typename T>
TProduction<T> TernaryOp(const TProduction<T>& operand, T(*make_op)(const T&, const FParseMatch*, const T&, const FParseMatch*, const T&)) NOEXCEPT {
    return TProduction<T>{
        [&operand, make_op](FParseList& input, T* value) -> FParseResult {
            PPE_STACKMARKER("ternaryop");

            FParseResult result = operand(input, value);

            if (result) {
                if (input.PeekType() ^ _OpA) {
                    const FParseMatch* opA = input.Read();
                    const T a = operand.Parse(input);
                    const FParseMatch* opB = nullptr;
                    if (not input.Expect<_OpB>(&opB))
                        input.Error("ill-formed ternary operator", input.Site());
                    const T b = operand.Parse(input);

                    *value = make_op(*value, opA, a, opB, b);
                }

                result = FParseResult::Success(result.Site, input.Site());
            }

            return result;
        }};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <Lexer::FSymbol::ETypeId _Open, Lexer::FSymbol::ETypeId _Close, typename T>
TProduction<T> Closure(const TProduction<T>& inner) NOEXCEPT {
    return TProduction<T>{
        [&inner](FParseList& input, T* value) -> FParseResult {
            PPE_STACKMARKER("closure");

            const FParseMatch* beg;
            if (input.Expect<_Open>(&beg)) {
                *value = inner.Parse(input);

                const FParseMatch* end;
                if (not input.Expect<_Close>(&end))
                    input.Error("missing closure closing token",
                        Lexer::FSpan::FromSpan(beg->Site(), input.Site()) );

                return FParseResult::Success(beg->Site(), end->Site());
            }
            else {
                return FParseResult::Failure(input.Site());
            }
        }};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
