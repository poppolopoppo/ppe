#pragma once

#include "RTTI_fwd.h"

#include "RTTI/Any.h"
#include "RTTI/NativeTypes.Definitions-inl.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Instantiate official "opaque" data containers, TRawInlineAllocator<> is used InSitu without defining FAny
namespace details {
constexpr int _sizeof_FAny = (4 * sizeof(intptr_t));
using FOpaqueArrayContainer_ = TVector<
    FAny,
    TRawInlineAllocator<MEMORYDOMAIN_TAG(OpaqueData), _sizeof_FAny * 3>
>;
using FOpaqueDataContainer_ = TAssociativeVector<
    FName, FAny,
    Meta::TEqualTo<FName>,
    TVector<
        TPair<FName, FAny>,
        TRawInlineAllocator<MEMORYDOMAIN_TAG(OpaqueData), (sizeof(FName) + _sizeof_FAny) * 3>
    >
 >;
} //!details
INSTANTIATE_CLASS_TYPEDEF(PPE_RTTI_API, FOpaqueArray, details::FOpaqueArrayContainer_);
INSTANTIATE_CLASS_TYPEDEF(PPE_RTTI_API, FOpaqueData, details::FOpaqueDataContainer_);
//----------------------------------------------------------------------------
PPE_RTTI_API FAny& GetOpaqueData(FOpaqueData* opaque, const FName& name) NOEXCEPT;
PPE_RTTI_API const FAny* GetOpaqueDataIFP(const FOpaqueData* opaque, const FName& name) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_RTTI_API bool TryGetOpaqueData(const FOpaqueData* opaque, const FName& name, const FAtom& dst) NOEXCEPT;
//----------------------------------------------------------------------------
PPE_RTTI_API void SetOpaqueDataCopy(FOpaqueData* opaque, const FName& name, const FAtom& value);
PPE_RTTI_API void SetOpaqueDataMove(FOpaqueData* opaque, const FName& name, const FAtom& value);
//----------------------------------------------------------------------------
PPE_RTTI_API void UnsetOpaqueData(FOpaqueData* opaque, const FName& name);
//----------------------------------------------------------------------------
PPE_RTTI_API void MergeOpaqueData(FOpaqueData* dst, const FOpaqueData& src, bool allowOverride = false);
//----------------------------------------------------------------------------
// These helpers allow to manipulate opaque data with native types without knowing the RTTI layer
#define PPE_RTTI_OPAQUEDATA_NATIVETYPE_DECL(_Name, _Type, _Uid) \
    PPE_RTTI_API bool TryGetOpaqueData(const FOpaqueData* opaque, const FName& name, _Type* dst) NOEXCEPT; \
    PPE_RTTI_API void SetOpaqueData(FOpaqueData* opaque, const FName& name, const _Type& value); \
    PPE_RTTI_API void SetOpaqueData(FOpaqueData* opaque, const FName& name, _Type&& value);
FOREACH_RTTI_NATIVETYPES(PPE_RTTI_OPAQUEDATA_NATIVETYPE_DECL)
#undef PPE_RTTI_OPAQUEDATA_NATIVETYPE_DECL
//----------------------------------------------------------------------------
#define DEF_RTTI_ALIASING_OPAQUE_TRAITS(_FROM, ...) \
    CONSTEXPR auto/* forward-declaration */ TypeInfos(TType< _FROM >) { \
        return FTypeHelpers::Alias< _FROM, __VA_ARGS__ >; \
    } \
    inline PTypeTraits Traits(TType< _FROM >) NOEXCEPT { \
        return Traits(Type< __VA_ARGS__ >); \
    }
DEF_RTTI_ALIASING_OPAQUE_TRAITS(FOpaqueArray, details::FOpaqueArrayContainer_)
DEF_RTTI_ALIASING_OPAQUE_TRAITS(FOpaqueData, details::FOpaqueDataContainer_)
#undef DEF_RTTI_ALIASING_OPAQUE_TRAITS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
