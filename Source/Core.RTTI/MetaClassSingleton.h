#pragma once

#include "Core.RTTI/RTTI.h"

#include "Core/Meta/Singleton.h"

namespace Core {
namespace RTTI {
class FMetaClass;
class FMetaClassDatabase;
FWD_REFPTR(MetaObject);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename = typename std::enable_if< std::is_base_of<RTTI::FMetaObject, T>::value >::type >
class TMetaClassSingleton {
public:
    typedef T object_type;
    typedef typename T::FMetaClass metaclass_type;
    STATIC_ASSERT(std::is_base_of<RTTI::FMetaClass, metaclass_type>::value);

    static bool HasInstance() { return (nullptr != _gInstance); }
    static const metaclass_type& Instance() { Assert(_gInstance); return *_gInstance; }

    static void Create() {
        AssertIsMainThread();
        Assert(nullptr == _gInstance);
        _gInstance = new metaclass_type();
        _gInstance->Register(FMetaClassDatabase::Instance());
    }

    static void Destroy() {
        AssertIsMainThread();
        Assert(nullptr != _gInstance);
        _gInstance->Unregister(FMetaClassDatabase::Instance());
        checked_delete(_gInstance);
        _gInstance = nullptr;
    }

private:
    static const metaclass_type* _gInstance;
};
//----------------------------------------------------------------------------
template <typename T, typename _Enabled >
const typename T::FMetaClass* TMetaClassSingleton<T, _Enabled>::_gInstance = nullptr;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T> // valid RTTI parent
static const RTTI::FMetaClass *GetMetaClass(typename std::enable_if< std::is_base_of<RTTI::FMetaObject, T>::value >::type* = 0) {
    typedef Meta::TDecay<T> metaobject_type;
    return &TMetaClassSingleton<metaobject_type>::Instance();
}
//----------------------------------------------------------------------------
template <typename T> // no parent
static const RTTI::FMetaClass *GetMetaClass(typename std::enable_if< std::is_void<T>::value >::type* = 0) {
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace Core
