#pragma once

#include "RTTI_fwd.h"

#include "Allocator/TrackingMalloc.h"
#include "MetaEnum.h"
#include "MetaModule.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Self, typename T>
class TInScopeMetaEnum : public FMetaEnum {
protected:
    TInScopeMetaEnum(const FName& name, EEnumFlags flags, const FMetaModule* metaNamespace) NOEXCEPT
    :   FMetaEnum(name, flags, sizeof(T), metaNamespace) {
        STATIC_ASSERT(std::is_enum_v<T>);
    }

    virtual PTypeTraits MakeTraits() const NOEXCEPT override final {
        return RTTI::MakeTraits<T>();
    }

public:
    class RTTI_FMetaEnumHandle : public FMetaEnumHandle {
    public:
        RTTI_FMetaEnumHandle() NOEXCEPT
            : FMetaEnumHandle(
                TMetaEnum<T>::Module(),
                CreateMetaEnum_,
                [](FMetaEnum* metaEnum) { TRACKING_DELETE(MetaEnum, metaEnum); })
        {}
    };

    static const FMetaEnum* Get() NOEXCEPT {
        const RTTI_FMetaEnumHandle& handle = _Self::metaenum_handle();
        Assert(handle.Enum());
        return handle.Enum();
    }

private:
    static FMetaEnum* CreateMetaEnum_(const FMetaModule* metaNamespace) {
        return TRACKING_NEW(MetaEnum, TMetaEnum<T>) { metaNamespace };
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
