#include "stdafx.h"

#include "Json/Json.h"

#include "Lexer/Lexer.h"
#include "Lexer/Match.h"
#include "Lexer/Symbol.h"
#include "Lexer/Symbols.h"

#include "Container/AssociativeVector.h"
#include "Container/RawStorage.h"
#include "Container/Vector.h"
#include "IO/ConstNames.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryProvider.h"
#include "VirtualFileSystem.h"

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Json_ {
//----------------------------------------------------------------------------
static void EscapeString_(FTextWriter& oss, const FJson::FText& str) {
    Escape(oss, str, EEscape::Unicode);
}
//----------------------------------------------------------------------------
static void EscapeString_(FWTextWriter& oss, const FJson::FText& str) {
    Escape(oss, ToWCStr(INLINE_MALLOCA(wchar_t, str.size() + 1), str), EEscape::Unicode);
}
//----------------------------------------------------------------------------
static bool ParseValue_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value);
//----------------------------------------------------------------------------
static bool ParseObject_(Lexer::FLexer& lexer, FJson& doc, FJson::FValue& value) {
    FJson::FObject& object = value.SetType_AssumeNull(doc, FJson::TypeObject{});

    if (lexer.ReadIFN(Lexer::FSymbols::RBrace)) // quick reject for empty object
        return true;

    ASSOCIATIVE_VECTORINSITU(Json, FJson::FText, FJson::FValue, 8) tmp; // use temporary dico to don't bloat the linear heap

    Lexer::FMatch key;
    for (bool notFirst = false;; notFirst = true) {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        if (not lexer.Expect(key, Lexer::FSymbols::String))
            break;

        if (not lexer.Expect(Lexer::FSymbols::Colon))
            PPE_THROW_IT(FJsonException("missing comma", key.Site()));

        FJson::FValue& v = tmp.Add(doc.MakeString(key.Value()));
        if (not ParseValue_(lexer, doc, v))
            PPE_THROW_IT(FJsonException("missing value", key.Site()));
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

    VECTORINSITU(Json, FJson::FValue, 8) tmp; // use temporary array to don't bloat the linear heap

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
    else if (read.Symbol() == Lexer::FSymbols::Integer) {
        FJson::FInteger* i = &value.SetType_AssumeNull(doc, FJson::TypeInteger{});
        if (not Atoi(i, read.Value().MakeView(), 10))
            PPE_THROW_IT(FJsonException("malformed int", read.Site()));
    }
    else if (read.Symbol() == Lexer::FSymbols::Unsigned) {
        FJson::FInteger* i = &value.SetType_AssumeNull(doc, FJson::TypeInteger{});
        u64 u;
        if (not Atoi(&u, read.Value().MakeView(), 10))
            PPE_THROW_IT(FJsonException("malformed int", read.Site()));
        *i = i64(u);
    }
    else if (read.Symbol() == Lexer::FSymbols::Float) {
        FJson::FFloat* d = &value.SetType_AssumeNull(doc, FJson::TypeFloat{});
        if (not Atod(d, read.Value().MakeView()))
            PPE_THROW_IT(FJsonException("malformed float", read.Site()));
    }
    // unary minus ?
    else if (read.Symbol() == Lexer::FSymbols::Sub) {
        const Lexer::FMatch* const peek = lexer.Peek();

        unexpectedToken = true;

        if (peek && peek->Symbol() == Lexer::FSymbols::Integer) {
            if (ParseValue_(lexer, doc, value)) {
                value.ToInteger() = -value.ToInteger();
                unexpectedToken = false;
            }
        }
        else if (peek && peek->Symbol() == Lexer::FSymbols::Unsigned) {
            AssertNotReached(); // -unsigned isn't supported !
#if 0
            if (ParseValue_(lexer, doc, value)) {
                value.ToInteger() = -value.ToInteger();
                unexpectedToken = false;
            }
#endif
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
        PPE_THROW_IT(FJsonException("unexpected token", read.Site()));

    return true;
}
//----------------------------------------------------------------------------
template <typename _Char>
static void ToStream_(const FJson::FValue& value, TBasicTextWriter<_Char>& oss, Fmt::TBasicIndent<_Char>& indent, bool minify) {
    switch (value.Type()) {

    case PPE::Serialize::FJson::Null:
        oss << "null";
        break;
    case PPE::Serialize::FJson::Bool:
        oss << value.ToBool();
        break;
    case PPE::Serialize::FJson::Integer:
        oss << value.ToInteger();
        break;
    case PPE::Serialize::FJson::Float:
        oss << value.ToFloat();
        break;

    case PPE::Serialize::FJson::String:
        oss << Fmt::DoubleQuote;
        Json_::EscapeString_(oss, value.ToString());
        oss << Fmt::DoubleQuote;
        break;

    case PPE::Serialize::FJson::Array:
        if (not value.ToArray().empty()) {
            const auto& arr = value.ToArray();
            oss << Fmt::LBracket;
            if (not minify)
                oss << Eol;
            {
                const typename Fmt::TBasicIndent<_Char>::FScope scopeIndent(indent);

                size_t n = arr.size();
                for (const FJson::FValue& item : arr) {
                    oss << indent;
                    ToStream_(item, oss, indent, minify);
                    if (--n)
                        oss << Fmt::Comma;
                    if (not minify)
                        oss << Eol;
                }
            }
            oss << indent << Fmt::RBracket;
        }
        else {
            oss << Fmt::LBracket << Fmt::RBracket;
        }
        break;

    case PPE::Serialize::FJson::Object:
        if (not value.ToObject().empty()) {
            const auto& obj = value.ToObject();
            oss << Fmt::LBrace;
            if (not minify)
                oss << Eol;
            {
                const typename Fmt::TBasicIndent<_Char>::FScope scopeIndent(indent);

                size_t n = obj.size();
                for (const auto& member : obj) {
                    oss << indent << Fmt::DoubleQuote;
                    Json_::EscapeString_(oss, member.first);
                    oss << Fmt::DoubleQuote << Fmt::Colon  << Fmt::Space;
                    ToStream_(member.second, oss, indent, minify);
                    if (--n)
                        oss << Fmt::Comma;
                    if (not minify)
                        oss << Eol;
                }
            }
            oss << indent << Fmt::RBrace;
        }
        else {
            oss << Fmt::LBrace << Fmt::RBrace;
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
    case PPE::Serialize::FJson::Null:
        SetType_AssumeNull(doc, TypeNull{});
        return;
    case PPE::Serialize::FJson::Bool:
        SetType_AssumeNull(doc, TypeBool{});
        return;
    case PPE::Serialize::FJson::Integer:
        SetType_AssumeNull(doc, TypeInteger{});
        return;
    case PPE::Serialize::FJson::Float:
        SetType_AssumeNull(doc, TypeFloat{});
        return;
    case PPE::Serialize::FJson::String:
        SetType_AssumeNull(doc, TypeString{});
        return;
    case PPE::Serialize::FJson::Array:
        SetType_AssumeNull(doc, TypeArray{});
        return;
    case PPE::Serialize::FJson::Object:
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
    case PPE::Serialize::FJson::Null:
        break;
    case PPE::Serialize::FJson::Bool:
        _bool = other._bool;
        break;
    case PPE::Serialize::FJson::Integer:
        _integer = other._integer;
        break;
    case PPE::Serialize::FJson::Float:
        _float = other._float;
        break;
    case PPE::Serialize::FJson::String:
        INPLACE_NEW(&_string, FText)(other._string);
        break;
    case PPE::Serialize::FJson::Array:
        INPLACE_NEW(&_array, FArray)(other._array);
        break;
    case PPE::Serialize::FJson::Object:
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
    case PPE::Serialize::FJson::Null:
        Assert(Null == _type);
        return (*this);
    case PPE::Serialize::FJson::Bool:
        _bool = rvalue._bool;
        break;
    case PPE::Serialize::FJson::Integer:
        _integer = rvalue._integer;
        break;
    case PPE::Serialize::FJson::Float:
        _float = rvalue._float;
        break;
    case PPE::Serialize::FJson::String:
        INPLACE_NEW(&_string, FText)(std::move(rvalue._string));
        //rvalue._string.~FText();
        break;
    case PPE::Serialize::FJson::Array:
        INPLACE_NEW(&_array, FArray)(std::move(rvalue._array));
        //rvalue._array.~FArray();
        break;
    case PPE::Serialize::FJson::Object:
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
    case PPE::Serialize::FJson::Null:
        SetType_AssumeNull(doc, TType<Null>{});
        return (*this);
    case PPE::Serialize::FJson::Bool:
        SetType_AssumeNull(doc, TType<Bool>{});
        return (*this);
    case PPE::Serialize::FJson::Integer:
        SetType_AssumeNull(doc, TType<Integer>{});
        return (*this);
    case PPE::Serialize::FJson::Float:
        SetType_AssumeNull(doc, TType<Float>{});
        return (*this);
    case PPE::Serialize::FJson::String:
        SetType_AssumeNull(doc, TType<String>{});
        return (*this);
    case PPE::Serialize::FJson::Array:
        SetType_AssumeNull(doc, TType<Array>{});
        return (*this);
    case PPE::Serialize::FJson::Object:
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
auto FJson::FValue::SetType_AssumeNull(FJson& , TType<String>) -> FText& {
    Assert(Null == _type);
    _type = String;
    return (*INPLACE_NEW(&_string, FText));
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
void FJson::FValue::SetValue(FText&& value) {
    Clear();
    _type = EType::String;
    INPLACE_NEW(&_string, FText)(std::move(value));
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
    case PPE::Serialize::FJson::Null:
        return true;
    case PPE::Serialize::FJson::Bool:
        return (_bool == other._bool);
    case PPE::Serialize::FJson::Integer:
        return (_integer == other._integer);
    case PPE::Serialize::FJson::Float:
        return (_float == other._float);
    case PPE::Serialize::FJson::String:
        return (_string == other._string);
    case PPE::Serialize::FJson::Array:
        return (_array == other._array);
    case PPE::Serialize::FJson::Object:
        return (_object == other._object);
    }

    AssertNotImplemented();
}
//----------------------------------------------------------------------------
void FJson::FValue::Clear() {
#if 0 // skip destructor thanks to linear heap
    switch (_type) {
    case PPE::Serialize::FJson::Null:
        return;
    case PPE::Serialize::FJson::Bool:
    case PPE::Serialize::FJson::Integer:
    case PPE::Serialize::FJson::Float:
    case PPE::Serialize::FJson::String:
        break;
    case PPE::Serialize::FJson::Array:
        _array.~FArray();
        break;
    case PPE::Serialize::FJson::Object:
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
    oss << FTextFormat::DefaultFloat
        << FTextFormat::BoolAlpha;
    Json_::ToStream_(*this, oss, indent, minify);
    Assert(0 == indent.Level);
}
//----------------------------------------------------------------------------
void FJson::FValue::ToStream(FWTextWriter& oss, bool minify/* = true */) const {
    Fmt::FWIndent indent = (minify ? Fmt::FWIndent::None() : Fmt::FWIndent::TwoSpaces());
    oss << FTextFormat::DefaultFloat
        << FTextFormat::BoolAlpha;
    Json_::ToStream_(*this, oss, indent, minify);
    Assert(0 == indent.Level);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::TCheckSameSize<FJson::FObject, FJson::FArray>::value);
STATIC_ASSERT(Meta::TCheckSameSize<FJson::FText, FJson::FArray>::value);
STATIC_ASSERT(Meta::TCheckSameSize<FJson::FObject, FJson::FText>::value);
//----------------------------------------------------------------------------
FJson::FJson()
    : _textHeap(_heap)
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

    RAWSTORAGE(FileSystem, u8) content;
    if (not VFS_ReadAll(&content, filename, policy))
        return false;

    return Load(json, filename.ToWString(), content.MakeConstView().Cast<const char>());
}
//----------------------------------------------------------------------------
auto FJson::MakeString(const FStringView& str, bool mergeable/* = true */) -> FText {
    return _textHeap.MakeText(str, mergeable);
}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FWStringView& filename, const FStringView& content) {
    FMemoryViewReader reader(content.Cast<const u8>());
    return Load(json, filename, &reader);
}
//----------------------------------------------------------------------------
bool FJson::Load(FJson* json, const FWStringView& filename, IBufferedStreamReader* input) {
    Assert(json);
    Assert(input);

    Lexer::FLexer lexer(input, filename, false);

    json->_root = FValue();
    json->_textHeap.Clear();
    json->_heap.ReleaseAll();

    PPE_TRY{
        if (not Json_::ParseValue_(lexer, *json, json->_root))
            return false;
    }
    PPE_CATCH(Lexer::FLexerException e)
    PPE_CATCH_BLOCK({
        PPE_THROW_IT(FJsonException(e.What(), e.Match().Site()));
    })

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
