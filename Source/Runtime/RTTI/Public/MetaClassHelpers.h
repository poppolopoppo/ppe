#pragma once

#include "MetaClass.h"
#include "MetaObjectHelpers.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T>
bool CreateMetaObject_(PMetaObject& dst, bool resetToDefaultValue, std::true_type ) {
    dst.reset(NEW_RTTI(T)());
    Assert(dst);
    if (resetToDefaultValue)
        ResetToDefaultValue(*dst);
    return true;
}
template <typename T>
FMetaObject* CreateMetaObject_(PMetaObject&, bool resetToDefaultValue, std::false_type) {
    return false;
}
inline void DeleteMetaClass_(FMetaClass* metaClass) {
    checked_delete(metaClass);
}
} //!details
//----------------------------------------------------------------------------
template <typename T>
class TInScopeMetaClass : public FMetaClass {
protected:
    TInScopeMetaClass(FClassId id, const FName& name, EClassFlags flags, const FMetaNamespace* metaNamespace)
        : FMetaClass(id, name, FlagsForT_(flags), metaNamespace)
    {}

public:
    virtual const FMetaClass* Parent() const override final {
        typedef typename TMetaClass<T>::parent_type parent_type;
        return RTTI::MetaClass<parent_type>();
    }

    virtual bool CreateInstance(PMetaObject& dst, bool resetToDefaultValue = true) const override final {
        Assert(not dst);
        return details::CreateMetaObject_<T>(dst, resetToDefaultValue, typename std::is_default_constructible<T>::type{});
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
        return new TMetaClass<T>(id, metaNamespace);
    }
};
//----------------------------------------------------------------------------
template <typename T>
const FMetaClassHandle TInScopeMetaClass<T>::GMetaClassHandle(
    TMetaClass<T>::Namespace(),
    &TInScopeMetaClass<T>::CreateMetaClass_,
    &details::DeleteMetaClass_
);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
