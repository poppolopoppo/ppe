#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Meta/Singleton.h"

namespace Core {
namespace RTTI {
class MetaClassDatabase;
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class MetaClassSingleton : Meta::Singleton<typename T::MetaClass, typename T::MetaClass> {
    typedef Meta::Singleton<typename T::MetaClass, typename T::MetaClass> parent_type;
public:
    using parent_type::HasInstance;
    using parent_type::Instance;

    static void Create() {
        parent_type::Create();
        parent_type::Instance().Register(MetaClassDatabase::Instance());
    }

    static void Destroy() {
        parent_type::Instance().Unregister(MetaClassDatabase::Instance());
        parent_type::Destroy();
    }
};
//----------------------------------------------------------------------------
template <typename T> // valid RTTI parent
static const RTTI::MetaClass *GetMetaClass(typename std::enable_if< std::is_base_of<RTTI::MetaObject, T>::value >::type* = 0) {
    return &MetaClassSingleton<T>::Instance();
}
//----------------------------------------------------------------------------
template <typename T> // no parent
static const RTTI::MetaClass *GetMetaClass(typename std::enable_if< std::is_void<T>::value >::type* = 0) {
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
