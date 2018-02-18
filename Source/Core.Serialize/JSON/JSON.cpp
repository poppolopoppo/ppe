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
#include "Core/IO/TextWriter.h"
#include "Core/IO/VirtualFileSystem.h"
#include "Core/Memory/MemoryProvider.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace JSON_ {
//----------------------------------------------------------------------------
static void EscapeString_(FTextWriter& oss, const FJSON::FString& str) {
    Escape(oss, str, EEscape::Unicode);
}
//----------------------------------------------------------------------------
static bool ParseValue_(Lexer::FLexer& lexer, FJSON& doc, FJSON::FValue& value);
//----------------------------------------------------------------------------
static bool ParseObject_(Lexer::FLexer& lexer, FJSON& doc, FJSON::FValue& value) {
    FJSON::FObject& object = value.SetType_AssumeNull(doc, FJSON::TypeObject{});

    Lexer::FMatch key;
    for (bool notFirst = false;; notFirst = true)
    {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        if (not lexer.Expect(key, Lexer::FSymbols::String))
            break;

        if (not lexer.Expect(Lexer::FSymbols::Colon))
            CORE_THROW_IT(FJSONException("missing comma", key.Site()));

        if (not ParseValue_(lexer, doc, object[doc.MakeString(key.Value())]))
            CORE_THROW_IT(FJSONException("missing value", key.Site()));
    }

    return lexer.Expect(Lexer::FSymbols::RBrace);
}
//----------------------------------------------------------------------------
static bool ParseArray_(Lexer::FLexer& lexer, FJSON& doc, FJSON::FValue& value) {
    FJSON::FArray& arr = value.SetType_AssumeNull(doc, FJSON::TypeArray{});

    FJSON::FValue item;
    for (bool notFirst = false;; notFirst = true)
    {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        if (ParseValue_(lexer, doc, item))
            arr.emplace_back(std::move(item));
        else
            break;
    }

    return lexer.Expect(Lexer::FSymbols::RBracket);
}
//----------------------------------------------------------------------------
static bool ParseValue_(Lexer::FLexer& lexer, FJSON& doc, FJSON::FValue& value) {
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
        FJSON::FInteger* i = &value.SetType_AssumeNull(doc, FJSON::TypeInteger{});
        if (not Atoi64(i, read.Value().MakeView(), 10))
            CORE_THROW_IT(FJSONException("malformed int", read.Site()));
    }
    else if (read.Symbol() == Lexer::FSymbols::Float) {
        FJSON::FFloat* d = &value.SetType_AssumeNull(doc, FJSON::TypeFloat{});
        if (not Atod(d, read.Value().MakeView()))
            CORE_THROW_IT(FJSONException("malformed float", read.Site()));
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
        value.SetType_AssumeNull(doc, FJSON::TypeBool{}) = true;
    }
    else if (read.Symbol() == Lexer::FSymbols::False) {
        value.SetType_AssumeNull(doc, FJSON::TypeBool{}) = false;
    }
    else if (read.Symbol() == Lexer::FSymbols::Null) {
        value.SetType_AssumeNull(doc, FJSON::TypeNull{});
    }
    else {
        unexpectedToken = true;
    }

    if (unexpectedToken)
        CORE_THROW_IT(FJSONException("unexpected token", read.Site()));

    return true;
}
//----------------------------------------------------------------------------
static void ToStream_(const FJSON::FValue& value, FTextWriter& oss, Fmt::FIndent& indent, bool minify) {
    switch (value.Type()) {

    case Core::Serialize::FJSON::Null:
        oss << "null";
        break;
    case Core::Serialize::FJSON::Bool:
        oss << (value.ToBool() ? "true" : "false");
        break;
    case Core::Serialize::FJSON::Integer:
        oss << value.ToInteger();
        break;
    case Core::Serialize::FJSON::Float:
        oss << value.ToFloat();
        break;

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
                oss << Eol;
            {
                const Fmt::FIndent::FScope scopeIndent(indent);

                size_t n = arr.size();
                for (const FJSON::FValue& item : arr) {
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

    case Core::Serialize::FJSON::Object:
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
                    JSON_::EscapeString_(oss, member.first);
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
} //!namespace JSON_
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FJSON::FValue::FValue(FJSON& doc, EType type)
    : _type(type) {
    switch (_type) {
    case Core::Serialize::FJSON::Null:
        SetType_AssumeNull(doc, TypeNull{});
        return;
    case Core::Serialize::FJSON::Bool:
        SetType_AssumeNull(doc, TypeBool{});
        return;
    case Core::Serialize::FJSON::Integer:
        SetType_AssumeNull(doc, TypeInteger{});
        return;
    case Core::Serialize::FJSON::Float:
        SetType_AssumeNull(doc, TypeFloat{});
        return;
    case Core::Serialize::FJSON::String:
        SetType_AssumeNull(doc, TypeString{});
        return;
    case Core::Serialize::FJSON::Array:
        SetType_AssumeNull(doc, TypeArray{});
        return;
    case Core::Serialize::FJSON::Object:
        SetType_AssumeNull(doc, TypeObject{});
        return;
    }
    AssertNotImplemented();
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
    case Core::Serialize::FJSON::Integer:
        _integer = other._integer;
        break;
    case Core::Serialize::FJSON::Float:
        _float = other._float;
        break;
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
    case Core::Serialize::FJSON::Integer:
        _integer = rvalue._integer;
        break;
    case Core::Serialize::FJSON::Float:
        _float = rvalue._float;
        break;
    case Core::Serialize::FJSON::String:
        new (&_string) FString(std::move(rvalue._string));
        //rvalue._string.~FString();
        break;
    case Core::Serialize::FJSON::Array:
        new (&_array) FArray(std::move(rvalue._array));
        //rvalue._array.~FArray();
        break;
    case Core::Serialize::FJSON::Object:
        new (&_object) FObject(std::move(rvalue._object));
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
FJSON::FValue& FJSON::FValue::SetType(FJSON& doc, EType type) {
    if (_type != FJSON::Null)
        Clear();

    switch (type) {
    case Core::Serialize::FJSON::Null:
        SetType_AssumeNull(doc, TType<Null>{});
        return (*this);
    case Core::Serialize::FJSON::Bool:
        SetType_AssumeNull(doc, TType<Bool>{});
        return (*this);
    case Core::Serialize::FJSON::Integer:
        SetType_AssumeNull(doc, TType<Integer>{});
        return (*this);
    case Core::Serialize::FJSON::Float:
        SetType_AssumeNull(doc, TType<Float>{});
        return (*this);
    case Core::Serialize::FJSON::String:
        SetType_AssumeNull(doc, TType<String>{});
        return (*this);
    case Core::Serialize::FJSON::Array:
        SetType_AssumeNull(doc, TType<Array>{});
        return (*this);
    case Core::Serialize::FJSON::Object:
        SetType_AssumeNull(doc, TType<Object>{});
        return (*this);
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetType_AssumeNull(FJSON& , TType<Null>) { _type = Null; }
//----------------------------------------------------------------------------
auto FJSON::FValue::SetType_AssumeNull(FJSON& , TType<Bool>) ->FBool& {
    Assert(Null == _type);
    _type = Bool;
    return (*new (&_bool) FBool);
}
//----------------------------------------------------------------------------
auto FJSON::FValue::SetType_AssumeNull(FJSON& , TType<Integer>) -> FInteger& {
    Assert(Null == _type);
    _type = Integer;
    return (*new (&_integer) FInteger);
}
//----------------------------------------------------------------------------
auto FJSON::FValue::SetType_AssumeNull(FJSON& , TType<Float>) -> FFloat& {
    Assert(Null == _type);
    _type = Float;
    return (*new (&_float) FFloat);
}
//----------------------------------------------------------------------------
auto FJSON::FValue::SetType_AssumeNull(FJSON& , TType<String>) -> FString& {
    Assert(Null == _type);
    _type = String;
    return (*new (&_string) FString);
}
//----------------------------------------------------------------------------
auto FJSON::FValue::SetType_AssumeNull(FJSON& doc, TType<Array>) -> FArray& {
    Assert(Null == _type);
    _type = Array;
    return (*new (&_array) FArray(FArray::allocator_type(doc._heap)));
}
//----------------------------------------------------------------------------
auto FJSON::FValue::SetType_AssumeNull(FJSON& doc, TType<Object>) -> FObject& {
    _type = Object;
    return (*new (&_object) FObject(FObject::allocator_type(doc._heap)));
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FBool value) {
    Clear();
    _type = EType::Bool;
    new (&_bool) FBool(value);
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FInteger value) {
    Clear();
    _type = EType::Integer;
    new (&_integer) FInteger(value);
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FFloat value) {
    Clear();
    _type = EType::Float;
    new (&_float) FFloat(value);
}
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
#if 0 // skip destructor thanks to linear heap
    switch (_type) {
    case Core::Serialize::FJSON::Null:
        return;
    case Core::Serialize::FJSON::Bool:
    case Core::Serialize::FJSON::Integer:
    case Core::Serialize::FJSON::Float:
    case Core::Serialize::FJSON::String:
        break;
    case Core::Serialize::FJSON::Array:
        _array.~FArray();
        break;
    case Core::Serialize::FJSON::Object:
        _object.~FObject();
        break;
    default:
        
        break;
    }
#endif

    _type = Null;
}
//----------------------------------------------------------------------------
void FJSON::FValue::ToStream(FTextWriter& oss, bool minify/* = true */) const {
    Fmt::FIndent indent = (minify ? Fmt::FIndent::None() : Fmt::FIndent::TwoSpaces());
    oss << FTextFormat::DefaultFloat;
    JSON_::ToStream_(*this, oss, indent, minify);
    Assert(0 == indent.Level);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FJSON::FJSON() 
    : _heap(LINEARHEAP_DOMAIN_TRACKINGDATA(JSON))
    , _strings(stringtable_type::allocator_type(_heap))
{}
//----------------------------------------------------------------------------
FJSON::~FJSON() {}
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
FStringView FJSON::MakeString(const FStringView& str, bool mergeable/* = true */) {
    if (str.empty()) {
        return str;
    }
    else if (mergeable) {
        // tries to pool short to medium strings :
        const auto it = _strings.insert(str);
        if (it.second) {
            void* const storage = _heap.Allocate(str.SizeInBytes(), std::alignment_of_v<char>);
            ::memcpy(storage, str.data(), str.SizeInBytes());
            // hack for replacing registered string view with one pointer to linear heap storage
            const auto allocated = FStringView((char*)storage, str.size());
            auto& registered = const_cast<FStringView&>(*it.first);
            Assert(hash_string(allocated) == hash_string(registered));
            registered = allocated;
        }
        return (*it.first);
    }
    else {
        // some strings have too much entropy and won't benefit from merging :
        void* const storage = _heap.Allocate(str.SizeInBytes(), std::alignment_of_v<char>);
        ::memcpy(storage, str.data(), str.SizeInBytes());
        return FStringView((char*)storage, str.size());
    }
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
    json->_strings.reserve(32);

    CORE_TRY{
        if (not JSON_::ParseValue_(lexer, *json, json->_root))
            return false;
    }
    CORE_CATCH(Lexer::FLexerException e)
    CORE_CATCH_BLOCK({
        CORE_THROW_IT(FJSONException(e.What(), e.Match().Site()));
    })

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
