#pragma once

#include "RTTI_fwd.h"

#include "RTTI_fwd.h"
#include "RTTI/Typedefs.h"

#include "Memory/RefPtr.h"
#include "Misc/Function_fwd.h"

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
namespace details {
template <typename T, typename _Any>
bool CreateMetaObject_(PMetaObject& dst, std::true_type, _Any) {
    dst = NEW_RTTI(T) { ConstructorTag };
    return true;
}
template <typename T>
bool CreateMetaObject_(PMetaObject& dst, std::false_type, std::true_type) {
    dst = NEW_RTTI(T) {};
    return true;
}
template <typename T>
bool CreateMetaObject_(PMetaObject&, std::false_type, std::false_type) {
    return false;
}
} //!namespace details
//----------------------------------------------------------------------------
#if USE_PPE_RTTI_CHECKS
PPE_RTTI_API void CheckMetaClassAllocation(const FMetaClass* metaClass);
#endif
//----------------------------------------------------------------------------
template <typename T>
bool CreateMetaObject(PMetaObject& dst, bool resetToDefaultValue) {
#if USE_PPE_RTTI_CHECKS
    CheckMetaClassAllocation(MetaClass<T>());
#endif

    if (details::CreateMetaObject_<T>(
            dst,
            Meta::has_constructor<T, FConstructorTag>{},
            typename std::is_default_constructible<T>::type{})) {
        Assert(dst);

        if (resetToDefaultValue)
            ResetToDefaultValue(*dst);

        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_RTTI_API size_t CollectReferences(
    const FMetaObject& root,
    TFunction<bool(const ITypeTraits&, FMetaObject&)>&& prefix,
    TFunction<bool(const ITypeTraits&, FMetaObject&)>&& postfix,
    EVisitorFlags flags);
//----------------------------------------------------------------------------
PPE_RTTI_API size_t CollectReferences(
    const TMemoryView<const PMetaObject>& roots,
    TFunction<bool(const ITypeTraits&, FMetaObject&)>&& prefix,
    TFunction<bool(const ITypeTraits&, FMetaObject&)>&& postfix,
    EVisitorFlags flags);
//----------------------------------------------------------------------------
PPE_RTTI_API bool CheckCircularReferences(const FMetaObject& root);
PPE_RTTI_API bool CheckCircularReferences(const TMemoryView<const PMetaObject>& roots);
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
