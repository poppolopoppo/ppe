#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Container/Tuple.h"

#include "Core.Serialize/Parser/ParseList.h"
#include "Core.Serialize/Parser/ParseResult.h"

#include <functional>
#include <type_traits>

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using Enumerable = VECTOR_THREAD_LOCAL(Parser, T);
//----------------------------------------------------------------------------
template <typename T, typename _Allocator = THREAD_LOCAL_ALLOCATOR(Parser, int) >
struct Production : public std::unary_function<ParseList&, ParseResult<T> >, _Allocator {
    typedef T value_type;
    typedef std::function< ParseResult<T>(ParseList&) > lambda_type;

    lambda_type Lambda;
    Production(lambda_type&& lambda) {
        Lambda.assign(std::move(lambda), *this);
    }

    ParseResult<T> operator ()(ParseList& input) const {
        return Lambda(input);
    }

    ParseResult<T> TryParse(ParseList& input) const {
        const Lexer::Match *state = input.Peek();

        ParseResult<T> result{ Lambda(input) };
        if (!result.Succeed())
            input.Seek(state);

        return result;
    }

    T Parse(ParseList& input) const {
        ParseResult<T> result = Lambda(input);
        if (result.Succeed())
            return result.Value();

        throw ParserException(result.Message(), result.Site(), nullptr);
    }

    Production Ref() const {
        const Production *ref = this;
        return Production{[ref](ParseList& input) -> ParseResult<T> {
            return ref->Lambda(input);
        }};
    }

    static Production Return(T&& rvalue) {
        return Production{[rvalue](ParseList& input) -> ParseResult<T> {
            return ParseResult<T>.Success(rvalue, input.Site());
        }};
    }

    template <typename U>
    Production<U> Then(std::function< Production<U>(T&&) >&& then) const {
        const Production& first = *this;
        return Production<U>{[first, then](ParseList& input) -> ParseResult<U> {
            ParseResult<T> result = first(input);
            return (result.Succeed())
                ? then(std::move(result.Value()))
                : ParseResult<U>::Failure(result.Message(), result.Expected(), result.Site());
        }};
    }

    template <typename U>
    Production<U> Select(std::function< U(T&&) >&& convert) const {
        const Production& first = *this;
        return Production<U>{[first, convert](ParseList& input) -> ParseResult<U> {
            ParseResult<T> result = first(input);
            return (result.Succeed())
                ? ParseResult<U>::Success(convert(std::move(result.Value())), result.Site())
                : ParseResult<U>::Failure(result.Message(), result.Expected(), result.Site());
        }};
    }

    template <typename U>
    Production<U> Return(U&& rvalue) const {
        return Select([value = std::move(rvalue)](T&& ) -> U {
            return value;
        });
    }

    Production< Enumerable<T> > Many() const {
        const Production& parser = *this;
        return Production< Enumerable<T> >{[parser](ParseList& input) -> ParseResult< Enumerable<T> > {
            const Lexer::Match *offset = input.Peek();

            Enumerable<T> many;
            ParseResult<T> result;
            while ((result = parser.TryParse(input)).Succeed())
                many.push_back(std::move(result.Value()));

            return ParseResult< Enumerable<T> >::Success(std::move(many), offset ? offset->Site() : input.Site());
        }};
    }

    Production< Enumerable<T> > AtLeastOnce() const {
        const Production& parser = *this;
        return Production< Enumerable<T> >{[parser](ParseList& input) -> ParseResult< Enumerable<T> > {
            const Lexer::Match *offset = input.Peek();

            Enumerable<T> many;
            ParseResult<T> result;
            while ((result = parser.TryParse(input)).Succeed())
                many.push_back(std::move(result.Value()));

            return (many.size())
                ? ParseResult< Enumerable<T> >::Success(std::move(many), offset ? offset->Site() : input.Site())
                : ParseResult< Enumerable<T> >::Failure("invalid match", Lexer::Symbol::Invalid, offset ? offset->Site() : input.Site());
        }};
    }

    Production Or(Production<T>&& other) const {
        const Production& first = *this;
        return Production<T>{[first, other](ParseList& input) -> ParseResult<T> {
            ParseResult<T> result = first.TryParse(input);
            return (result.Succeed())
                ? result
                : other(input);
        }};
    }

    template <typename U>
    Production< typename TupleMerger<T, U>::type > And(Production<U>&& other) const {
        typedef typename TupleMerger<T, U>::type tuple_type;
        const Production& first = *this;
        return Production<tuple_type>{[first, other](ParseList& input) -> ParseResult<tuple_type> {
            ParseResult<T> a = first.TryParse(input);
            if (!a.Succeed())
                return ParseResult<tuple_type>::Failure(a.Message(), a.Expected(), a.Site());

            ParseResult<U> b = other.TryParse(input);
            if (!b.Succeed())
                return ParseResult<tuple_type>::Failure(b.Message(), b.Expected(), b.Site());

            tuple_type merged = MergeTuple(std::move(a.Value()), std::move(b.Value()));

            return ParseResult<tuple_type>::Success(std::move(merged), a.Site());
        }};
    }

    Production Except(Production&& other) const {
        const Production& second = *this;
        return Production<T>{[other, second](ParseList& input) {
            ParseResult<T> result = other.TryParse(input);
            return (result.Succeed())
                ? ParseResult<T>::Failure("excepted parser succeeded", result.Site())
                : second(input);
        }};
    }
};
//----------------------------------------------------------------------------
inline Production<const Lexer::Match *> Expect(Lexer::Symbol::TypeId symbol, const char *message = nullptr) {
    return Production<const Lexer::Match *>{[symbol, message](ParseList& input) -> ParseResult<const Lexer::Match *> {
        const Lexer::Match *match = input.Read();

        return (match && match->Symbol()->Type() == symbol)
            ? ParseResult<const Lexer::Match *>::Success(match, match->Site())
            : ParseResult<const Lexer::Match *>::Unexpected(symbol, match, input);
    }};
}
//----------------------------------------------------------------------------
inline Production<const Lexer::Match *> ExpectMask(uint64_t symbolMask, const char *message = nullptr) {
    return Production<const Lexer::Match *>{[symbolMask, message](ParseList& input) -> ParseResult<const Lexer::Match *> {
        const Lexer::Match *match = input.Read();

        return (match && match->Symbol()->Type() & symbolMask)
            ? ParseResult<const Lexer::Match *>::Success(match, match->Site())
            : ParseResult<const Lexer::Match *>::Unexpected(Lexer::Symbol::TypeId(symbolMask), match, input);
    }};
}
//----------------------------------------------------------------------------
inline Production<const Lexer::Match *> Optional(Lexer::Symbol::TypeId symbol) {
    return Production<const Lexer::Match *>{[symbol](ParseList& input) -> ParseResult<const Lexer::Match *> {
        const Lexer::Match *match = input.Peek();
        if (!match || match->Symbol()->Type() != symbol)
            return ParseResult<const Lexer::Match *>::Success(nullptr, match ? match->Site() : input.Site());

        match = input.Read();
        Assert(match && match->Symbol()->Type() == symbol);

        return ParseResult<const Lexer::Match *>::Success(match, match->Site());
    }};
}
//----------------------------------------------------------------------------
inline Production<const Lexer::Match *> OptionalMask(uint64_t symbolMask) {
    return Production<const Lexer::Match *>{[symbolMask](ParseList& input) -> ParseResult<const Lexer::Match *> {
        const Lexer::Match *match = input.Peek();
        if (!match || 0 == (match->Symbol()->Type() & symbolMask) )
            return ParseResult<const Lexer::Match *>::Success(nullptr, match ? match->Site() : input.Site());

        match = input.Read();
        Assert(match && match->Symbol()->Type() & symbolMask);

        return ParseResult<const Lexer::Match *>::Success(match, match->Site());
    }};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
