#pragma once

#include "RTTI_fwd.h"

#include "Allocator/TrackingMalloc.h"
#include "MetaClass.h"
#include "MetaObjectHelpers.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TInScopeMetaClass : public FMetaClass {
protected:
    TInScopeMetaClass(FClassId id, const FName& name, EClassFlags flags, const FMetaNamespace* metaNamespace)
    :   FMetaClass(id, name, FlagsForT_(flags), metaNamespace) {
        STATIC_ASSERT(std::is_base_of_v<FMetaObject, T>);
    }

public:
    virtual const FMetaClass* Parent() const override final {
        typedef typename TMetaClass<T>::parent_type parent_type;
        return RTTI::MetaClass<parent_type>();
    }

    virtual bool CreateInstance(PMetaObject& dst, bool resetToDefaultValue = true) const override final {
        Assert(not dst);
        return CreateMetaObject<T>(dst, resetToDefaultValue);
    }

    static const FMetaClass* Get() {
        Assert(GMetaClassHandle.Class());
        return GMetaClassHandle.Class();
    }

private:
    static const FMetaClassHandle GMetaClassHandle;

    static constexpr EClassFlags FlagsForT_(EClassFlags flags) {
        return (std::is_abstract<T>::value
            ? flags + EClassFlags::Abstract
            : flags + EClassFlags::Concrete );
    }

    static FMetaClass* CreateMetaClass_(FClassId id, const FMetaNamespace* metaNamespace) {
        return TRACKING_NEW(MetaClass, TMetaClass<T>) { id, metaNamespace };
    }
};
//----------------------------------------------------------------------------
template <typename T>
const FMetaClassHandle TInScopeMetaClass<T>::GMetaClassHandle(
    TMetaClass<T>::Namespace(),
    &TInScopeMetaClass<T>::CreateMetaClass_,
    [](FMetaClass* metaClass) { TRACKING_DELETE(MetaClass, metaClass); }
);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
