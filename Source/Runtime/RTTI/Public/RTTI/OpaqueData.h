#pragma once

#include "RTTI_fwd.h"

#include "RTTI/NativeTypes.Definitions-inl.h"

#include "Container/AssociativeVector.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FOpaqueData = ASSOCIATIVE_VECTOR(OpaqueData, FName, FAny);
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
