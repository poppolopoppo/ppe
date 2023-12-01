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
template <typename T, typename _Tag = T, bool _ThreadLocal = false, bool _Shared = true>
class TSingleton : details::TSingletonStorage_<T, _Tag, _ThreadLocal> {
    using pod_type = details::TSingletonPOD_<T>;
    using storage_type = details::TSingletonStorage_<T, _Tag, _ThreadLocal>;

    static pod_type& SRef_() NOEXCEPT {
        // client can provide their own storage (way to handle shared libraries)
        IF_CONSTEXPR(_Shared)
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
    NODISCARD static bool HasInstance() NOEXCEPT {
        return SRef_().HasInstance;
    }
#endif

    NODISCARD static T& Get() NOEXCEPT {
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
template <typename T, typename _Tag = T, bool _Shared = false>
using TThreadLocalSingleton = TSingleton<T, _Tag, true, _Shared>;
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T, bool _ThreadLocal = false>
using TStaticSingleton = TSingleton<T, _Tag, _ThreadLocal, false>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T, bool _ThreadLocal = false, bool _Shared = true>
class TIndirectSingleton : TSingleton<TUniquePtr<T>, _Tag, _ThreadLocal, _Shared> {
    using parent_type = TSingleton<TUniquePtr<T>, _Tag, _ThreadLocal, _Shared>;
protected:
    TIndirectSingleton() = default;

public:
#if USE_PPE_ASSERT
    using parent_type::HasInstance;
#endif

    NODISCARD static T& Get() NOEXCEPT {
        return (*parent_type::Get());
    }

    template <typename... _Args>
    static void Create(_Args&&... args) {
        using FPointer = typename TUniquePtr<T>::FPointer;
        parent_type::Create(FPointer{ TUniquePtr<T>::New(std::forward<_Args>(args)...) });
    }

    static void Destroy() {
        parent_type::Destroy();
    }
};
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T, bool _Shared = false>
using TThreadLocalIndirectSingleton = TIndirectSingleton<T, _Tag, true, _Shared>;
//----------------------------------------------------------------------------
template <typename T, typename _Tag = T, bool _ThreadLocal = false>
using TStaticIndirectSingleton = TIndirectSingleton<T, _Tag, _ThreadLocal, false>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
