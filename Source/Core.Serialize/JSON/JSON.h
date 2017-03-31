#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/Lexer/Location.h"

#include "Core.RTTI/RTTI_fwd.h"

#include "Core/Container/StringHashMap.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"

namespace Core {
struct FIndent;
class IStreamReader;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FJSONException : public Core::Serialize::FSerializeException {
public:
    typedef Core::Serialize::FSerializeException parent_type;

    FJSONException(const char *what)
        : FJSONException(what, Lexer::FLocation()) {}

    FJSONException(const char *what, const Lexer::FLocation& site)
        : parent_type(what), _site(site) {}

    virtual ~FJSONException() {}

    const Lexer::FLocation& Site() const { return _site; }

private:
    Lexer::FLocation _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FJSON {
public:
    class FValue;

    typedef bool FBool;
    typedef double FNumber;
    typedef Core::FString FString;
    typedef VECTOR(JSON, FValue) FArray;
    typedef STRING_HASHMAP(JSON, FValue, ECase::Sensitive) FObject;

    enum EType {
        Null = 0,
        Bool,
        Number,
        String,
        Array,
        Object,
    };

    class FValue {
    public:
        FValue() : _type(Null) {}
        ~FValue() { Clear(); }

        explicit FValue(EType type);

        explicit FValue(FBool value) : _type(Bool), _bool(value) {}
        explicit FValue(FNumber value) : _type(Number), _number(value) {}
        explicit FValue(FString&& value) : _type(String), _string(std::move(value)) {}
        explicit FValue(FArray&& value) : _type(Array), _array(std::move(value)) {}
        explicit FValue(FObject&& value) : _type(Object), _object(std::move(value)) {}

        FValue(const FValue& other) : FValue() { operator =(other); }
        FValue& operator =(const FValue& other);

        FValue(FValue&& rvalue) : FValue() { operator =(std::move(rvalue)); }
        FValue& operator =(FValue&& rvalue);

        EType Type() const { return _type; }
        FValue& SetType(EType type);

        void SetValue(FBool value);
        void SetValue(FNumber value);
        void SetValue(FString&& value);
        void SetValue(FArray&& value);
        void SetValue(FObject&& value);

        bool AsNull() const { return (_type == Null); }
        const FBool* AsBool() const { return (_type == Bool ? &_bool : nullptr); }
        const FNumber* AsNumber() const { return (_type == Number ? &_number : nullptr); }
        const FString* AsString() const { return (_type == String ? &_string : nullptr); }
        const FArray* AsArray() const { return (_type == Array ? &_array : nullptr); }
        const FObject* AsObject() const { return (_type == Object ? &_object : nullptr); }

        FBool& ToBool() { Assert(_type == Bool); return _bool; }
        FNumber& ToNumber() { Assert(_type == Number); return _number; }
        FString& ToString() { Assert(_type == String); return _string; }
        FArray& ToArray() { Assert(_type == Array); return _array; }
        FObject& ToObject() { Assert(_type == Object); return _object; }

        const FBool& ToBool() const { Assert(_type == Bool); return _bool; }
        const FNumber& ToNumber() const { Assert(_type == Number); return _number; }
        const FString& ToString() const { Assert(_type == String); return _string; }
        const FArray& ToArray() const { Assert(_type == Array); return _array; }
        const FObject& ToObject() const { Assert(_type == Object); return _object; }

        void Clear();

        void ToStream(std::basic_ostream<char>& oss, bool minify = true) const;
        void ToStream(std::basic_ostream<char>& oss, FIndent& indent, bool minify) const;

    private:
        EType _type;
        union {
            FBool _bool;
            FNumber _number;
            FString _string;
            FArray _array;
            FObject _object;
        };
    };

public:
    FJSON() {}

    FJSON(const FJSON&) = delete;
    FJSON& operator =(const FJSON&) = delete;

    FValue& Root() { return _root; }
    const FValue& Root() const { return _root; }

    void ToStream(std::basic_ostream<char>& oss, bool minify = false) const { _root.ToStream(oss, minify); }

    static bool Load(FJSON* json, const FFilename& filename);
    static bool Load(FJSON* json, const FFilename& filename, IStreamReader* input);
    static bool Load(FJSON* json, const FFilename& filename, const FStringView& content);

private:
    FValue _root;
};
//----------------------------------------------------------------------------
inline std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const FJSON::FValue& jsonValue) {
    jsonValue.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
inline std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const FJSON& json) {
    json.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void RTTItoJSON(FJSON& dst, const VECTOR(Serialize, RTTI::PMetaAtom)& src);
//----------------------------------------------------------------------------
void JSONtoRTTI(VECTOR(Serialize, RTTI::PMetaAtom)& dst, const FJSON& src);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
