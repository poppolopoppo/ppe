#pragma once

#include "RTTI_fwd.h"

#include "MetaClass.h"
#include "MetaObjectHelpers.h"
#include "RTTI/NativeTypes.h"

#include "Allocator/TrackingMalloc.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Self, typename T>
class TInScopeMetaClass : public FMetaClass {
protected:
    TInScopeMetaClass(FClassId id, const FName& name, EClassFlags flags, const FMetaModule* module) NOEXCEPT
    :   FMetaClass(id, name, std::is_abstract<T>::value
            ? flags + EClassFlags::Abstract
            : flags + EClassFlags::Concrete,
        module ) {
        STATIC_ASSERT(std::is_base_of_v<FMetaObject, T>);
    }

public:
    class RTTI_FMetaClassHandle : public FMetaClassHandle {
    public:
        RTTI_FMetaClassHandle() NOEXCEPT
        :   FMetaClassHandle(
                TMetaClass<T>::Module(),
                &CreateMetaClass_,
                [](FMetaClass* metaClass) { TRACKING_DELETE(MetaClass, metaClass); })
        {}
    };

    virtual const FMetaClass* Parent() const NOEXCEPT override final {
        using parent_type = typename TMetaClass<T>::parent_type;
        return RTTI::MetaClass<parent_type>();
    }

    virtual bool CreateInstance(PMetaObject& dst, bool resetToDefaultValue = true) const override final {
        Assert(not dst);
        return RTTI::CreateMetaObject<T>(dst, resetToDefaultValue);
    }

    virtual PTypeTraits MakeTraits() const NOEXCEPT override final {
        return RTTI::MakeTraits<TRefPtr<T>>();
    }

    static const FMetaClass* Get() NOEXCEPT {
        const FMetaClassHandle& handle = _Self::metaclass_handle();
        Assert(handle.Class());
        return handle.Class();
    }

private:
    static FMetaClass* CreateMetaClass_(FClassId id, const FMetaModule* module) {
        return TRACKING_NEW(MetaClass, TMetaClass<T>) { id, module };
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
