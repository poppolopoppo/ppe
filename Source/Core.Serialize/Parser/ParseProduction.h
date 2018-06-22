#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Parser/Parser.h"
#include "Core.Serialize/Parser/ParseList.h"
#include "Core.Serialize/Parser/ParseResult.h"

#include "Core/Allocator/SingletonPoolAllocator.h"
#include "Core/Container/Tuple.h"
#include "Core/Container/Vector.h"
#include "Core/Misc/Function.h"

#include <functional>
#include <type_traits>

namespace Core {
namespace Parser {
POOL_TAG_FWD(Parser);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TEnumerable = VECTORINSITU(Parser, T, 4);
//----------------------------------------------------------------------------
template <typename T>
struct TProduction {
    typedef T value_type;

    typedef TFunction< TParseResult<T>(FParseList&) > lambda_type;

    lambda_type Lambda;

    explicit TProduction(lambda_type&& lambda)
    :   Lambda(std::move(lambda))
    {}
    explicit TProduction(const lambda_type& lambda)
    :   Lambda(lambda)
    {}

    TProduction(const TProduction& other) { operator =(other); }
    TProduction& operator =(const TProduction& other) { Lambda = other.Lambda; return *this; }

    TProduction(TProduction&& rvalue) : Lambda(std::move(rvalue.Lambda)) {}
    TProduction& operator =(TProduction&& rvalue) { Lambda = std::move(rvalue.Lambda); return *this; }

    TParseResult<T> operator ()(FParseList& input) const {
        return Lambda(input);
    }

    TParseResult<T> TryParse(FParseList& input) const {
        const Lexer::FMatch *state = input.Peek();

        TParseResult<T> result{ Lambda(input) };
        if (!result.Succeed())
            input.Seek(state);

        return result;
    }

    T Parse(FParseList& input) const {
        TParseResult<T> result = Lambda(input);
        if (result.Succeed())
            return result.Value();

        CORE_THROW_IT(FParserException(result.Message(), result.Site(), nullptr));
    }

    TProduction Ref() const {
        const TProduction *ref = this;
        return TProduction{[ref](FParseList& input) -> TParseResult<T> {
            return ref->Lambda(input);
        }};
    }

    static TProduction Return(T&& rvalue) {
        return TProduction{[rvalue](FParseList& input) -> TParseResult<T> {
            return TParseResult<T>.Success(rvalue, input.Site());
        }};
    }

    template <typename U>
    TProduction<U> Then(TFunction< TProduction<U>(T&&) >&& then) const {
        const TProduction& first = *this;
        return TProduction<U>{[first, then](FParseList& input) -> TParseResult<U> {
            TParseResult<T> result = first(input);
            return (result.Succeed())
                ? then(std::move(result.Value()))
                : TParseResult<U>::Failure(result.Message(), result.Expected(), result.Site());
        }};
    }

    template <typename U>
    TProduction<U> Select(TFunction< U(T&&) >&& convert) const {
        const TProduction& first = *this;
        return TProduction<U>{[first, convert](FParseList& input) -> TParseResult<U> {
            TParseResult<T> result = first(input);
            return (result.Succeed())
                ? TParseResult<U>::Success(convert(std::move(result.Value())), result.Site())
                : TParseResult<U>::Failure(result.Message(), result.Expected(), result.Site());
        }};
    }

    template <typename U>
    TProduction<U> Return(U&& rvalue) const {
        return Select([value = std::move(rvalue)](T&& ) -> U {
            return value;
        });
    }

    TProduction< TEnumerable<T> > Many() const {
        const TProduction& parser = *this;
        return TProduction< TEnumerable<T> >{[parser](FParseList& input) -> TParseResult< TEnumerable<T> > {
            const Lexer::FMatch *offset = input.Peek();

            TEnumerable<T> many;
            TParseResult<T> result;
            while ((result = parser.TryParse(input)).Succeed())
                many.push_back(std::move(result.Value()));

            return TParseResult< TEnumerable<T> >::Success(std::move(many), offset ? offset->Site() : input.Site());
        }};
    }

    TProduction< TEnumerable<T> > AtLeastOnce() const {
        const TProduction& parser = *this;
        return TProduction< TEnumerable<T> >{[parser](FParseList& input) -> TParseResult< TEnumerable<T> > {
            const Lexer::FMatch *offset = input.Peek();

            TEnumerable<T> many;
            TParseResult<T> result;
            while ((result = parser.TryParse(input)).Succeed())
                many.push_back(std::move(result.Value()));

            return (many.size())
                ? TParseResult< TEnumerable<T> >::Success(std::move(many), offset ? offset->Site() : input.Site())
                : TParseResult< TEnumerable<T> >::Failure("invalid match", Lexer::FSymbol::Invalid, offset ? offset->Site() : input.Site());
        }};
    }

    TProduction Or(TProduction<T>&& other) const {
        const TProduction& first = *this;
        return TProduction<T>{[first, other](FParseList& input) -> TParseResult<T> {
            TParseResult<T> result = first.TryParse(input);
            if (!result.Succeed())
                result = other(input);
            return result;
        }};
    }

    template <typename U>
    TProduction< typename TTupleMerger<T, U>::type > And(TProduction<U>&& other) const {
        typedef typename TTupleMerger<T, U>::type tuple_type;
        const TProduction& first = *this;
        return TProduction<tuple_type>{[first, other](FParseList& input) -> TParseResult<tuple_type> {
            TParseResult<T> a = first.TryParse(input);
            if (!a.Succeed())
                return TParseResult<tuple_type>::Failure(a.Message(), a.Expected(), a.Site());

            TParseResult<U> b = other.TryParse(input);
            if (!b.Succeed())
                return TParseResult<tuple_type>::Failure(b.Message(), b.Expected(), b.Site());

            tuple_type merged = MergeTuple(std::move(a.Value()), std::move(b.Value()));

            return TParseResult<tuple_type>::Success(std::move(merged), a.Site());
        }};
    }

    TProduction Except(TProduction&& other) const {
        const TProduction& second = *this;
        return TProduction<T>{[other, second](FParseList& input) {
            TParseResult<T> result = other.TryParse(input);
            return (result.Succeed())
                ? TParseResult<T>::Failure("excepted parser succeeded", result.Site())
                : second(input);
        }};
    }
};
//----------------------------------------------------------------------------
inline TProduction<const Lexer::FMatch *> Expect(Lexer::FSymbol::ETypeId symbol, const char *message = nullptr) {
    return TProduction<const Lexer::FMatch *>{[symbol](FParseList& input) -> TParseResult<const Lexer::FMatch *> {
        const Lexer::FMatch *match = input.Read();

        return (match && match->Symbol()->Type() == symbol)
            ? TParseResult<const Lexer::FMatch *>::Success(match, match->Site())
            : TParseResult<const Lexer::FMatch *>::Unexpected(symbol, match, input);
    }};
}
//----------------------------------------------------------------------------
inline TProduction<const Lexer::FMatch *> ExpectMask(uint64_t symbolMask, const char *message = nullptr) {
    return TProduction<const Lexer::FMatch *>{[symbolMask](FParseList& input) -> TParseResult<const Lexer::FMatch *> {
        const Lexer::FMatch *match = input.Read();

        return (match && match->Symbol()->Type() & symbolMask)
            ? TParseResult<const Lexer::FMatch *>::Success(match, match->Site())
            : TParseResult<const Lexer::FMatch *>::Unexpected(Lexer::FSymbol::ETypeId(symbolMask), match, input);
    }};
}
//----------------------------------------------------------------------------
inline TProduction<const Lexer::FMatch *> Optional(Lexer::FSymbol::ETypeId symbol) {
    return TProduction<const Lexer::FMatch *>{[symbol](FParseList& input) -> TParseResult<const Lexer::FMatch *> {
        const Lexer::FMatch *match = input.Peek();
        if (!match || match->Symbol()->Type() != symbol)
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
    return TProduction<const Lexer::FMatch *>{[symbolMask](FParseList& input) -> TParseResult<const Lexer::FMatch *> {
        const Lexer::FMatch *match = input.Peek();
        if (!match || 0 == (match->Symbol()->Type() & symbolMask) )
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
} //!namespace Core
