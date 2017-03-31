#include "stdafx.h"

#include "JSON.h"

#include "Lexer/Lexer.h"
#include "Lexer/Match.h"
#include "Lexer/Symbol.h"
#include "Lexer/Symbols.h"

#include "Core.RTTI/MetaAtom.h"
#include "Core.RTTI/MetaClass.h"
#include "Core.RTTI/MetaObject.h"
#include "Core.RTTI/MetaProperty.h"
#include "Core.RTTI/MetaTransaction.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/Format.h"
#include "Core/IO/FormatHelpers.h"
#include "Core/Memory/MemoryProvider.h"

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace JSON_ {
//----------------------------------------------------------------------------
static void EscapeString_(std::basic_ostream<char>& oss, const FStringView& str) {
    for (char ch : str) {
        switch (ch) {
        case '\\':
        case '"':
        case '/':
            oss << '\\' << ch;
            break;
        case '\b':
            oss << "\\b";
            break;
        case '\t':
            oss << "\\t";
            break;
        case '\n':
            oss << "\\n";
            break;
        case '\f':
            oss << "\\f";
            break;
        case '\r':
            oss << "\\r";
            break;
        default:
            if (IsPrint(ch))
                oss << ch;
            else
                Format(oss, "\\u{0:#4x}", u8(ch));
        }
    }
}
//----------------------------------------------------------------------------
static void EscapeString_(std::basic_ostream<char>& oss, const FJSON::FString& str) {
    EscapeString_(oss, str.MakeView());
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
    FJSON::FArray& array = value.SetType(FJSON::Array).ToArray();

    FJSON::FValue item;
    for (bool notFirst = false;; notFirst = true)
    {
        if (notFirst && not lexer.ReadIFN(Lexer::FSymbols::Comma))
            break;

        if (ParseValue_(lexer, item))
            array.emplace_back(std::move(item));
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

    if (read.Symbol() == Lexer::FSymbols::LBrace) {
        return ParseObject_(lexer, value);
    }
    else if (read.Symbol() == Lexer::FSymbols::LBracket) {
        return ParseArray_(lexer, value);
    }
    else if (read.Symbol() == Lexer::FSymbols::String) {
        value.SetValue(std::move(read.Value()));
    }
    else if (read.Symbol() == Lexer::FSymbols::Int) {
        if (not Atod(&value.SetType(FJSON::Number).ToNumber(), read.Value().MakeView()))
            CORE_THROW_IT(FJSONException("malformed int", read.Site()));
    }
    else if (read.Symbol() == Lexer::FSymbols::Float) {
        if (not Atod(&value.SetType(FJSON::Number).ToNumber(), read.Value().MakeView()))
            CORE_THROW_IT(FJSONException("malformed float", read.Site()));
    }
    else if (read.Symbol() == Lexer::FSymbols::True) {
        value.SetValue(true);
    }
    else if (read.Symbol() == Lexer::FSymbols::False) {
        value.SetValue(false);
    }
    else {
        CORE_THROW_IT(FJSONException("unexpected token", read.Site()));
    }

    return true;
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
    case Core::Serialize::FJSON::Number:
        new (&_number) FNumber(0);
        break;
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
    case Core::Serialize::FJSON::Number:
        _number = other._number;
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
    case Core::Serialize::FJSON::Number:
        _number = rvalue._number;
        break;
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
    case Core::Serialize::FJSON::Number:
        new (&_number) FNumber();
        break;
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
    if (_type != EType::Null)
        Clear();

    _type = EType::Bool;
    new (&_bool) FBool(value);
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FNumber value) {
    if (_type != EType::Null)
        Clear();

    _type = EType::Number;
    new (&_number) FNumber(value);
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FString&& value) {
    if (_type != EType::Null)
        Clear();

    _type = EType::String;
    new (&_string) FString(std::move(value));
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FArray&& value) {
    if (_type != EType::Null)
        Clear();

    _type = EType::Array;
    new (&_array) FArray(std::move(value));
}
//----------------------------------------------------------------------------
void FJSON::FValue::SetValue(FObject&& value) {
    if (_type != EType::Null)
        Clear();

    _type = EType::Object;
    new (&_object) FObject(std::move(value));
}
//----------------------------------------------------------------------------
void FJSON::FValue::Clear() {
    switch (_type) {
    case Core::Serialize::FJSON::Null:
        return;
    case Core::Serialize::FJSON::Bool:
    case Core::Serialize::FJSON::Number:
        break;
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
    FIndent indent = (minify ? FIndent::None() : FIndent::TwoSpaces());
    oss << std::fixed;
    ToStream(oss, indent, minify);
    Assert(0 == indent.Level);
}
//----------------------------------------------------------------------------
void FJSON::FValue::ToStream(std::basic_ostream<char>& oss, FIndent& indent, bool minify) const {
    switch (_type) {
    case Core::Serialize::FJSON::Null:
        oss << "null";
        break;
    case Core::Serialize::FJSON::Bool:
        oss << (_bool ? "true" : "false");
        break;
    case Core::Serialize::FJSON::Number:
        oss << _number;
        break;
    case Core::Serialize::FJSON::String:
        oss << '"';
        JSON_::EscapeString_(oss, _string);
        oss << '"';
        break;
    case Core::Serialize::FJSON::Array:
        ;
        if (_array.size()) {
            oss << '[';
            if (not minify)
                oss << eol;
            {
                const FIndent::FScope scopeIndent(indent);
                size_t n = _array.size();
                for (const FValue& item : _array) {
                    oss << indent;
                    item.ToStream(oss, indent, minify);
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
        if (_object.size()) {
            oss << '{';
            if (not minify)
                oss << eol;
            {
                const FIndent::FScope scopeIndent(indent);
                size_t n = _object.size();
                for (const auto& member : _object) {
                    oss << indent << '"';
                    JSON_::EscapeString_(oss, member.first);
                    oss << "\": ";
                    member.second.ToStream(oss, indent, minify);
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
        break;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FJSON::Load(FJSON* json, const FFilename& filename) {
    Assert(json);
    Assert(not filename.empty());

    RAWSTORAGE_THREAD_LOCAL(FileSystem, u8) content;
    if (not VFS_ReadAll(&content, filename, AccessPolicy::Binary))
        return false;

    return Load(json, filename, content.MakeConstView().Cast<const char>());
}
//----------------------------------------------------------------------------
bool FJSON::Load(FJSON* json, const FFilename& filename, const FStringView& content) {
    FMemoryViewReader reader(content.Cast<const u8>());
    return Load(json, filename, &reader);
}
//----------------------------------------------------------------------------
bool FJSON::Load(FJSON* json, const FFilename& filename, IStreamReader* input) {
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
namespace {
//----------------------------------------------------------------------------
class FRTTItoJSON_ : public RTTI::FMetaAtomWrapCopyVisitor {
public:
    void PushHead(FJSON::FValue& value) {
        Assert(_values.empty());
        _values.push_back(&value);
    }

    void PopHead(FJSON::FValue& value) {
        UNUSED(value);
        Assert(1 == _values.size());
        Assert(&value == _values.front());
        _values.pop_back();
    }

    virtual void Inspect(const RTTI::IMetaAtomPair* ppair, const RTTI::TPair<RTTI::PMetaAtom, RTTI::PMetaAtom>& pair) override {
        UNUSED(ppair);
        FJSON::FArray& array = Head_().SetType(FJSON::Array).ToArray();
        array.resize(2);
        _values.push_back(&array.front());
        Append(pair.first.get());
        _values.pop_back();
        _values.push_back(&array.back());
        Append(pair.second.get());
        _values.pop_back();
    }

    virtual void Inspect(const RTTI::IMetaAtomVector* pvector, const RTTI::TVector<RTTI::PMetaAtom>& vector) override {
        UNUSED(pvector);
        FJSON::FArray& array = Head_().SetType(FJSON::Array).ToArray();
        array.resize(vector.size());
        forrange(i, 0, vector.size()) {
            _values.push_back(&array[i]);
            Append(vector[i].get());
            _values.pop_back();
        }
    }

    virtual void Inspect(const RTTI::IMetaAtomDictionary* pdictionary, const RTTI::TDictionary<RTTI::PMetaAtom, RTTI::PMetaAtom>& dictionary) override {
        UNUSED(pdictionary);
        FJSON::FArray& array = Head_().SetType(FJSON::Array).ToArray();
        array.resize(dictionary.size());
        size_t index = 0;
        for (const auto& it : dictionary) {
            _values.push_back(&array[index++]);
            Inspect(nullptr, it);
            _values.pop_back();
        }
    }

#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) \
    virtual void Visit(const RTTI::TMetaTypedAtom<T>* scalar) override { \
        Assert(scalar); \
        Assert(_TypeId == scalar->TypeInfo().Id); \
        ToJSON_(scalar->Wrapper()); \
        /*parent_type::Visit(scalar);*/ \
    }
    FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR

private:
    VECTOR(Serialize, FJSON::FValue*) _values;

    FJSON::FValue& Head_() const { return (*_values.back()); }

    template <typename T>
    void ToJSON_(const T& value) {
        Head_().SetValue(FJSON::FNumber(value));
    }

    void ToJSON_(const bool& value) {
        Head_().SetValue(FJSON::FBool(value));
    }

    void ToJSON_(const FString& str) {
        Head_().SetValue(FJSON::FString(std::move(str)));
    }

    void ToJSON_(const FWString& wstr) {
        Head_().SetValue(FJSON::FString(std::move(ToString(wstr))));
    }

    void ToJSON_(const RTTI::FName& name) {
        Head_().SetValue(FJSON::FString(std::move(ToString(name.MakeView()))));
    }

    void ToJSON_(const RTTI::FBinaryData& rawdata) {
        Head_().SetValue(ToString(rawdata.MakeView().Cast<const char>()));
    }

    void ToJSON_(const RTTI::FOpaqueData& opaqueData) {
        FJSON::FObject& object = Head_().SetType(FJSON::Object).ToObject();
        object.reserve(opaqueData.size());
        for (const RTTI::TPair<RTTI::FName, RTTI::PMetaAtom>& pair : opaqueData) {
            _values.push_back(&object[std::move(ToString(pair.first.MakeView()))]);
            if (pair.second)
                Append(pair.second.get());
            _values.pop_back();
        }
    }

    template <typename T, size_t _Dim>
    void ToJSON_(const TScalarVector<T, _Dim>& v) {
        FJSON::FArray& array = Head_().SetType(FJSON::Array).ToArray();
        array.resize(_Dim);
        forrange(i, 0, _Dim)
            array[i].SetValue(FJSON::FNumber(v._data[i]));
    }

    template <typename T, size_t _Width, size_t _Height>
    void ToJSON_(const TScalarMatrix<T, _Width, _Height>& m) {
        constexpr size_t Dim = (_Width * _Height);
        FJSON::FArray& array = Head_().SetType(FJSON::Array).ToArray();
        array.resize(Dim);
        forrange(i, 0, Dim)
            array[i].SetValue(FJSON::FNumber(m._data.raw[i]));
    }

    void ToJSON_(const RTTI::PMetaAtom& atom) {
        if (atom) {
            const RTTI::FMetaTypeId typeId = atom->TypeInfo().Id;
            switch (typeId)
            {
#define DEF_METATYPE_SCALAR(_Name, T, _TypeId, _Unused) case _TypeId:
                FOREACH_CORE_RTTI_NATIVE_TYPES(DEF_METATYPE_SCALAR)
#undef DEF_METATYPE_SCALAR
                    atom->Accept(this);
                break;

            default:
                CORE_THROW_IT(FJSONException("no support for abstract RTTI atoms serialization"));
            }
        }
    }

    void ToJSON_(const RTTI::PMetaObject& metaObject) {
        FJSON::FObject& jsonObject = Head_().SetType(FJSON::Object).ToObject();
        if (metaObject) {
            const RTTI::FMetaClass* MetaClass = metaObject->RTTI_MetaClass();
            jsonObject["@MetaClass"].SetValue(ToString(MetaClass->Name().MakeView()));
            for (const RTTI::FMetaProperty* prop : MetaClass->AllProperties()) {
                if (not prop->IsDefaultValue(metaObject.get())) {
                    const RTTI::PMetaAtom atom = prop->WrapCopy(metaObject.get());
                    _values.push_back(&jsonObject[std::move(ToString(prop->Name().MakeView()))]);
                    atom->Accept(this);
                    _values.pop_back();
                }
            }
        }
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
void RTTItoJSON(FJSON& dst, const VECTOR(Serialize, RTTI::PMetaAtom)& src) {
    FRTTItoJSON_ visitor;
    FJSON::FArray& array = dst.Root().SetType(FJSON::Array).ToArray();
    array.resize(src.size());
    forrange(i, 0, src.size()) {
        FJSON::FValue& item = array[i];
        visitor.PushHead(item);
        visitor.Append(src[i].get());
        visitor.PopHead(item);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void JSONtoRTTI(VECTOR(Serialize, RTTI::PMetaAtom)& dst, const FJSON& src) {
    AssertNotImplemented(); // TODO
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
