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
        : _succeed(false), _message(nullptr), _site(Lexer::FLocation::None()) {}

    TParseResult(T&& rvalue, const Lexer::FLocation& site)
        : _succeed(true), _value(std::move(rvalue)), _message(nullptr), _site(site) {
        Assert(not _site.Filename.empty());
    }

    TParseResult(const char *message, Lexer::FSymbol::ETypeId expected, const Lexer::FLocation& site)
        : _succeed(false), _message(message), _expected(expected), _site(site) {
        Assert(not _site.Filename.empty());
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
    Lexer::FSymbol::ETypeId Expected() const { Assert(!_succeed); return _expected; }

    const Lexer::FLocation& Site() const { return _site; }

    static TParseResult Success(T&& rvalue, const Lexer::FLocation& site) {
        return TParseResult(std::move(rvalue), site);
    }

    static TParseResult Success(const T& value, const Lexer::FLocation& site) {
        T rvalue(value);
        return TParseResult(std::move(rvalue), site);
    }

    static TParseResult Failure(const char *message, Lexer::FSymbol::ETypeId expected, const Lexer::FLocation& site) {
        return TParseResult(message, expected, site);
    }

    static TParseResult Unexpected(Lexer::FSymbol::ETypeId expected, const Lexer::FMatch *found, const FParseList& input) {
        return TParseResult("unexpected match", expected, found ? found->Site() : input.Site());
    }

private:
    bool _succeed;

    T _value;

    const char *_message;
    Lexer::FSymbol::ETypeId _expected;

    Lexer::FLocation _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
