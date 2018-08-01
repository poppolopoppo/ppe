#pragma once

#include "Core.h"

#include "Meta/AlignedStorage.h"
#include "Memory/UniquePtr.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T, typename _Tag, bool _ThreadLocal>
class TSingletonStorage_ {
public:
#ifdef WITH_PPE_ASSERT
    static bool GHasInstance;
#endif

    static T& Ref() {
        static POD_STORAGE(T) GPod;
        return reinterpret_cast<T&>(GPod);
    }
};
#ifdef WITH_PPE_ASSERT
template <typename T, typename _Tag, bool _ThreadLocal>
bool TSingletonStorage_<T, _Tag, _ThreadLocal>::GHasInstance = false;
#endif
template <typename T, typename _Tag>
class TSingletonStorage_<T, _Tag, true> {
public:
#ifdef WITH_PPE_ASSERT
    static THREAD_LOCAL bool GHasInstance;
#endif

    static T& Ref() {
        static THREAD_LOCAL POD_STORAGE(T) GPod;
        return reinterpret_cast<T&>(GPod);
    }
};
#ifdef WITH_PPE_ASSERT
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

#ifdef WITH_PPE_ASSERT
    static bool HasInstance() {
        return storage_type::GHasInstance;
    }
#endif

    static T& Get() {
        Assert_NoAssume(HasInstance());
        return storage_type::Ref();
    }

    template <typename... _Args>
    static void Create(_Args&&... args) {
        Assert_NoAssume(not HasInstance());
        ONLY_IF_ASSERT(storage_type::GHasInstance = true);
        new ((void*)std::addressof(storage_type::Ref())) T{ std::forward<_Args>(args)... };
    }

    static void Destroy() {
        Assert_NoAssume(HasInstance());
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
template <typename T, typename _Tag = T, bool _ThreadLocal = false>
class TIndirectSingleton : TSingleton<TUniquePtr<T>, _Tag, _ThreadLocal> {
    typedef TSingleton<TUniquePtr<T>, _Tag, _ThreadLocal> parent_type;
protected:
    TIndirectSingleton() = default;

public:
#ifdef WITH_PPE_ASSERT
    static bool HasInstance() {
        return parent_type::GHasInstance;
    }
#endif

    static T& Get() {
        return (*parent_type::Get());
    }

    template <typename... _Args>
    static void Create(_Args&&... args) {
        parent_type::Create(new T{ std::forward<_Args>(args)... });
    }

    static void Destroy() {
        parent_type::Destroy();
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T>
using TThreadLocalIndirectSingleton = TIndirectSingleton<T, _Tag, true>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
