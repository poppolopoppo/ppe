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
template <typename T>
struct TSingletonPOD_ {
    POD_STORAGE(T) Storage;
#if USE_PPE_ASSERT
    bool HasInstance{ false };
#endif
    T* Get() NOEXCEPT {
        return static_cast<T*>(static_cast<void*>(&Storage));
    }
};
template <typename T, typename _Tag, bool _ThreadLocal>
class TSingletonStorage_ {
protected:
    static void* make_singleton_storage() NOEXCEPT {
        ONE_TIME_DEFAULT_INITIALIZE(TSingletonPOD_<T>, GStorage);
        return (&GStorage);
    }
};
template <typename T, typename _Tag>
class TSingletonStorage_<T, _Tag, true> {
protected:
    static void* make_singleton_storage() NOEXCEPT {
        ONE_TIME_DEFAULT_INITIALIZE_THREAD_LOCAL(TSingletonPOD_<T>, GStorageTLS);
        return (&GStorageTLS);
    }
};
} //!details
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T, bool _ThreadLocal = false>
class TSingleton : details::TSingletonStorage_<T, _Tag, _ThreadLocal> {
    using pod_type = details::TSingletonPOD_<T>;
    using storage_type = details::TSingletonStorage_<T, _Tag, _ThreadLocal>;

    template <typename U>
    using has_class_singleton_storage_ = decltype(U::class_singleton_storage());

    static pod_type& SRef_() NOEXCEPT {
        // client can provide their own storage (way to handle shared libraries)
        IF_CONSTEXPR(Meta::has_defined_v<has_class_singleton_storage_, _Tag>)
            return (*static_cast<pod_type*>(_Tag::class_singleton_storage()));
        else
            return (*static_cast<pod_type*>(storage_type::make_singleton_storage()));
    }

protected:
    TSingleton() = default;

    using storage_type::make_singleton_storage;

public:
    TSingleton(const TSingleton& ) = delete;
    TSingleton& operator =(const TSingleton& ) = delete;

    TSingleton(TSingleton&& ) = delete;
    TSingleton& operator =(TSingleton&& ) = delete;

#if USE_PPE_ASSERT
    static bool HasInstance() NOEXCEPT {
        return SRef_().HasInstance;
    }
#endif

    static T& Get() NOEXCEPT {
        auto& storage = SRef_();
        Assert_NoAssume(storage.HasInstance);
        return (*storage.Get());
    }

    template <typename... _Args>
    static void Create(_Args&&... args) {
        auto& storage = SRef_();
        Assert_NoAssume(not storage.HasInstance);
        ONLY_IF_ASSERT(storage.HasInstance = true);
        new (static_cast<void*>(storage.Get())) T{ std::forward<_Args>(args)... };
    }

    static void Destroy() {
        auto& storage = SRef_();
        Assert_NoAssume(storage.HasInstance);
        storage.Get()->~T();
        ONLY_IF_ASSERT(storage.HasInstance = false);
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
    using parent_type = TSingleton<TUniquePtr<T>, _Tag, _ThreadLocal>;
protected:
    TIndirectSingleton() = default;

public:
#if USE_PPE_ASSERT
    using parent_type::HasInstance;
#endif

    static T& Get() NOEXCEPT {
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
