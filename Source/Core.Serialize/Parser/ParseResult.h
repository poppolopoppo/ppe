#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Parser/ParseList.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core.Serialize/Lexer/Symbol.h"

#include <functional>

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TParseResult {
public:
    typedef T value_type;

    TParseResult()
        : _succeed(false), _message(nullptr), _site(FLexer::FLocation::None()) {}

    TParseResult(T&& rvalue, const FLexer::FLocation& site)
        : _succeed(true), _value(std::move(rvalue)), _message(nullptr), _site(site) {
        Assert(_site.FileName);
    }

    TParseResult(const char *message, FLexer::FSymbol::ETypeId expected, const FLexer::FLocation& site)
        : _succeed(false), _message(message), _expected(expected), _site(site) {
        Assert(_site.FileName);
    }

    ~TParseResult() {}

    TParseResult(TParseResult&& rvalue)
        :   _succeed(std::move(rvalue._succeed))
        ,   _value(std::move(rvalue._value))
        ,   _message(std::move(rvalue._message))
        ,   _expected(std::move(rvalue._expected))
        ,   _site(std::move(rvalue._site)) {}

    TParseResult& operator =(TParseResult&& rvalue) {
        _succeed = std::move(rvalue._succeed);
        _value = std::move(rvalue._value);
        _message = std::move(rvalue._message);
        _expected = std::move(rvalue._expected);
        _site = std::move(rvalue._site);
        return *this;
    }

    bool Succeed() const { return _succeed; }

    T& Value() { Assert(_succeed); return _value; }
    const T& Value() const { Assert(_succeed); return _value; }

    const char *Message() const { Assert(!_succeed); return _message; }
    FLexer::FSymbol::ETypeId Expected() const { Assert(!_succeed); return _expected; }

    const FLexer::FLocation& Site() const { return _site; }

    static TParseResult Success(T&& rvalue, const FLexer::FLocation& site) {
        return TParseResult(std::move(rvalue), site);
    }

    static TParseResult Success(const T& value, const FLexer::FLocation& site) {
        T rvalue(value);
        return TParseResult(std::move(rvalue), site);
    }

    static TParseResult Failure(const char *message, FLexer::FSymbol::ETypeId expected, const FLexer::FLocation& site) {
        return TParseResult(message, expected, site);
    }

    static TParseResult Unexpected(FLexer::FSymbol::ETypeId expected, const FLexer::FMatch *found, const FParseList& input) {
        return TParseResult("unexpected match", expected, found ? found->Site() : input.Site());
    }

private:
    bool _succeed;

    T _value;

    const char *_message;
    FLexer::FSymbol::ETypeId _expected;

    FLexer::FLocation _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
