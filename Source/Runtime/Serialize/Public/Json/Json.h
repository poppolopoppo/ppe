#pragma once

#include "Serialize.h"

#include "Lexer/Location.h"
#include "SerializeExceptions.h"

#include "RTTI_fwd.h"
#include "RTTI/Any.h"
#include "RTTI/OpaqueData.h"

#include "Container/AssociativeVector.h"
#include "IO/Filename.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
class IBufferedStreamReader;
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FJsonException : public PPE::Serialize::FSerializeException {
public:
    typedef FSerializeException parent_type;

    FJsonException(const char *what)
        : FJsonException(what, Lexer::FLocation()) {}

    FJsonException(const char *what, const Lexer::FLocation& site)
        : parent_type(what), _site(site) {}

    virtual ~FJsonException() = default;

    const Lexer::FLocation& Site() const { return _site; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    virtual FWTextWriter& Description(FWTextWriter& oss) const override final;
#endif

private:
    Lexer::FLocation _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_SERIALIZE_API FJson {
public:
    class FValue;

    template <typename T, size_t N>
    using TJsonInlineAllocator = TRawInlineAllocator<
        MEMORYDOMAIN_TAG(Json),
        sizeof(T) * N
    >;

    using FNull = RTTI::PMetaObject;
    using FBool = bool;
    using FInteger = i64;
    using FFloat = double;
    using FText = FString;
    using FArray = TVector<FValue, TJsonInlineAllocator<RTTI::FAny, 3> >; //RTTI::FOpaqueArray;
    using FObject = TAssociativeVector<
        RTTI::FName, FValue,
        Meta::TEqualTo<RTTI::FName>,
        TVector<
            TPair<RTTI::FName, FValue>,
            TJsonInlineAllocator<TPair<RTTI::FName, RTTI::FAny>, 3>
        >
    > ; //RTTI::FOpaqueData;

    enum EType {
        Null = 0,
        Bool,
        Integer,
        Float,
        String,
        Array,
        Object,
    };

    template <typename T>
    static CONSTEXPR bool IsKnownType{
        std::is_same_v<T, FNull> ||
        std::is_same_v<T, FBool> ||
        std::is_same_v<T, FInteger> ||
        std::is_same_v<T, FFloat> ||
        std::is_same_v<T, FText> ||
        std::is_same_v<T, FArray> ||
        std::is_same_v<T, FObject>
        };

    class FValue : private RTTI::FAny {
    public:
        FValue() = default;

        FValue(const FValue&) = default;
        FValue& operator =(const FValue&) = default;

        FValue(FValue&&) = default;
        FValue& operator =(FValue&&) = default;

        explicit FValue(EType type) NOEXCEPT;

        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        FValue(T&& rvalue) NOEXCEPT : RTTI::FAny(std::move(rvalue)) {}
        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        FValue(const T& value) NOEXCEPT : RTTI::FAny(value) {}

        operator RTTI::FAny& () { return (*this);  }
        operator const RTTI::FAny& () const { return (*this); }

        RTTI::FTypeId TypeId() const { return RTTI::FAny::Traits()->TypeId(); }

        void Reset() { RTTI::FAny::Reset(); }

        friend bool operator ==(const FValue& lhs, const FValue& rhs) NOEXCEPT { return lhs.Equals(rhs); }
        friend bool operator !=(const FValue& lhs, const FValue& rhs) NOEXCEPT { return not operator ==(lhs, rhs); }

        friend hash_t hash_value(const FValue& v) { return v.HashValue(); }

        // restrict FAny to Json types

        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        void Assign(T&& rvalue) { RTTI::FAny::Assign(std::move(rvalue)); }
        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        void Assign(const T& value) { RTTI::FAny::Assign(value); }

        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        T& MakeDefault_AssumeNotValid() { return RTTI::FAny::MakeDefault_AssumeNotValid<T>();  }

        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        T& FlatData() { return RTTI::FAny::FlatData<T>(); }
        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        const T& FlatData() const { return RTTI::FAny::FlatData<T>(); }

        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        T& TypedData() const { return RTTI::FAny::TypedData<T>(); }
        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        const T& TypedConstData() const { return RTTI::FAny::TypedConstData<T>(); }
        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        T* TypedDataIFP() const { return RTTI::FAny::TypedDataIFP<T>(); }
        template <typename T, class = Meta::TEnableIf< IsKnownType<T> > >
        const T* TypedConstDataIFP() const { return RTTI::FAny::TypedConstDataIFP<T>(); }

    };

    // assumes FValue is a FAny, as far as RTTI will ever know
    friend CONSTEXPR RTTI::PTypeInfos TypeInfos(RTTI::TType<FValue>) {
        return RTTI::FTypeHelpers::Alias<FValue, RTTI::FAny>;
    }
    friend RTTI::PTypeTraits Traits(RTTI::TType<FValue>) NOEXCEPT {
        return RTTI::Traits(RTTI::Type<RTTI::FAny>);
    }

    static FValue MakeValue(EType type) NOEXCEPT;

public:
    FJson() = default;

    FJson(const FJson&) = delete;
    FJson& operator =(const FJson&) = delete;

    FJson(FJson&&) = default;
    FJson& operator =(FJson&&) = default;

    FValue& Root() { return _root; }
    const FValue& Root() const { return _root; }

    void ToStream(FTextWriter& oss, bool minify = false) const;
    void ToStream(FWTextWriter& oss, bool minify = false) const;

    static bool Load(FJson* json, const FFilename& filename);
    static bool Load(FJson* json, const FFilename& filename, IBufferedStreamReader* input);
    static bool Load(FJson* json, const FWStringView& filename, IBufferedStreamReader* input);
    static bool Load(FJson* json, const FWStringView& filename, const FStringView& content);

private:
    FValue _root;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const Serialize::FJson& json) {
    json.ToStream(oss);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
