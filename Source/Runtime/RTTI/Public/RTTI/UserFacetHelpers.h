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
    virtual FWTextWriter& Description(FWTextWriter& oss) const override;
#endif

private:
    const void* _data;
    const _Facet& _facet;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_RTTI_API FDescriptionFacet : FGenericUserFacet {
    FConstChar Text;

    FDescriptionFacet(FConstChar text) NOEXCEPT;

    template <typename _Meta>
    static FStringView GetIFP(const _Meta& meta) NOEXCEPT {
        if (auto* const facet = meta.Facets().template GetIFP<FDescriptionFacet>())
            return facet->Text.MakeView();
        return Default;
    }
};
//----------------------------------------------------------------------------
struct PPE_RTTI_API FDependencyInjectionFacet : FGenericUserFacet {
    Meta::type_info_t ServiceKey;

    template <typename T>
    CONSTEXPR FDependencyInjectionFacet(Meta::TType<T>)
    :   FDependencyInjectionFacet(Meta::type_info<T>) {
        STATIC_ASSERT(std::is_base_of_v<FMetaObject, T>);
    }

    CONSTEXPR explicit FDependencyInjectionFacet(Meta::type_info_t&& serviceKey)
    :   ServiceKey(std::move(serviceKey))
    {}

    void Decorate(const FMetaProperty& meta, void* data) const NOEXCEPT;
};
//----------------------------------------------------------------------------
struct PPE_RTTI_API FEnumFacet : FGenericUserFacet {
    VECTORINSITU(UserFacet, FAny, 3) Values;

    template <typename T>
    FEnumFacet(std::initializer_list<T> values)
    :   Values(values) {
        Assert_NoAssume(not Values.empty());
    }

    using FValidateException = TPropertyFacetException<FEnumFacet>;
    void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FWTextWriter& TPropertyFacetException<FEnumFacet>::Description(FWTextWriter& oss) const;
#endif
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TPropertyFacetException<FEnumFacet>;
//----------------------------------------------------------------------------
struct PPE_RTTI_API FFormatFacet : FGenericUserFacet {
    EBuiltinRegexp Format;

    FFormatFacet(EBuiltinRegexp format) NOEXCEPT;

    using FValidateException = TPropertyFacetException<FFormatFacet>;
    void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FWTextWriter& TPropertyFacetException<FFormatFacet>::Description(FWTextWriter& oss) const;
#endif
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TPropertyFacetException<FFormatFacet>;
//----------------------------------------------------------------------------
struct PPE_RTTI_API FLengthFacet : FGenericUserFacet {
    size_t MinLength{ 0 };
    size_t MaxLength{ TNumericLimits<size_t>::MaxValue() };

    FLengthFacet(size_t length) NOEXCEPT : FLengthFacet(length, length) {}
    FLengthFacet(size_t minLength, size_t maxLength) NOEXCEPT;

    using FValidateException = TPropertyFacetException<FLengthFacet>;
    void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FWTextWriter& TPropertyFacetException<FLengthFacet>::Description(FWTextWriter& oss) const;
#endif
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TPropertyFacetException<FLengthFacet>;
//----------------------------------------------------------------------------
struct PPE_RTTI_API FPatternFacet : FGenericUserFacet {
    FConstChar Pattern;

    FPatternFacet(FConstChar pattern) NOEXCEPT;

    using FValidateException = TPropertyFacetException<FPatternFacet>;
    void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FWTextWriter& TPropertyFacetException<FPatternFacet>::Description(FWTextWriter& oss) const;
#endif
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TPropertyFacetException<FPatternFacet>;
//----------------------------------------------------------------------------
struct PPE_RTTI_API FRangeFacet : FGenericUserFacet {
    FAny Minimum;
    FAny Maximum;

    template <typename T, class = Meta::TEnableIf<Meta::has_trivial_compare_v<T>>>
    FRangeFacet(T&& minimum, T&& maximum)
    :   Minimum(std::move(minimum))
    ,   Maximum(std::move(maximum)) {
        Assert_NoAssume(Minimum.Traits()->AsScalar());
    }

    void Decorate(const FMetaProperty& meta, void* data) const NOEXCEPT;

    using FValidateException = TPropertyFacetException<FRangeFacet>;
    void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FWTextWriter& TPropertyFacetException<FRangeFacet>::Description(FWTextWriter& oss) const;
#endif
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TPropertyFacetException<FRangeFacet>;
//----------------------------------------------------------------------------
struct PPE_RTTI_API FUniqueFacet : FGenericUserFacet {
    FUniqueFacet() = default;

    using FValidateException = TPropertyFacetException<FUniqueFacet>;
    void Validate(const FMetaProperty& meta, const void* data) const;
};
#if USE_PPE_EXCEPTION_DESCRIPTION
template <>
PPE_RTTI_API FWTextWriter& TPropertyFacetException<FUniqueFacet>::Description(FWTextWriter& oss) const;
#endif
EXTERN_TEMPLATE_CLASS_DEF(PPE_RTTI_API) TPropertyFacetException<FUniqueFacet>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //namespace RTTI
} //namespace PPE
