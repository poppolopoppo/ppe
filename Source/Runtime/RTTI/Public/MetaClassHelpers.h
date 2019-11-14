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
template <typename T>
class TInScopeMetaClass : public FMetaClass {
protected:
    TInScopeMetaClass(FClassId id, const FName& name, EClassFlags flags, const FMetaModule* module)
    :   FMetaClass(id, name, std::is_abstract<T>::value
            ? flags + EClassFlags::Abstract
            : flags + EClassFlags::Concrete,
        module ) {
        STATIC_ASSERT(std::is_base_of_v<FMetaObject, T>);
    }

public:
    virtual const FMetaClass* Parent() const override final {
        typedef typename TMetaClass<T>::parent_type parent_type;
        return RTTI::MetaClass<parent_type>();
    }

    virtual bool CreateInstance(PMetaObject& dst, bool resetToDefaultValue = true) const override final {
        Assert(not dst);
        return RTTI::CreateMetaObject<T>(dst, resetToDefaultValue);
    }

    virtual PTypeTraits MakeTraits() const override final {
        return RTTI::MakeTraits<TRefPtr<T>>();
    }

    static const FMetaClass* Get() {
        Assert(GMetaClassHandle.Class());
        return GMetaClassHandle.Class();
    }

private:
    static const FMetaClassHandle GMetaClassHandle;

    static FMetaClass* CreateMetaClass_(FClassId id, const FMetaModule* module) {
        return TRACKING_NEW(MetaClass, TMetaClass<T>) { id, module };
    }
};
//----------------------------------------------------------------------------
template <typename T>
const FMetaClassHandle TInScopeMetaClass<T>::GMetaClassHandle(
    TMetaClass<T>::Module(),
    &TInScopeMetaClass<T>::CreateMetaClass_,
    [](FMetaClass* metaClass) { TRACKING_DELETE(MetaClass, metaClass); }
);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
