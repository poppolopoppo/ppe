#pragma once

#include "RTTI.h"

#include "RTTI_fwd.h"

#include "Container/Vector.h"
#include "Memory/RefPtr.h"

namespace PPE {
namespace RTTI {
class FMetaClass;
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API bool Equals(const FMetaObject& lhs, const FMetaObject& rhs);
//----------------------------------------------------------------------------
PPE_RTTI_API bool DeepEquals(const FMetaObject& lhs, const FMetaObject& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API void Move(const FMetaObject& src, FMetaObject& dst);
//----------------------------------------------------------------------------
PPE_RTTI_API void Copy(const FMetaObject& src, FMetaObject& dst);
//----------------------------------------------------------------------------
PPE_RTTI_API void Swap(FMetaObject& lhs, FMetaObject& rhs);
//----------------------------------------------------------------------------
PPE_RTTI_API void Clone(const FMetaObject& src, PMetaObject& pdst);
//----------------------------------------------------------------------------
PPE_RTTI_API void DeepCopy(const FMetaObject& src, FMetaObject& dst);
//----------------------------------------------------------------------------
PPE_RTTI_API void DeepClone(const FMetaObject& src, PMetaObject& pdst);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API void ResetToDefaultValue(FMetaObject& obj);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API hash_t hash_value(const FMetaObject& obj);
//----------------------------------------------------------------------------
PPE_RTTI_API u128 Fingerprint128(const FMetaObject& obj);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T* Cast(FMetaObject* p) {
    Assert(p);
    const FMetaClass* const metaClass = MetaClass<T>();
    Assert(metaClass);
    return (p->RTTI_CastTo(*metaClass) ? checked_cast<T*>(p) : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
const T* Cast(const FMetaObject* p) {
    Assert(p);
    const FMetaClass* const metaClass = MetaClass<T>();
    Assert(metaClass);
    return (p->RTTI_CastTo(*metaClass) ? checked_cast<const T*>(p) : nullptr);
}
//----------------------------------------------------------------------------
template <typename T>
T* CastChecked(FMetaObject* p) {
#ifdef WITH_PPE_ASSERT_RELEASE
    T* const result = Cast<T>(p);
    AssertRelease(result);
    return result;
#else
    return reinterpret_cast<T*>(p);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
const T* CastChecked(const FMetaObject* p) {
#ifdef WITH_PPE_ASSERT_RELEASE
    const T* const result = Cast<T>(p);
    AssertRelease(result);
    return result;
#else
    return reinterpret_cast<const T*>(p);
#endif
}
//----------------------------------------------------------------------------
template <typename T>
auto Cast(const PMetaObject& p) {
    return Cast<T>(p.get());
}
//----------------------------------------------------------------------------
template <typename T>
auto CastChecked(const PMetaObject& p) {
    return Cast<T>(p.get());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using FReferencedObjects = VECTORINSITU(MetaObject, FMetaObject*, 8);
PPE_RTTI_API void CollectReferencedObjects(
    const FMetaObject& root,
    FReferencedObjects& references,
    size_t maxDepth = INDEX_NONE );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API FTextWriter& operator << (FTextWriter& oss, const RTTI::FMetaObject& obj);
PPE_RTTI_API FWTextWriter& operator << (FWTextWriter& oss, const RTTI::FMetaObject& obj);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
