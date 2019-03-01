#pragma once

#include "Serialize.h"

#include "Parser/Parser.h"
#include "Parser/ParseList.h"
#include "Parser/ParseResult.h"

#include "Allocator/SingletonPoolAllocator.h"
#include "Container/Tuple.h"
#include "Container/TupleHelpers.h"
#include "Container/Vector.h"
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
POOL_TAG_FWD(Parser);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TEnumerable = VECTORINSITU(Parser, T, 4);
//----------------------------------------------------------------------------
static CONSTEXPR size_t GProductionInSituSize = sizeof(intptr_t);
//----------------------------------------------------------------------------
template <typename T>
class TProduction {
public:
    typedef T value_type;
    typedef TFunction< TParseResult<T>(FParseList&), GProductionInSituSize > lambda_type;

private:
    lambda_type _lambda;

public:
    CONSTEXPR explicit TProduction(lambda_type&& lambda) NOEXCEPT
    :   _lambda(std::move(lambda))
    {}

    CONSTEXPR TProduction(const TProduction&) NOEXCEPT = default;
    CONSTEXPR TProduction& operator =(const TProduction&) NOEXCEPT = default;

    CONSTEXPR TProduction(TProduction&&) NOEXCEPT = default;
    CONSTEXPR TProduction& operator =(TProduction&&) NOEXCEPT = default;

    TParseResult<T> operator ()(FParseList& input) const {
        return _lambda(input);
    }

    TParseResult<T> Invoke(FParseList& input) const {
        return _lambda(input);
    }

    TParseResult<T> TryParse(FParseList& input) const {
        const Lexer::FMatch *state = input.Peek();

        TParseResult<T> result{ _lambda(input) };
        if (not result.Succeed())
            input.Seek(state);

        return result;
    }

    T Parse(FParseList& input) const {
        TParseResult<T> result = _lambda(input);
        if (result.Succeed())
            return result.Value();

        PPE_THROW_IT(FParserException(result.Message(), result.Site(), nullptr));
    }

    TProduction Ref() const {
        const TProduction *ref = this;
        return TProduction{ [ref](FParseList& input) -> TParseResult<T> {
            return ref->Invoke(input);
        } };
    }

    static TProduction Return(T&& rvalue) {
        return TProduction{ { Meta::ForceInit, [value{ std::move(rvalue) }](FParseList& input) -> TParseResult<T> {
            return TParseResult<T>.Success(value, input.Site());
        }} };
    }

    template <typename U>
    TProduction<U> Then(TFunction< TProduction<U>(T&&) >&& rthen) const {
        return TProduction<U>{ { Meta::ForceInit, [first{ *this }, then{ std::move(rthen) }](FParseList& input) -> TParseResult<U> {
            TParseResult<T> result = first(input);
            return (result.Succeed())
                ? then(std::move(result.Value()))
                : TParseResult<U>::Failure(result.Message(), result.Expected(), result.Site());
        }}};
    }

    template <typename U>
    TProduction<U> Select(TFunction< U(T&&) >&& rconvert) const {
        return TProduction<U>{ { Meta::ForceInit, [first{ *this }, convert{ std::move(rconvert) }](FParseList& input) -> TParseResult<U> {
            TParseResult<T> result = first(input);
            return (result.Succeed())
                ? TParseResult<U>::Success(convert(std::move(result.Value())), result.Site())
                : TParseResult<U>::Failure(result.Message(), result.Expected(), result.Site());
        }}};
    }

    TProduction< TEnumerable<T> > Many() const {
        return TProduction< TEnumerable<T> >{ { Meta::ForceInit, [parser{ *this }](FParseList& input) -> TParseResult< TEnumerable<T> > {
            const Lexer::FMatch* const offset = input.Peek();

            TEnumerable<T> many;
            TParseResult<T> result;
            while ((result = parser.TryParse(input)).Succeed())
                many.push_back(std::move(result.Value()));

            return TParseResult< TEnumerable<T> >::Success(std::move(many), offset ? offset->Site() : input.Site());
        }}};
    }

    TProduction< TEnumerable<T> > AtLeastOnce() const {
        return TProduction< TEnumerable<T> >{ { Meta::ForceInit, [parser{ *this }](FParseList& input)->TParseResult< TEnumerable<T> > {
            const Lexer::FMatch* const offset = input.Peek();

            TEnumerable<T> many;
            TParseResult<T> result;
            while ((result = parser.TryParse(input)).Succeed())
                many.push_back(std::move(result.Value()));

            return (many.size())
                ? TParseResult< TEnumerable<T> >::Success(std::move(many), offset ? offset->Site() : input.Site())
                : TParseResult< TEnumerable<T> >::Failure("invalid match", Lexer::FSymbol::Invalid, offset ? offset->Site() : input.Site());
        }}};
    }

    TProduction< TEnumerable<T> > Join(Lexer::FSymbol::ETypeId symbol) const {
        return TProduction < TEnumerable<T> >{ { Meta::ForceInit, [parser{ *this }, symbol](FParseList& input) {
            const Lexer::FMatch* const offset = input.Peek();

            auto delimiter = Parser::Expect(symbol);

            TEnumerable<T> join;
            TParseResult<T> result;
            while ((result = parser.TryParse(input)).Succeed()) {
                join.emplace_back(std::move(result.Value()));

                if (not delimiter.TryParse(input).Succeed())
                    break;
            }

            return TParseResult< TEnumerable<T> >::Success(std::move(join), offset ? offset->Site() : input.Site());
        }}};
    }

    TProduction Or(TProduction<T>&& rother) const {
        return TProduction{ { Meta::ForceInit,[first{ *this }, other{ std::move(rother) }](FParseList& input)->TParseResult<T> {
            TParseResult<T> result = first.TryParse(input);
            if (not result.Succeed())
                result = other(input);
            return result;
        }} };
    }

    template <typename U>
    auto Optional(TProduction<U>&& rother) const {
        using tuple_type = decltype(MergeTuple(std::declval<T>(), Meta::MakeOptional(std::declval<U>())));
        return TProduction<tuple_type>{ { Meta::ForceInit, [first{ *this }, other{ std::move(rother) }](FParseList& input)->TParseResult<tuple_type> {
            TParseResult<T> a = first.TryParse(input);
            if (not a.Succeed())
                return TParseResult<tuple_type>::Failure(a.Message(), a.Expected(), a.Site());

            TParseResult<U> b = other.TryParse(input);

            Meta::TOptional<U> opt;
            if (b.Succeed())
                opt = Meta::MakeOptional(std::move(b.Value()));

            tuple_type merged{ MergeTuple(std::move(a.Value()), std::move(opt)) };

            return TParseResult<tuple_type>::Success(std::move(merged), a.Site());
        }}};
    }

    template <typename U>
    auto And(TProduction<U>&& rother) const {
        using tuple_type = decltype(MergeTuple(std::declval<T>(), std::declval<U>()));
        return TProduction<tuple_type>{ { Meta::ForceInit, [first{ *this }, other{ std::move(rother) }](FParseList& input)->TParseResult<tuple_type> {
            TParseResult<T> a = first.TryParse(input);
            if (not a.Succeed())
                return TParseResult<tuple_type>::Failure(a.Message(), a.Expected(), a.Site());

            TParseResult<U> b = other.TryParse(input);
            if (not b.Succeed())
                return TParseResult<tuple_type>::Failure(b.Message(), b.Expected(), b.Site());

            tuple_type merged{ MergeTuple(std::move(a.Value()), std::move(b.Value())) };

            return TParseResult<tuple_type>::Success(std::move(merged), a.Site());
        }}};
    }

    TProduction Except(TProduction&& rother) const {
        return TProduction{ [other{ std::move(rother) }, second{ *this }](FParseList& input) {
            TParseResult<T> result = other.TryParse(input);
            return (result.Succeed())
                ? TParseResult<T>::Failure("excepted parser succeeded", result.Site())
                : second(input);
        } };
    }
};
//----------------------------------------------------------------------------
inline TProduction<const Lexer::FMatch *> Expect(Lexer::FSymbol::ETypeId symbol) {
    return TProduction<const Lexer::FMatch *>{
        [symbol](FParseList& input) -> TParseResult<const Lexer::FMatch *> {
            const Lexer::FMatch *match = input.Read();

            return (match && match->Symbol()->Type() == symbol)
                ? TParseResult<const Lexer::FMatch *>::Success(match, match->Site())
                : TParseResult<const Lexer::FMatch *>::Unexpected(symbol, match, input);
        }};
}
//----------------------------------------------------------------------------
inline TProduction<const Lexer::FMatch *> ExpectMask(uint64_t symbolMask) {
    return TProduction<const Lexer::FMatch *>{
        [symbolMask](FParseList& input) -> TParseResult<const Lexer::FMatch *> {
            const Lexer::FMatch *match = input.Read();

            return (match && match->Symbol()->Type() & symbolMask)
                ? TParseResult<const Lexer::FMatch *>::Success(match, match->Site())
                : TParseResult<const Lexer::FMatch *>::Unexpected(Lexer::FSymbol::ETypeId(symbolMask), match, input);
        }};
}
//----------------------------------------------------------------------------
inline TProduction<const Lexer::FMatch *> Optional(Lexer::FSymbol::ETypeId symbol) {
    return TProduction<const Lexer::FMatch *>{
        [symbol](FParseList& input) -> TParseResult<const Lexer::FMatch *> {
            const Lexer::FMatch *match = input.Peek();
            if (not match || match->Symbol()->Type() != symbol)
                return TParseResult<const Lexer::FMatch *>::Success(nullptr, match ? match->Site() : input.Site());

            Assert(match->Symbol()->Type() == symbol);
            const Lexer::FMatch *consumed = input.Read(); // consumes match
            Assert(consumed == match);
            UNUSED(consumed);

            return TParseResult<const Lexer::FMatch *>::Success(match, match->Site());
        }};
}
//----------------------------------------------------------------------------
inline TProduction<const Lexer::FMatch *> OptionalMask(uint64_t symbolMask) {
    return TProduction<const Lexer::FMatch *>{
        [symbolMask](FParseList& input) -> TParseResult<const Lexer::FMatch *> {
            const Lexer::FMatch *match = input.Peek();
            if (not match || 0 == (match->Symbol()->Type() & symbolMask))
                return TParseResult<const Lexer::FMatch *>::Success(nullptr, match ? match->Site() : input.Site());

            Assert(match->Symbol()->Type() & symbolMask);
            const Lexer::FMatch *consumed = input.Read(); // consumes match
            Assert(consumed == match);
            UNUSED(consumed);

            return TParseResult<const Lexer::FMatch *>::Success(match, match->Site());
        }};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
