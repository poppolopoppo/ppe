#include "stdafx.h"

#include "Json.h"

#include "Lexer/Lexer.h"
#include "Lexer/Match.h"
#include "Lexer/Symbol.h"
#include "Lexer/Symbols.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/ConstNames.h"
#include "Core/IO/Format.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/IO/TextWriter.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Memory/MemoryProvider.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Json_ {
//----------------------------------------------------------------------------
static void EscapeString_(FTextWriter& oss, const FJson::FString& str) {
    Escape(oss, str, EEscape::Unicode);
}
//----------------------------------------------------------------------------
static bool ParseValue_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value);
//----------------------------------------------------------------------------
static bool ParseObject_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value) {
    FJson::FObject& object = value.SetType_AssumeNull(doc, FJson::TypeObject{});

    if (lexer.ReadIFN(Lexer::FSymbols::RBrace)) // quick reject for empty object
        return true;

    ASSOCIATIVE_VECTORINSITU_THREAD_LOCAL(Json, FJson::FString, FJson::FValue, 8) tmp; // use temporary dico to don't bloat the linear heap

    Lexer::FMatch key;
    for (bool notFirst = false;; notFirst = true) {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        if (not lexer.Expect(key, Lexer::FSymbols::String))
            break;

        if (not lexer.Expect(Lexer::FSymbols::Colon))
            CORE_THROW_IT(FJsonException("missing comma", key.Site()));

        FJson::FValue& value = tmp.Add(doc.MakeString(key.Value()));
        if (not ParseValue_(lexer, doc, value))
            CORE_THROW_IT(FJsonException("missing value", key.Site()));
    }

    object.Vector().assign(
        MakeMoveIterator(tmp.begin()),
        MakeMoveIterator(tmp.end()) );

    return lexer.Expect(Lexer::FSymbols::RBrace);
}
//----------------------------------------------------------------------------
static bool ParseArray_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value) {
    FJson::FArray& arr = value.SetType_AssumeNull(doc, FJson::TypeArray{});

    if (lexer.ReadIFN(Lexer::FSymbols::RBracket)) // quick reject for empty array
        return true;

    VECTORINSITU_THREAD_LOCAL(Json, FJson::FValue, 8) tmp; // use temporary array to don't bloat the linear heap

    for (bool notFirst = false;; notFirst = true) {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        tmp.push_back_Default();
        if (not ParseValue_(lexer, doc, tmp.back()))
            return false;
    }

    arr.assign(
        MakeMoveIterator(tmp.begin()),
        MakeMoveIterator(tmp.end()) );

    return lexer.Expect(Lexer::FSymbols::RBracket);
}
//----------------------------------------------------------------------------
static bool ParseValue_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value) {
    Lexer::FMatch read;
    if (not lexer.Read(read))
        return false;

    bool unexpectedToken = false;

    if (read.Symbol() == Lexer::FSymbols::LBrace) {
        return ParseObject_(lexer, doc, value);
    }
    else if (read.Symbol() == Lexer::FSymbols::LBracket) {
        return ParseArray_(lexer, doc, value);
    }
    else if (read.Symbol() == Lexer::FSymbols::String) {
        value.SetValue(doc.MakeString(read.Value()));
    }
    else if (read.Symbol() == Lexer::FSymbols::Int) {
        FJson::FInteger* i = &value.SetType_AssumeNull(doc, FJson::TypeInteger{});
        if (not Atoi64(i, read.Value().MakeView(), 10))
            CORE_THROW_IT(FJsonException("malformed int", read.Site()));
    }
    else if (read.Symbol() == Lexer::FSymbols::Float) {
        FJson::FFloat* d = &value.SetType_AssumeNull(doc, FJson::TypeFloat{});
        if (not Atod(d, read.Value().MakeView()))
            CORE_THROW_IT(FJsonException("malformed float", read.Site()));
    }
    // unary minus ?
    else if (read.Symbol() == Lexer::FSymbols::Sub) {
        const Lexer::FMatch* const peek = lexer.Peek();

        unexpectedToken = true;

        if (peek && peek->Symbol() == Lexer::FSymbols::Int) {
            if (ParseValue_(lexer, doc, value)) {
                value.ToInteger() = -value.ToInteger();
                unexpectedToken = false;
            }
        }
        else if (peek && peek->Symbol() == Lexer::FSymbols::Float) {
            if (ParseValue_(lexer, doc, value)) {
                value.ToFloat() = -value.ToFloat();
                unexpectedToken = false;
            }
        }
    }
    else if (read.Symbol() == Lexer::FSymbols::True) {
        value.SetType_AssumeNull(doc, FJson::TypeBool{}) = true;
    }
    else if (read.Symbol() == Lexer::FSymbols::False) {
        value.SetType_AssumeNull(doc, FJson::TypeBool{}) = false;
    }
    else if (read.Symbol() == Lexer::FSymbols::Null) {
        value.SetType_AssumeNull(doc, FJson::TypeNull{});
    }
    else {
        unexpectedToken = true;
    }

    if (unexpectedToken)
        CORE_THROW_IT(FJsonException("unexpected token", read.Site()));

    return true;
}
//----------------------------------------------------------------------------
static void ToStream_(const FJson::FValue& value, FTextWriter& oss, Fmt::FIndent& indent, bool minify) {
    switch (value.Type()) {

    case Core::Serialize::FJson::Null:
        oss << "null";
        break;
    case Core::Serialize::FJson::Bool:
        oss << (value.ToBool() ? "true" : "false");
        break;
    case Core::Serialize::FJson::Integer:
        oss << value.ToInteger();
        break;
    case Core::Serialize::FJson::Float:
        oss << value.ToFloat();
        break;

    case Core::Serialize::FJson::String:
        oss << '"';
        Json_::EscapeString_(oss, value.ToString());
        oss << '"';
        break;

    case Core::Serialize::FJson::Array:
        if (not value.ToArray().empty()) {
            const auto& arr = value.ToArray();
            oss << '[';
            if (not minify)
                oss << Eol;
            {
                const Fmt::FIndent::FScope scopeIndent(indent);

                size_t n = arr.size();
                for (const FJson::FValue& item : arr) {
                    oss << indent;
                    ToStream_(item, oss, indent, minify);
                    if (--n)
                        oss << ',';
                    if (not minify)
                        oss << Eol;
                }
            }
            oss << indent << ']';
        }
        else {
            oss << "[]";
        }
        break;

    case Core::Serialize::FJson::Object:
        if (not value.ToObject().empty()) {
            const auto& obj = value.ToObject();
            oss << '{';
            if (not minify)
                oss << Eol;
            {
                const Fmt::FIndent::FScope scopeIndent(indent);

                size_t n = obj.size();
                for (const auto& member : obj) {
                    oss << indent << '"';
                    Json_::EscapeString_(oss, member.first);
                    oss << "\": ";
                    ToStream_(member.second, oss, indent, minify);
                    if (--n)
                        oss << ',';
                    if (not minify)
                        oss << Eol;
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
} //!namespace Json_
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FJson::FValue::FValue(FJson& doc, EType type)
    : _type(type) {
    switch (_type) {
    case Core::Serialize::FJson::Null:
        SetType_AssumeNull(doc, TypeNull{});
        return;
    case Core::Serialize::FJson::Bool:
        SetType_AssumeNull(doc, TypeBool{});
        return;
    case Core::Serialize::FJson::Integer:
        SetType_AssumeNull(doc, TypeInteger{});
        return;
    case Core::Serialize::FJson::Float:
        SetType_AssumeNull(doc, TypeFloat{});
        return;
    case Core::Serialize::FJson::String:
        SetType_AssumeNull(doc, TypeString{});
        return;
    case Core::Serialize::FJson::Array:
        SetType_AssumeNull(doc, TypeArray{});
        return;
    case Core::Serialize::FJson::Object:
        SetType_AssumeNull(doc, TypeObject{});
        return;
    }
    AssertNotImplemented();
}
//----------------------------------------------------------------------------
FJson::FValue& FJson::FValue::operator =(const FValue& other) {
    Clear();

    _type = other._type;
    switch (other._type) {
    case Core::Serialize::FJson::Null:
        break;
    case Core::Serialize::FJson::Bool:
        _bool = other._bool;
        break;
    case Core::Serialize::FJson::Integer:
        _integer = other._integer;
        break;
    case Core::Serialize::FJson::Float:
        _float = other._float;
        break;
    case Core::Serialize::FJson::String:
        INPLACE_NEW(&_string, FString)(other._string);
        break;
    case Core::Serialize::FJson::Array:
        INPLACE_NEW(&_array, FArray)(other._array);
        break;
    case Core::Serialize::FJson::Object:
        INPLACE_NEW(&_object, FObject)(other._object);
        break;
    default:
        AssertNotImplemented();
        break;
    }

    return (*this);
}
//----------------------------------------------------------------------------
FJson::FValue& FJson::FValue::operator =(FValue&& rvalue) {
    Clear();

    switch (rvalue._type) {
    case Core::Serialize::FJson::Null:
        Assert(Null == _type);
        return (*this);
    case Core::Serialize::FJson::Bool:
        _bool = rvalue._bool;
        break;
    case Core::Serialize::FJson::Integer:
        _integer = rvalue._integer;
        break;
    case Core::Serialize::FJson::Float:
        _float = rvalue._float;
        break;
    case Core::Serialize::FJson::String:
        INPLACE_NEW(&_string, FString)(std::move(rvalue._string));
        //rvalue._string.~FString();
        break;
    case Core::Serialize::FJson::Array:
        INPLACE_NEW(&_array, FArray)(std::move(rvalue._array));
        //rvalue._array.~FArray();
        break;
    case Core::Serialize::FJson::Object:
        INPLACE_NEW(&_object, FObject)(std::move(rvalue._object));
        //rvalue._object.~FObject();
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
FJson::FValue& FJson::FValue::SetType(FJson& doc, EType type) {
    if (_type != FJson::Null)
        Clear();

    switch (type) {
    case Core::Serialize::FJson::Null:
        SetType_AssumeNull(doc, TType<Null>{});
        return (*this);
    case Core::Serialize::FJson::Bool:
        SetType_AssumeNull(doc, TType<Bool>{});
        return (*this);
    case Core::Serialize::FJson::Integer:
        SetType_AssumeNull(doc, TType<Integer>{});
        return (*this);
    case Core::Serialize::FJson::Float:
        SetType_AssumeNull(doc, TType<Float>{});
        return (*this);
    case Core::Serialize::FJson::String:
        SetType_AssumeNull(doc, TType<String>{});
        return (*this);
    case Core::Serialize::FJson::Array:
        SetType_AssumeNull(doc, TType<Array>{});
        return (*this);
    case Core::Serialize::FJson::Object:
        SetType_AssumeNull(doc, TType<Object>{});
        return (*this);
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void FJson::FValue::SetType_AssumeNull(FJson& , TType<Null>) { _type = Null; }
//----------------------------------------------------------------------------
auto FJson::FValue::SetType_AssumeNull(FJson& , TType<Bool>) ->FBool& {
    Assert(Null == _type);
    _type = Bool;
    return (*INPLACE_NEW(&_bool, FBool));
}
//----------------------------------------------------------------------------
auto FJson::FValue::SetType_AssumeNull(FJson& , TType<Integer>) -> FInteger& {
    Assert(Null == _type);
    _type = Integer;
    return (*INPLACE_NEW(&_integer, FInteger));
}
//----------------------------------------------------------------------------
auto FJson::FValue::SetType_AssumeNull(FJson& , TType<Float>) -> FFloat& {
    Assert(Null == _type);
    _type = Float;
    return (*INPLACE_NEW(&_float, FFloat));
}
//----------------------------------------------------------------------------
auto FJson::FValue::SetType_AssumeNull(FJson& , TType<String>) -> FString& {
    Assert(Null == _type);
    _type = String;
    return (*INPLACE_NEW(&_string, FString));
}
//----------------------------------------------------------------------------
auto FJson::FValue::SetType_AssumeNull(FJson& doc, TType<Array>) -> FArray& {
    Assert(Null == _type);
    _type = Array;
    return (*INPLACE_NEW(&_array, FArray)(FArray::allocator_type(doc._heap)));
}
//----------------------------------------------------------------------------
auto FJson::FValue::SetType_AssumeNull(FJson& doc, TType<Object>) -> FObject& {
    _type = Object;
    return (*INPLACE_NEW(&_object, FObject)(FObject::allocator_type(doc._heap)));
}
//----------------------------------------------------------------------------
void FJson::FValue::SetValue(FBool value) {
    Clear();
    _type = EType::Bool;
    INPLACE_NEW(&_bool, FBool)(value);
}
//----------------------------------------------------------------------------
void FJson::FValue::SetValue(FInteger value) {
    Clear();
    _type = EType::Integer;
    INPLACE_NEW(&_integer, FInteger)(value);
}
//----------------------------------------------------------------------------
void FJson::FValue::SetValue(FFloat value) {
    Clear();
    _type = EType::Float;
    INPLACE_NEW(&_float, FFloat)(value);
}
//----------------------------------------------------------------------------
void FJson::FValue::SetValue(FString&& value) {
    Clear();
    _type = EType::String;
    INPLACE_NEW(&_string, FString)(std::move(value));
}
//----------------------------------------------------------------------------
void FJson::FValue::SetValue(FArray&& value) {
    Clear();
    _type = EType::Array;
    INPLACE_NEW(&_array, FArray)(std::move(value));
}
//----------------------------------------------------------------------------
void FJson::FValue::SetValue(FObject&& value) {
    Clear();
    _type = EType::Object;
    INPLACE_NEW(&_object, FObject)(std::move(value));
}
//----------------------------------------------------------------------------
bool FJson::FValue::Equals(const FValue& other) const {
    if (other._type != _type)
        return false;

    switch (_type) {
    case Core::Serialize::FJson::Null:
        return true;
    case Core::Serialize::FJson::Bool:
        return (_bool == other._bool);
    case Core::Serialize::FJson::Integer:
        return (_integer == other._integer);
    case Core::Serialize::FJson::Float:
        return (_float == other._float);
    case Core::Serialize::FJson::String:
        return (_string == other._string);
    case Core::Serialize::FJson::Array:
        return (_array == other._array);
    case Core::Serialize::FJson::Object:
        return (_object == other._object);
    default:
        break;
    }

    AssertNotImplemented();
    return false;
}
//----------------------------------------------------------------------------
void FJson::FValue::Clear() {
#if 0 // skip destructor thanks to linear heap
    switch (_type) {
    case Core::Serialize::FJson::Null:
        return;
    case Core::Serialize::FJson::Bool:
    case Core::Serialize::FJson::Integer:
    case Core::Serialize::FJson::Float:
    case Core::Serialize::FJson::String:
        break;
    case Core::Serialize::FJson::Array:
        _array.~FArray();
        break;
    case Core::Serialize::FJson::Object:
        _object.~FObject();
        break;
    default:

        break;
    }
#endif

    _type = Null;
}
//----------------------------------------------------------------------------
void FJson::FValue::ToStream(FTextWriter& oss, bool minify/* = true */) const {
    Fmt::FIndent indent = (minify ? Fmt::FIndent::None() : Fmt::FIndent::TwoSpaces());
    oss << FTextFormat::DefaultFloat;
    Json_::ToStream_(*this, oss, indent, minify);
    Assert(0 == indent.Level);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::TCheckSameSize<FJson::FObject, FJson::FArray>::value);
STATIC_ASSERT(Meta::TCheckSameSize<FJson::FString, FJson::FArray>::value);
STATIC_ASSERT(Meta::TCheckSameSize<FJson::FObject, FJson::FString>::value);
//----------------------------------------------------------------------------
FJson::FJson()
    : _heap(LINEARHEAP_DOMAIN_TRACKINGDATA(Json))
    , _textHeap(_heap)
{}
//----------------------------------------------------------------------------
FJson::~FJson() {}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FFilename& filename) {
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
auto FJson::MakeString(const FStringView& str, bool mergeable/* = true */) -> FString {
    return _textHeap.MakeText(str);
}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FFilename& filename, const FStringView& content) {
    FMemoryViewReader reader(content.Cast<const u8>());
    return Load(json, filename, &reader);
}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FFilename& filename, IBufferedStreamReader* input) {
    Assert(json);
    Assert(input);

    const FWString filenameStr(filename.ToWString());
    Lexer::FLexer lexer(input, filenameStr.MakeView(), false);

    json->_root = FValue();
    json->_textHeap.reserve(32);

    CORE_TRY{
        if (not Json_::ParseValue_(lexer, *json, json->_root))
            return false;
    }
    CORE_CATCH(Lexer::FLexerException e)
    CORE_CATCH_BLOCK({
        CORE_THROW_IT(FJsonException(e.What(), e.Match().Site()));
    })

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
