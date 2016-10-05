#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Allocator/SingletonPoolAllocator.h"
#include "Core/Container/Tuple.h"

#include "Core.Serialize/Parser/Parser.h"
#include "Core.Serialize/Parser/ParseList.h"
#include "Core.Serialize/Parser/ParseResult.h"

#include <functional>
#include <type_traits>

namespace Core {
namespace Parser {
POOL_TAG_FWD(Parser);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TEnumerable = VECTOR_THREAD_LOCAL(Parser, T);
//----------------------------------------------------------------------------
template <typename T>
struct TProduction : public std::unary_function<FParseList&, TParseResult<T> > {
    typedef T value_type;

    typedef SINGLETON_POOL_ALLOCATOR(Parser, int, POOL_TAG(Parser)) allocator_type;
    typedef std::function< TParseResult<T>(FParseList&) > lambda_type;

    lambda_type Lambda;

    template <typename _Func>
    explicit TProduction(_Func&& lambda) {
        Lambda.assign(std::move(lambda), allocator_type());
    }

    TProduction(const TProduction& other) { operator =(other); }
    TProduction& operator =(const TProduction& other) { Lambda.assign(other.Lambda, allocator_type()); return *this; }

    TProduction(TProduction&& rvalue) : Lambda(std::move(rvalue.Lambda)) {}
    TProduction& operator =(TProduction&& rvalue) { Lambda = std::move(rvalue.Lambda); return *this; }

    TParseResult<T> operator ()(FParseList& input) const {
        return Lambda(input);
    }

    TParseResult<T> TryParse(FParseList& input) const {
        const FLexer::FMatch *state = input.Peek();

        TParseResult<T> result{ Lambda(input) };
        if (!result.Succeed())
            input.Seek(state);

        return result;
    }

    T Parse(FParseList& input) const {
        TParseResult<T> result = Lambda(input);
        if (result.Succeed())
            return result.Value();

        throw FParserException(result.Message(), result.Site(), nullptr);
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
    TProduction<U> Then(std::function< TProduction<U>(T&&) >&& then) const {
        const TProduction& first = *this;
        return TProduction<U>{[first, then](FParseList& input) -> TParseResult<U> {
            TParseResult<T> result = first(input);
            return (result.Succeed())
                ? then(std::move(result.Value()))
                : TParseResult<U>::Failure(result.Message(), result.Expected(), result.Site());
        }};
    }

    template <typename U>
    TProduction<U> Select(std::function< U(T&&) >&& convert) const {
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
            const FLexer::FMatch *offset = input.Peek();

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
            const FLexer::FMatch *offset = input.Peek();

            TEnumerable<T> many;
            TParseResult<T> result;
            while ((result = parser.TryParse(input)).Succeed())
                many.push_back(std::move(result.Value()));

            return (many.size())
                ? TParseResult< TEnumerable<T> >::Success(std::move(many), offset ? offset->Site() : input.Site())
                : TParseResult< TEnumerable<T> >::Failure("invalid match", FLexer::FSymbol::Invalid, offset ? offset->Site() : input.Site());
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
inline TProduction<const FLexer::FMatch *> Expect(FLexer::FSymbol::ETypeId symbol, const char *message = nullptr) {
    return TProduction<const FLexer::FMatch *>{[symbol, message](FParseList& input) -> TParseResult<const FLexer::FMatch *> {
        const FLexer::FMatch *match = input.Read();

        return (match && match->Symbol()->Type() == symbol)
            ? TParseResult<const FLexer::FMatch *>::Success(match, match->Site())
            : TParseResult<const FLexer::FMatch *>::Unexpected(symbol, match, input);
    }};
}
//----------------------------------------------------------------------------
inline TProduction<const FLexer::FMatch *> ExpectMask(uint64_t symbolMask, const char *message = nullptr) {
    return TProduction<const FLexer::FMatch *>{[symbolMask, message](FParseList& input) -> TParseResult<const FLexer::FMatch *> {
        const FLexer::FMatch *match = input.Read();

        return (match && match->Symbol()->Type() & symbolMask)
            ? TParseResult<const FLexer::FMatch *>::Success(match, match->Site())
            : TParseResult<const FLexer::FMatch *>::Unexpected(FLexer::FSymbol::ETypeId(symbolMask), match, input);
    }};
}
//----------------------------------------------------------------------------
inline TProduction<const FLexer::FMatch *> Optional(FLexer::FSymbol::ETypeId symbol) {
    return TProduction<const FLexer::FMatch *>{[symbol](FParseList& input) -> TParseResult<const FLexer::FMatch *> {
        const FLexer::FMatch *match = input.Peek();
        if (!match || match->Symbol()->Type() != symbol)
            return TParseResult<const FLexer::FMatch *>::Success(nullptr, match ? match->Site() : input.Site());

        Assert(match->Symbol()->Type() == symbol);
        const FLexer::FMatch *consumed = input.Read(); // consumes match
        Assert(consumed == match);
        UNUSED(consumed);

        return TParseResult<const FLexer::FMatch *>::Success(match, match->Site());
    }};
}
//----------------------------------------------------------------------------
inline TProduction<const FLexer::FMatch *> OptionalMask(uint64_t symbolMask) {
    return TProduction<const FLexer::FMatch *>{[symbolMask](FParseList& input) -> TParseResult<const FLexer::FMatch *> {
        const FLexer::FMatch *match = input.Peek();
        if (!match || 0 == (match->Symbol()->Type() & symbolMask) )
            return TParseResult<const FLexer::FMatch *>::Success(nullptr, match ? match->Site() : input.Site());

        Assert(match->Symbol()->Type() & symbolMask);
        const FLexer::FMatch *consumed = input.Read(); // consumes match
        Assert(consumed == match);
        UNUSED(consumed);

        return TParseResult<const FLexer::FMatch *>::Success(match, match->Site());
    }};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
