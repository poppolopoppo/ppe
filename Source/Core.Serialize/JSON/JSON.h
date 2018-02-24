#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/Lexer/Location.h"

#include "Core/Allocator/LinearHeap.h"
#include "Core/Container/StringHashMap.h"
#include "Core/Container/StringHashSet.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/TextWriter_fwd.h"

namespace Core {
class IBufferedStreamReader;
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
class CORE_SERIALIZE_API FJSON {
public:
    class FValue;

    using FBool = bool;
    using FInteger = i64;
    using FFloat = double;
    using FString = FStringView;
    using FArray = VECTOR_LINEARHEAP(FValue);
    using FObject = TBasicStringViewHashMap<
        char, FValue, ECase::Sensitive,
        TLinearHeapAllocator<TPair<FString, FValue>>
    >;

    enum EType {
        Null = 0,
        Bool,
        Integer,
        Float,
        String,
        Array,
        Object,
    };

    template <EType _Type>
    using TType = Meta::TIntegralConstant<EType, _Type>;

    using TypeNull = TType<Null>;
    using TypeBool = TType<Bool>;
    using TypeInteger = TType<Integer>;
    using TypeFloat = TType<Float>;
    using TypeString = TType<String>;
    using TypeArray = TType<Array>;
    using TypeObject = TType<Object>;

    class FValue {
    public:
        FValue() : _type(Null) {}
        ~FValue() { Clear(); }

        explicit FValue(FJSON& doc, EType type);

        explicit FValue(FBool value) : _type(Bool), _bool(value) {}
        explicit FValue(FInteger value) : _type(Integer), _integer(value) {}
        explicit FValue(FFloat value) : _type(Float), _float(value) {}
        explicit FValue(FString&& value) : _type(String), _string(std::move(value)) {}
        explicit FValue(FArray&& value) : _type(Array), _array(std::move(value)) {}
        explicit FValue(FObject&& value) : _type(Object), _object(std::move(value)) {}

        FValue(const FValue& other) : FValue() { operator =(other); }
        FValue& operator =(const FValue& other);

        FValue(FValue&& rvalue) : FValue() { operator =(std::move(rvalue)); }
        FValue& operator =(FValue&& rvalue);

        EType Type() const { return _type; }

        FValue& SetType(FJSON& doc, EType type);

        void SetType_AssumeNull(FJSON& doc, TType<Null>);
        FBool& SetType_AssumeNull(FJSON& doc, TType<Bool>);
        FInteger& SetType_AssumeNull(FJSON& doc, TType<Integer>);
        FFloat& SetType_AssumeNull(FJSON& doc, TType<Float>);
        FString& SetType_AssumeNull(FJSON& doc, TType<String>);
        FArray& SetType_AssumeNull(FJSON& doc, TType<Array>);
        FObject& SetType_AssumeNull(FJSON& doc, TType<Object>);

        void SetValue(FBool value);
        void SetValue(FInteger value);
        void SetValue(FFloat value);
        void SetValue(FString&& value);
        void SetValue(FArray&& value);
        void SetValue(FObject&& value);

        bool AsNull() const { return (_type == Null); }
        const FBool* AsBool() const { return (_type == Bool ? &_bool : nullptr); }
        const FInteger* AsInteger() const { return (_type == Integer ? &_integer : nullptr); }
        const FFloat* AsFloat() const { return (_type == Float ? &_float : nullptr); }
        const FString* AsString() const { return (_type == String ? &_string : nullptr); }
        const FArray* AsArray() const { return (_type == Array ? &_array : nullptr); }
        const FObject* AsObject() const { return (_type == Object ? &_object : nullptr); }

        FBool& ToBool() { Assert(_type == Bool); return _bool; }
        FInteger& ToInteger() { Assert(_type == Integer); return _integer; }
        FFloat& ToFloat() { Assert(_type == Float); return _float; }
        FString& ToString() { Assert(_type == String); return _string; }
        FArray& ToArray() { Assert(_type == Array); return _array; }
        FObject& ToObject() { Assert(_type == Object); return _object; }

        const FBool& ToBool() const { Assert(_type == Bool); return _bool; }
        const FInteger& ToInteger() const { Assert(_type == Integer); return _integer; }
        const FFloat& ToFloat() const { Assert(_type == Float); return _float; }
        const FString& ToString() const { Assert(_type == String); return _string; }
        const FArray& ToArray() const { Assert(_type == Array); return _array; }
        const FObject& ToObject() const { Assert(_type == Object); return _object; }

        void Clear();

        bool Equals(const FValue& other) const;
        inline friend bool operator ==(const FValue& lhs, const FValue& rhs) { return lhs.Equals(rhs); }
        inline friend bool operator !=(const FValue& lhs, const FValue& rhs) { return (not operator ==(lhs, rhs)); }

        void ToStream(FTextWriter& oss, bool minify = true) const;

    private:
        EType _type;
        union {
            FBool _bool;
            FInteger _integer;
            FFloat _float;
            FString _string;
            FArray _array;
            FObject _object;
        };
    };

public:
    FJSON();
    ~FJSON();

    FJSON(const FJSON&) = delete;
    FJSON& operator =(const FJSON&) = delete;

    FValue& Root() { return _root; }
    const FValue& Root() const { return _root; }

    FString MakeString(const FStringView& str, bool mergeable = true);

    void ToStream(FTextWriter& oss, bool minify = false) const { _root.ToStream(oss, minify); }

    static bool Load(FJSON* json, const FFilename& filename);
    static bool Load(FJSON* json, const FFilename& filename, IBufferedStreamReader* input);
    static bool Load(FJSON* json, const FFilename& filename, const FStringView& content);

private:
    typedef TBasicStringViewHashSet<
        char, ECase::Sensitive,
        TLinearHeapAllocator<FStringView>
    >   stringtable_type;

    FLinearHeap _heap;
    FValue _root;
    stringtable_type _strings;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FTextWriter& operator <<(FTextWriter& oss, const Serialize::FJSON::FValue& jsonValue) {
    jsonValue.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
inline FTextWriter& operator <<(FTextWriter& oss, const Serialize::FJSON& json) {
    json.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
