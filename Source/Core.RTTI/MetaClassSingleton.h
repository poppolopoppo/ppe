#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Meta/Singleton.h"

namespace Core {
namespace RTTI {
class MetaClass;
class MetaClassDatabase;
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename = typename std::enable_if< std::is_base_of<RTTI::MetaObject, T>::value >::type >
class MetaClassSingleton {
public:
    typedef T object_type;
    typedef typename T::MetaClass metaclass_type;
    STATIC_ASSERT(std::is_base_of<RTTI::MetaClass, metaclass_type>::value);

    static bool HasInstance() { return (nullptr != _gInstance); }
    static const metaclass_type& Instance() { Assert(_gInstance); return *_gInstance; }

    static void Create() {
        AssertIsMainThread();
        Assert(nullptr == _gInstance);
        _gInstance.reset(new metaclass_type());
        _gInstance->Register(MetaClassDatabase::Instance());
    }

    static void Destroy() {
        AssertIsMainThread();
        Assert(nullptr != _gInstance);
        _gInstance->Unregister(MetaClassDatabase::Instance());
        _gInstance.reset(nullptr);
    }

private:
    static UniquePtr<const metaclass_type> _gInstance;
};
//----------------------------------------------------------------------------
template <typename T, typename _Enabled >
UniquePtr<const typename T::MetaClass> MetaClassSingleton<T, _Enabled>::_gInstance = nullptr;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T> // valid RTTI parent
static const RTTI::MetaClass *GetMetaClass(typename std::enable_if< std::is_base_of<RTTI::MetaObject, T>::value >::type* = 0) {
    typedef typename std::decay<T>::type metaobject_type;
    return &MetaClassSingleton<metaobject_type>::Instance();
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
