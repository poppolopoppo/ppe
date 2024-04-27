#pragma once

#include "MetaClass.h"
#include "RTTI_fwd.h"

#include "RTTI/Any.h"
#include "RTTI/Exceptions.h"
#include "RTTI/UserFacet.h"

#include "Meta/TypeInfo.h"

// #TODO: virtual facets manipulated through interfaces to use native code in Decorate/Validate() instead of RTTI layer? (#PERF)

namespace PPE {
enum class EBuiltinRegexp : u8; // IO/regexp.h
namespace RTTI {
EXTERN_LOG_CATEGORY(PPE_RTTI_API, RTTI)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Facet>
class TPropertyFacetException final : public FPropertyException {
public:
    TPropertyFacetException(const char* what, const FMetaProperty& meta, const void* data, const _Facet& facet)
    :   FPropertyException(what, &meta)
    ,   _data(data)
    ,   _facet(facet)
    {}

    const void* Data() const { return _data; }
    const _Facet& Facet() const { return _facet; }

#if USE_PPE_EXCEPTION_DESCRIPTION
    PPE_RTTI_API virtual FTextWriter& Description(FTextWriter& oss) const override;
#endif

private:
    const void* _data;
    const _Facet& _facet;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FDescriptionFacet : FGenericUserFacet {
    FStringLiteral Text;

    PPE_RTTI_API FDescriptionFacet(FStringLiteral text) NOEXCEPT;

    template <typename _Meta>
    static FStringLiteral GetIFP(const _Meta& meta) NOEXCEPT {
        if (auto* const facet = meta.Facets().template GetIFP<FDescriptionFacet>())
            return facet->Text;
        return Default;
    }
};
//----------------------------------------------------------------------------
struct FDependencyInjectionFacet : FGenericUserFacet {
    Meta::type_info_t ServiceKey;

    template <typename T>
    CONSTEXPR FDependencyInjectionFacet(Meta::TType<T>)
    :   FDependencyInjectionFacet(Meta::type_info<T>) {
        STATIC_ASSERT(std::is_base_of_v<FMetaObject, T>);
    }

    CONSTEXPR explicit FDependencyInjectionFacet(Meta::type_info_t&& serviceKey)
    :   ServiceKey(std::move(serviceKey))
    {}

    PPE_RTTI_API void Decorate(const FMetaProperty& meta, void* data) const NOEXCEPT;
};
//----------------------------------------------------------------------------
struct FEnumFacet : FGenericUserFacet {
    VECTORINSITU(UserFacet, FAny, 3) Values;

    template <typename T>
    FEnumFacet(std::initializer_list<T> values)
    :   Values(values) {
        Assert_NoAssume(not Values.empty());
    }

    using FValidateException = TPropertyFacetException<FEnumFacet>;
    PPE_RTTI_API void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FTextWriter& TPropertyFacetException<FEnumFacet>::Description(FTextWriter& oss) const;
#endif
template class TPropertyFacetException<FEnumFacet>;
//----------------------------------------------------------------------------
struct FFormatFacet : FGenericUserFacet {
    EBuiltinRegexp Format;

    PPE_RTTI_API FFormatFacet(EBuiltinRegexp format) NOEXCEPT;

    using FValidateException = TPropertyFacetException<FFormatFacet>;
    PPE_RTTI_API void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FTextWriter& TPropertyFacetException<FFormatFacet>::Description(FTextWriter& oss) const;
#endif
template class TPropertyFacetException<FFormatFacet>;
//----------------------------------------------------------------------------
struct FLengthFacet : FGenericUserFacet {
    size_t MinLength{ 0 };
    size_t MaxLength{ TNumericLimits<size_t>::MaxValue() };

    FLengthFacet(size_t length) NOEXCEPT : FLengthFacet(length, length) {}
    PPE_RTTI_API FLengthFacet(size_t minLength, size_t maxLength) NOEXCEPT;

    using FValidateException = TPropertyFacetException<FLengthFacet>;
    PPE_RTTI_API void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FTextWriter& TPropertyFacetException<FLengthFacet>::Description(FTextWriter& oss) const;
#endif
template class TPropertyFacetException<FLengthFacet>;
//----------------------------------------------------------------------------
struct FPatternFacet : FGenericUserFacet {
    FStringLiteral Pattern;

    PPE_RTTI_API FPatternFacet(FStringLiteral pattern) NOEXCEPT;

    using FValidateException = TPropertyFacetException<FPatternFacet>;
    PPE_RTTI_API void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FTextWriter& TPropertyFacetException<FPatternFacet>::Description(FTextWriter& oss) const;
#endif
template class TPropertyFacetException<FPatternFacet>;
//----------------------------------------------------------------------------
struct FRangeFacet : FGenericUserFacet {
    FAny Minimum;
    FAny Maximum;

    template <typename T, class = Meta::TEnableIf<Meta::has_trivial_compare_v<T>>>
    FRangeFacet(T&& minimum, T&& maximum)
    :   Minimum(std::move(minimum))
    ,   Maximum(std::move(maximum)) {
        Assert_NoAssume(Minimum.Traits()->AsScalar());
    }

    PPE_RTTI_API void Decorate(const FMetaProperty& meta, void* data) const NOEXCEPT;

    using FValidateException = TPropertyFacetException<FRangeFacet>;
    PPE_RTTI_API void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FTextWriter& TPropertyFacetException<FRangeFacet>::Description(FTextWriter& oss) const;
#endif
template class TPropertyFacetException<FRangeFacet>;
//----------------------------------------------------------------------------
struct FUniqueFacet : FGenericUserFacet {
    FUniqueFacet() = default;

    using FValidateException = TPropertyFacetException<FUniqueFacet>;
    PPE_RTTI_API void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FTextWriter& TPropertyFacetException<FUniqueFacet>::Description(FTextWriter& oss) const;
#endif
template class TPropertyFacetException<FUniqueFacet>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //namespace RTTI
} //namespace PPE
