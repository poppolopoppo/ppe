#include "stdafx.h"

#include "JSON.h"

#include "Lexer/Lexer.h"
#include "Lexer/Match.h"
#include "Lexer/Symbol.h"
#include "Lexer/Symbols.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/FS/ConstNames.h"
#include "Core/IO/Format.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Memory/MemoryProvider.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace JSON_ {
//----------------------------------------------------------------------------
static void EscapeString_(std::basic_ostream<char>& oss, const FJSON::FString& str) {
    Escape(oss, str.MakeView(), EEscape::Unicode);
}
//----------------------------------------------------------------------------
static bool ParseValue_(Lexer::FLexer& lexer, FJSON::FValue& value);
//----------------------------------------------------------------------------
static bool ParseObject_(Lexer::FLexer& lexer, FJSON::FValue& value) {
    FJSON::FObject& object = value.SetType(FJSON::Object).ToObject();

    Lexer::FMatch key;
    for (bool notFirst = false;; notFirst = true)
    {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        if (not lexer.Expect(key, Lexer::FSymbols::String))
            break;

        if (not lexer.Expect(Lexer::FSymbols::Colon))
            CORE_THROW_IT(FJSONException("missing comma", key.Site()));

        if (not ParseValue_(lexer, object[std::move(key.Value())]))
            CORE_THROW_IT(FJSONException("missing value", key.Site()));
    }

    return lexer.Expect(Lexer::FSymbols::RBrace);
}
//----------------------------------------------------------------------------
static bool ParseArray_(Lexer::FLexer& lexer, FJSON::FValue& value) {
    FJSON::FArray& arr = value.SetType(FJSON::Array).ToArray();

    FJSON::FValue item;
    for (bool notFirst = false;; notFirst = true)
    {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        if (ParseValue_(lexer, item))
            arr.emplace_back(std::move(item));
        else
            break;
    }

    return lexer.Expect(Lexer::FSymbols::RBracket);
}
//----------------------------------------------------------------------------
static bool ParseValue_(Lexer::FLexer& lexer, FJSON::FValue& value) {
    Lexer::FMatch read;
    if (not lexer.Read(read))
        return false;

    bool unexpectedToken = false;

    if (read.Symbol() == Lexer::FSymbols::LBrace) {
        return ParseObject_(lexer, value);
    }
    else if (read.Symbol() == Lexer::FSymbols::LBracket) {
        return ParseArray_(lexer, value);
    }
    else if (read.Symbol() == Lexer::FSymbols::String) {
        value.SetValue(std::move(read.Value()));
    }
#if 0 // digress from JSON std here : need integral literals
    else if (read.Symbol() == Lexer::FSymbols::Number) {
        if (not Atod(&value.SetType(FJSON::Number).ToNumber(), read.Value().MakeView()))
            CORE_THROW_IT(FJSONException("malformed int", read.Site()));
    }
    // TODO : handle negative numbers as bellow VVVVVVV
#else
    else if (read.Symbol() == Lexer::FSymbols::Int) {
        FJSON::FInteger* i = &value.SetType(FJSON::Integer).ToInteger();
        if (not Atoi64(i, read.Value().MakeView(), 10))
            CORE_THROW_IT(FJSONException("malformed int", read.Site()));
    }
    else if (read.Symbol() == Lexer::FSymbols::Float) {
        FJSON::FFloat* d = &value.SetType(FJSON::Float).ToFloat();
        if (not Atod(d, read.Value().MakeView()))
            CORE_THROW_IT(FJSONException("malformed float", read.Site()));
    }
    // unary minus ?
    else if (read.Symbol() == Lexer::FSymbols::Sub) {
        const Lexer::FMatch* const peek = lexer.Peek();

        unexpectedToken = true;

        if (peek && peek->Symbol() == Lexer::FSymbols::Int) {
            if (ParseValue_(lexer, value)) {
                value.ToInteger() = -value.ToInteger();
                unexpectedToken = false;
            }
        }
        else if (peek && peek->Symbol() == Lexer::FSymbols::Float) {
            if (ParseValue_(lexer, value)) {
                value.ToFloat() = -value.ToFloat();
                unexpectedToken = false;
            }
        }
    }
#endif
    else if (read.Symbol() == Lexer::FSymbols::True) {
        value.SetValue(true);
    }
    else if (read.Symbol() == Lexer::FSymbols::False) {
        value.SetValue(false);
    }
    else if (read.Symbol() == Lexer::FSymbols::Null) {
        value.SetType(FJSON::Null);
    }
    else {
        unexpectedToken = true;
    }

    if (unexpectedToken)
        CORE_THROW_IT(FJSONException("unexpected token", read.Site()));

    return true;
}
//----------------------------------------------------------------------------
static void ToStream_(const FJSON::FValue& value, std::basic_ostream<char>& oss, Fmt::FIndent& indent, bool minify) {
    switch (value.Type()) {

    case Core::Serialize::FJSON::Null:
        oss << "null";
        break;

    case Core::Serialize::FJSON::Bool:
        oss << (value.ToBool() ? "true" : "false");
        break;

#if 0 // digress from JSON std here : need integral literals
    case Core::Serialize::FJSON::Number:
        oss << value.ToNumber();
        break;
#else
    case Core::Serialize::FJSON::Integer:
        oss << value.ToInteger();
        break;
    case Core::Serialize::FJSON::Float:
        oss << value.ToFloat();
        break;
#endif

    case Core::Serialize::FJSON::String:
        oss << '"';
        JSON_::EscapeString_(oss, value.ToString());
        oss << '"';
        break;

    case Core::Serialize::FJSON::Array:
        if (not value.ToArray().empty()) {
            const auto& arr = value.ToArray();
            oss << '[';
            if (not minify)
                oss << eol;
            {
                const Fmt::FIndent::FScope scopeIndent(indent);

                size_t n = arr.size();
                for (const FJSON::FValue& item : arr) {
                    oss << indent;
                    ToStream_(item, oss, indent, minify);
                    if (--n)
                        oss << ',';
                    if (not minify)
                        oss << eol;
                }
            }
            oss << indent << ']';
        }
        else {
            oss << "[]";
        }
        break;

    case Core::Serialize::FJSON::Object:
        if (not value.ToObject().empty()) {
            const auto& obj = value.ToObject();
            oss << '{';
            if (not minify)
                oss << eol;
            {
                const Fmt::FIndent::FScope scopeIndent(indent);

                size_t n = obj.size();
                for (const auto& member : obj) {
                    oss << indent << '"';
                    JSON_::EscapeString_(oss, member.first);
                    oss << "\": ";
                    ToStream_(member.second, oss, indent, minify);
                    if (--n)
                        oss << ',';
                    if (not minify)
                        oss << eol;
                }
            }
            oss << indent << '}';
        }
        else {
            oss << "{}";
        }
        break;

    default:
        AssertNotImplemented();
        break;
    }
}
//----------------------------------------------------------------------------
} //!namespace JSON_
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FJSON::FValue::FValue(EType type)
    : _type(type) {
    switch (_type) {
    case Core::Serialize::FJSON::Null:
        break;
    case Core::Serialize::FJSON::Bool:
        new (&_bool) FBool(false);
        break;
#if 0 // digress from JSON std here : need integral literals
    case Core::Serialize::FJSON::Number:
        new (&_number) FNumber(0);
        break;
#else
    case Core::Serialize::FJSON::Integer:
        new (&_integer) FInteger(0);
        break;
    case Core::Serialize::FJSON::Float:
        new (&_float) FFloat(0);
        break;
#endif
    case Core::Serialize::FJSON::String:
        new (&_string) FString();
        break;
    case Core::Serialize::FJSON::Array:
        new (&_array) FArray();
        break;
    case Core::Serialize::FJSON::Object:
        new (&_object) FObject();
        break;
    default:
        AssertNotImplemented();
        break;
    }
}
//----------------------------------------------------------------------------
FJSON::FValue& FJSON::FValue::operator =(const FValue& other) {
    Clear();

    _type = other._type;
    switch (other._type) {
    case Core::Serialize::FJSON::Null:
        break;
    case Core::Serialize::FJSON::Bool:
        _bool = other._bool;
        break;
#if 0 // digress from JSON std here : need integral literals
    case Core::Serialize::FJSON::Number:
        _number = other._number;
        break;
#else
    case Core::Serialize::FJSON::Integer:
        _integer = other._integer;
        break;
    case Core::Serialize::FJSON::Float:
        _float = other._float;
        break;
#endif
    case Core::Serialize::FJSON::String:
        new (&_string) FString(other._string);
        break;
    case Core::Serialize::FJSON::Array:
        new (&_array) FArray(other._array);
        break;
    case Core::Serialize::FJSON::Object:
        new (&_object) FObject(other._object);
        break;
    default:
        AssertNotImplemented();
        break;
    }

    return (*this);
}
//----------------------------------------------------------------------------
FJSON::FValue& FJSON::FValue::operator =(FValue&& rvalue) {
    Clear();

    switch (rvalue._type) {
    case Core::Serialize::FJSON::Null:
        Assert(Null == _type);
        return (*this);
    case Core::Serialize::FJSON::Bool:
        _bool = rvalue._bool;
        break;
#if 0 // digress from JSON std here : need integral literals
    case Core::Serialize::FJSON::Number:
        _number = rvalue._number;
        break;
#else
    case Core::Serialize::FJSON::Integer:
        _integer = rvalue._integer;
        break;
    case Core::Serialize::FJSON::Float:
        _float = rvalue._float;
        break;
#endif
    case Core::Serialize::FJSON::String:
        new (&_string) FString(std::move(rvalue._string));
        rvalue._string.~FString();
        break;
    case Core::Serialize::FJSON::Array:
        new (&_array) FArray(std::move(rvalue._array));
        rvalue._array.~FArray();
        break;
    case Core::Serialize::FJSON::Object:
        new (&_object) FObject(std::move(rvalue._object));
        rvalue._object.~FObject();
        break;
    default:
        AssertNotImplemented();
        break;
    }

    _type = rvalue._type;
    rvalue._type = Null;
    return (*this);
}
//----------------------------------------------------------------------------
FJSON::FValue& FJSON::FValue::SetType(EType type) {
    if (_type != FJSON::Null)
        Clear();

    _type = type;

    switch (type) {
    case Core::Serialize::FJSON::Null:
        break;
    case Core::Serialize::FJSON::Bool:
        new (&_bool) FBool();
        break;
#if 0 // digress from JSON std here : need integral literals
    case Core::Serialize::FJSON::Number:
        new (&_number) FNumber();
        break;
#else
    case Core::Serialize::FJSON::Integer:
        new (&_integer) FInteger();
        break;
    case Core::Serialize::FJSON::Float:
        new (&_float) FFloat();
        break;
#endif
    case Core::Serialize::FJSON::String:
        new (&_string) FString();
        break;
    case Core::Serialize::FJSON::Array:
        new (&_array) FArray();
        break;
    case Core::Serialize::FJSON::Object:
        new (&_object) FObject();
        break;
    default:
        AssertNotImplemented();
        break;
    }

    return (*this);
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FBool value) {
    Clear();
    _type = EType::Bool;
    new (&_bool) FBool(value);
}
//----------------------------------------------------------------------------
#if 0 // digress from JSON std here : need integral literals
void FJSON::FValue::SetValue(FNumber value) {
    if (NeedsDestructor_(_type))
        Clear();

    _type = EType::Number;
    new (&_number) FNumber(value);
}
#else
void FJSON::FValue::SetValue(FInteger value) {
    Clear();
    _type = EType::Integer;
    new (&_integer) FInteger(value);
}
void FJSON::FValue::SetValue(FFloat value) {
    Clear();
    _type = EType::Float;
    new (&_float) FFloat(value);
}
#endif
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FString&& value) {
    Clear();
    _type = EType::String;
    new (&_string) FString(std::move(value));
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FArray&& value) {
    Clear();
    _type = EType::Array;
    new (&_array) FArray(std::move(value));
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FObject&& value) {
    Clear();
    _type = EType::Object;
    new (&_object) FObject(std::move(value));
}
//----------------------------------------------------------------------------
bool FJSON::FValue::Equals(const FValue& other) const {
    if (other._type != _type)
        return false;

    switch (_type)
    {
    case Core::Serialize::FJSON::Null:
        return true;
    case Core::Serialize::FJSON::Bool:
        return (_bool == other._bool);
    case Core::Serialize::FJSON::Integer:
        return (_integer == other._integer);
    case Core::Serialize::FJSON::Float:
        return (_float == other._float);
    case Core::Serialize::FJSON::String:
        return (_string == other._string);
    case Core::Serialize::FJSON::Array:
        return (_array == other._array);
    case Core::Serialize::FJSON::Object:
        return (_object == other._object);
    default:
        break;
    }

    AssertNotImplemented();
    return false;
}
//----------------------------------------------------------------------------
void FJSON::FValue::Clear() {
    switch (_type) {
    case Core::Serialize::FJSON::Null:
        return;
    case Core::Serialize::FJSON::Bool:
#if 0 // digress from JSON std here : need integral literals
    case Core::Serialize::FJSON::Number:
        break;
#else
    case Core::Serialize::FJSON::Integer:
    case Core::Serialize::FJSON::Float:
        break;
#endif
    case Core::Serialize::FJSON::String:
        _string.~FString();
        break;
    case Core::Serialize::FJSON::Array:
        _array.~FArray();
        break;
    case Core::Serialize::FJSON::Object:
        _object.~FObject();
        break;
    default:
        AssertNotImplemented();
        break;
    }

    _type = Null;
}
//----------------------------------------------------------------------------
void FJSON::FValue::ToStream(std::basic_ostream<char>& oss, bool minify/* = true */) const {
    Fmt::FIndent indent = (minify ? Fmt::FIndent::None() : Fmt::FIndent::TwoSpaces());
    oss << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
        << std::defaultfloat << std::setw(0);
    JSON_::ToStream_(*this, oss, indent, minify);
    Assert(0 == indent.Level);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FJSON::Load(FJSON* json, const FFilename& filename) {
    Assert(json);
    Assert(not filename.empty());

    EAccessPolicy policy = EAccessPolicy::Binary;
    if (filename.Extname() == FFSConstNames::Jsonz())
        policy = policy + EAccessPolicy::Compress;

    RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) content;
    if (not VFS_ReadAll(&content, filename, policy))
        return false;

    return Load(json, filename, content.MakeConstView().Cast<const char>());
}
//----------------------------------------------------------------------------
bool FJSON::Load(FJSON* json, const FFilename& filename, const FStringView& content) {
    FMemoryViewReader reader(content.Cast<const u8>());
    return Load(json, filename, &reader);
}
//----------------------------------------------------------------------------
bool FJSON::Load(FJSON* json, const FFilename& filename, IBufferedStreamReader* input) {
    Assert(json);
    Assert(input);

    const FWString filenameStr(filename.ToWString());
    Lexer::FLexer lexer(input, filenameStr.MakeView(), false);

    json->_root = FValue();

    CORE_TRY{
        if (not JSON_::ParseValue_(lexer, json->_root))
            return false;
    }
    CORE_CATCH(Lexer::FLexerException e)
    CORE_CATCH_BLOCK({
        CORE_THROW_IT(FJSONException(e.what(), e.Match().Site()));
    })

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
