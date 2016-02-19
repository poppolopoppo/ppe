#pragma once

#include "Core/Core.h"

#include "Core/Memory/AlignedStorage.h"
#include "Core/Meta/Activator.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
class SingletonHelpers {
protected:
    SingletonHelpers();

public:
    ~SingletonHelpers();

    SingletonHelpers(SingletonHelpers&& ) = delete;
    SingletonHelpers& operator =(SingletonHelpers&& ) = delete;

    SingletonHelpers(const SingletonHelpers& ) = delete;
    SingletonHelpers& operator =(const SingletonHelpers& ) = delete;

    static T& Instance();
    static bool HasInstance();

    template <typename... _Args>
    static void Create(_Args&&... args);
    static void Destroy();
};
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
SingletonHelpers<T, _Impl>::SingletonHelpers() {}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
SingletonHelpers<T, _Impl>::~SingletonHelpers() {
    Assert(this == _Impl::_gInstancePtr);
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
T& SingletonHelpers<T, _Impl>::Instance() {
    Assert(_Impl::_gInstancePtr);
    Likely(_Impl::_gInstancePtr);
    return *_Impl::_gInstancePtr;
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
bool SingletonHelpers<T, _Impl>::HasInstance() {
    return (nullptr != _Impl::_gInstancePtr);
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
template <typename... _Args>
void SingletonHelpers<T, _Impl>::Create(_Args&&... args) {
    AssertRelease(nullptr == _Impl::_gInstancePtr);
    _Impl::_gInstancePtr = ::new (_Impl::Storage_()) T(std::forward<_Args>(args)... );
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
void SingletonHelpers<T, _Impl>::Destroy() {
    AssertRelease(nullptr != _Impl::_gInstancePtr);
    Assert(_Impl::Storage_() == _Impl::_gInstancePtr);
    _Impl::_gInstancePtr->~T();
    _Impl::_gInstancePtr = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T>
class Singleton : SingletonHelpers<T, Singleton<T, _Tag> > {
protected:
    Singleton() {}

public:
    friend class SingletonHelpers<T, Singleton<T, _Tag> >;
    typedef SingletonHelpers<T, Singleton<T, _Tag> > parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Create;
    using parent_type::Destroy;

private:
    static void* Storage_();
    static T *_gInstancePtr;
};
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
void* Singleton<T, _Tag>::Storage_() {
    typedef typename POD_STORAGE(T) pod_type;
    STATIC_ASSERT(std::is_pod<pod_type>::value);
    static pod_type _gStorage;
    return &_gStorage;
}
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
T *Singleton<T, _Tag>::_gInstancePtr = nullptr;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T>
class ThreadLocalSingleton : SingletonHelpers<T, ThreadLocalSingleton<T, _Tag> > {
protected:
    ThreadLocalSingleton() {}

public:
    friend class SingletonHelpers<T, ThreadLocalSingleton<T, _Tag> >;
    typedef SingletonHelpers<T, ThreadLocalSingleton<T, _Tag> > parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Create;
    using parent_type::Destroy;

private:
    static void* Storage_();
    static THREAD_LOCAL T *_gInstancePtr;
};
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
void* ThreadLocalSingleton<T, _Tag>::Storage_() {
    typedef typename POD_STORAGE(T) pod_type;
    STATIC_ASSERT(std::is_pod<pod_type>::value);
    static THREAD_LOCAL pod_type _gStorageTLS;
    return &_gStorageTLS;
}
//----------------------------------------------------------------------------
template <typename T, typename _Tag>
THREAD_LOCAL T *ThreadLocalSingleton<T, _Tag>::_gInstancePtr = nullptr;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
