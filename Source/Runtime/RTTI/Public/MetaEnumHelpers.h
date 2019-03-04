#pragma once

#include "RTTI_fwd.h"

#include "Allocator/TrackingMalloc.h"
#include "MetaEnum.h"
#include "MetaNamespace.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TInScopeMetaEnum : public FMetaEnum {
protected:
    TInScopeMetaEnum(const FName& name, EEnumFlags flags, const FMetaNamespace* metaNamespace)
    :   FMetaEnum(name, flags, sizeof(T), metaNamespace) {
        STATIC_ASSERT(std::is_enum_v<T>);
    }

    virtual PTypeTraits MakeTraits() const override final {
        return RTTI::MakeTraits<T>();
    }

public:
    static const FMetaEnum* Get() {
        Assert(GMetaEnumHandle.Enum());
        return GMetaEnumHandle.Enum();
    }

private:
    static const FMetaEnumHandle GMetaEnumHandle;

    static FMetaEnum* CreateMetaEnum_(const FMetaNamespace* metaNamespace) {
        return TRACKING_NEW(MetaEnum, TMetaEnum<T>) { metaNamespace };
    }
};
//----------------------------------------------------------------------------
template <typename T>
const FMetaEnumHandle TInScopeMetaEnum<T>::GMetaEnumHandle(
    TMetaEnum<T>::Namespace(),
    &TInScopeMetaEnum<T>::CreateMetaEnum_,
    [](FMetaEnum* metaEnum) { TRACKING_DELETE(MetaEnum, metaEnum); }
);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
