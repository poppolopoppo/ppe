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
class TSingletonStorage_ {
public:
#ifdef WITH_CORE_ASSERT
    static bool GHasInstance;
#endif
    static T& Ref() {
        static typename POD_STORAGE(T) GPod;
        return reinterpret_cast<T&>(GPod);
    }
};
#ifdef WITH_CORE_ASSERT
template <typename T, typename _Tag, bool _ThreadLocal>
bool TSingletonStorage_<T, _Tag, _ThreadLocal>::GHasInstance = false;
#endif
template <typename T, typename _Tag>
class TSingletonStorage_<T, _Tag, true> {
public:
#ifdef WITH_CORE_ASSERT
    static THREAD_LOCAL bool GHasInstance;
#endif
    static T& Ref() {
        static THREAD_LOCAL typename POD_STORAGE(T) GPod;
        return reinterpret_cast<T&>(GPod);
    }
};
#ifdef WITH_CORE_ASSERT
template <typename T, typename _Tag>
THREAD_LOCAL bool TSingletonStorage_<T, _Tag, true>::GHasInstance = false;
#endif
} //!details
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T, bool _ThreadLocal = false>
class TSingleton : details::TSingletonStorage_<T, _Tag, _ThreadLocal> {
    typedef details::TSingletonStorage_<T, _Tag, _ThreadLocal> storage_type;
protected:
    TSingleton() = default;

public:
    TSingleton(const TSingleton& ) = delete;
    TSingleton& operator =(const TSingleton& ) = delete;

    TSingleton(TSingleton&& ) = delete;
    TSingleton& operator =(TSingleton&& ) = delete;

#ifdef WITH_CORE_ASSERT
    static bool HasInstance() {
        return storage_type::GHasInstance;
    }
#endif

    static T& Instance() {
        Assert(HasInstance());
        return storage_type::Ref();
    }

    template <typename... _Args>
    static void Create(_Args&&... args) {
        Assert(not HasInstance());
        new ((void*)&storage_type::Ref()) T(std::forward<_Args>(args)...);
        ONLY_IF_ASSERT(storage_type::GHasInstance = true);
    }

    static void Destroy() {
        Assert(HasInstance());
        storage_type::Ref().~T();
        ONLY_IF_ASSERT(storage_type::GHasInstance = false);
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T>
using TThreadLocalSingleton = TSingleton<T, _Tag, true>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace Core
