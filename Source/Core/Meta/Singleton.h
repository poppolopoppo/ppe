#pragma once

#include "Core/Core.h"

#include "Core/Memory/AlignedStorage.h"

namespace Core {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename _Tag, bool _ThreadLocal>
class SingletonStorage_ {
public:
    static bool gHasInstance;
    static T& Ref() {
        static typename POD_STORAGE(T) gPod;
        return reinterpret_cast<T&>(gPod);
    }
};
template <typename T, typename _Tag, bool _ThreadLocal>
bool SingletonStorage_<T, _Tag, _ThreadLocal>::gHasInstance = false;
template <typename T, typename _Tag>
class SingletonStorage_<T, _Tag, true> {
public:
    static THREAD_LOCAL bool gHasInstance;
    static T& Ref() {
        static THREAD_LOCAL typename POD_STORAGE(T) gPod;
        return reinterpret_cast<T&>(gPod);
    }
};
template <typename T, typename _Tag>
THREAD_LOCAL bool SingletonStorage_<T, _Tag, true>::gHasInstance = false;
} //!details
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T, bool _ThreadLocal = false>
class Singleton : details::SingletonStorage_<T, _Tag, _ThreadLocal> {
    typedef details::SingletonStorage_<T, _Tag, _ThreadLocal> storage_type;
protected:
    Singleton() = default;

public:
    Singleton(const Singleton& ) = delete;
    Singleton& operator =(const Singleton& ) = delete;

    Singleton(Singleton&& ) = delete;
    Singleton& operator =(Singleton&& ) = delete;

    static bool HasInstance() {
        return storage_type::gHasInstance;
    }

    static T& Instance() {
        Assert(HasInstance());
        return storage_type::Ref();
    }

    template <typename... _Args>
    static void Create(_Args&&... args) {
        AssertRelease(not HasInstance());
        new ((void*)&storage_type::Ref()) T(std::forward<_Args>(args)...);
        storage_type::gHasInstance = true;
    }

    static void Destroy() {
        AssertRelease(HasInstance());
        storage_type::Ref().~T();
        storage_type::gHasInstance = false;
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T>
using ThreadLocalSingleton = Singleton<T, _Tag, true>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
