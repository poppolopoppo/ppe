#pragma once

#include "Core/Core.h"

#include "Core/Parser/ParseList.h"

#include "Core/Lexer/Location.h"
#include "Core/Lexer/Symbol.h"

#include <functional>

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class ParseResult {
public:
    typedef T value_type;

    ParseResult()
        : _succeed(false), _message(nullptr), _site(Lexer::Location::None()) {}

    ParseResult(T&& rvalue, const Lexer::Location& site)
        : _succeed(true), _message(nullptr), _value(std::move(rvalue)), _site(site) {
        Assert(_site.FileName);
    }

    ParseResult(const char *message, Lexer::Symbol::TypeId expected, const Lexer::Location& site)
        : _succeed(false), _message(message), _expected(expected), _site(site) {
        Assert(_site.FileName);
    }

    ~ParseResult() {}

    ParseResult(ParseResult&& rvalue)
        :   _succeed(std::move(rvalue._succeed))
        ,   _value(std::move(rvalue._value))
        ,   _message(std::move(rvalue._message))
        ,   _expected(std::move(rvalue._expected))
        ,   _site(std::move(rvalue._site)) {}

    ParseResult& operator =(ParseResult&& rvalue) {
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
    Lexer::Symbol::TypeId Expected() const { Assert(!_succeed); return _expected; }

    const Lexer::Location& Site() const { return _site; }

    static ParseResult Success(T&& rvalue, const Lexer::Location& site) {
        return ParseResult(std::move(rvalue), site);
    }

    static ParseResult Success(const T& value, const Lexer::Location& site) {
        T rvalue(value);
        return ParseResult(std::move(rvalue), site);
    }

    static ParseResult Failure(const char *message, Lexer::Symbol::TypeId expected, const Lexer::Location& site) {
        return ParseResult(message, expected, site);
    }

    static ParseResult Unexpected(Lexer::Symbol::TypeId expected, const Lexer::Match *found, const ParseList& input) {
        return ParseResult("unexpected match", expected, found ? found->Site() : input.Site());
    }

private:
    bool _succeed;

    T _value;

    const char *_message;
    Lexer::Symbol::TypeId _expected;

    Lexer::Location _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
