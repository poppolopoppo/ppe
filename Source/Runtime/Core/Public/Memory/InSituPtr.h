#pragma once

#include "HAL/PlatformMemory.h"
#include "Meta/AlignedStorage.h"
#include "Meta/Clonable.h"
#include "Meta/TypeTraits.h"

#include <memory>

#define _FWD_INSITUPTR_IMPL(T, _PREFIX)                                     \
    class CONCAT(_PREFIX, T);                                               \
    typedef ::PPE::TInSituPtr<const CONCAT(_PREFIX, T)> CONCAT(U,  T);

#define FWD_INSITUPTR(T_WITHOUT_F)              _FWD_INSITUPTR_IMPL(T_WITHOUT_F, F)
#define FWD_INTEFARCE_INSITUPTR(T_WITHOUT_I)    _FWD_INSITUPTR_IMPL(T_WITHOUT_I, I)

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _InSituSize = sizeof(T), bool _bAlwaysTrivial = false>
struct TInSituPtr {
    STATIC_ASSERT(std::is_base_of_v<Meta::IClonable, T>);
    STATIC_ASSERT(std::has_virtual_destructor_v<T>);
    STATIC_ASSERT(_InSituSize >= sizeof(T));

    alignas(T) std::byte Raw[_InSituSize];

    ~TInSituPtr() {
        Meta::Destroy(get());

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(Raw, _InSituSize));
    }

    template <typename _Impl, std::enable_if_t<std::is_base_of_v<T, _Impl> && sizeof(_Impl) <= _InSituSize>* = nullptr>
    CONSTEXPR TInSituPtr(_Impl&& impl) NOEXCEPT_IF(std::is_nothrow_move_constructible_v<_Impl>) {
        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        new (Block().Data) _Impl(std::move(impl));
    }

    CONSTEXPR TInSituPtr(const TInSituPtr& impl) {
        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        static_cast<const Meta::IClonable*>(impl.get())->ConstructCopy(Block());
    }

    CONSTEXPR TInSituPtr& operator =(const TInSituPtr& impl) {
        Meta::Destroy(get());

        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        static_cast<const Meta::IClonable*>(impl.get())->ConstructCopy(Block());
        return (*this);
    }

    CONSTEXPR TInSituPtr(TInSituPtr&& rimpl) NOEXCEPT {
        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        static_cast<Meta::IClonable*>(rimpl.get())->ConstructMove(Block());
    }

    CONSTEXPR TInSituPtr& operator =(TInSituPtr&& rimpl) NOEXCEPT {
        Meta::Destroy(get());

        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        static_cast<Meta::IClonable*>(rimpl.get())->ConstructMove(Block());
        return (*this);
    }

    NODISCARD CONSTEXPR FAllocatorBlock Block() NOEXCEPT {
        return {&Raw[0], _InSituSize};
    }

    NODISCARD T* get() NOEXCEPT {
        return reinterpret_cast<T*>(&Raw[0]);
    }
    NODISCARD const T* get() const NOEXCEPT {
        return reinterpret_cast<const T*>(&Raw[0]);
    }

    NODISCARD operator T* () NOEXCEPT {
        return get();
    }
    NODISCARD operator T& () NOEXCEPT {
        return (*get());
    }

    NODISCARD operator const T* () NOEXCEPT {
        return get();
    }
    NODISCARD operator const T& () NOEXCEPT {
        return (*get());
    }

    NODISCARD T* operator ->() NOEXCEPT {
        return get();
    }
    NODISCARD T& operator *() NOEXCEPT {
        return (*get());
    }

    NODISCARD const T* operator ->() const NOEXCEPT {
        return get();
    }
    NODISCARD const T& operator *() const NOEXCEPT {
        return (*get());
    }

    // Consider TInSituPtr<T> as POD if and only if T is considered as POD
    NODISCARD friend CONSTEXPR bool is_pod_type(TInSituPtr*) {
        return Meta::is_pod_v<T>;
    }
};
//----------------------------------------------------------------------------
template <typename T, size_t _InSituSize>
struct TInSituPtr<T, _InSituSize, true> {
    STATIC_ASSERT(_InSituSize >= sizeof(T));

    alignas(T) std::byte Raw[_InSituSize];

#if USE_PPE_ASSERT
    ~TInSituPtr() {
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(Raw, _InSituSize));
    }
#endif

    template <typename _Impl, std::enable_if_t<std::is_base_of_v<T, _Impl> && sizeof(_Impl) <= _InSituSize>* = nullptr>
    CONSTEXPR TInSituPtr(_Impl&& impl) NOEXCEPT_IF(std::is_nothrow_move_constructible_v<_Impl>) {
        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        new (Block().Data) _Impl(std::move(impl));
    }

    CONSTEXPR TInSituPtr(const TInSituPtr& impl) {
        FPlatformMemory::Memcpy(Raw, impl.Raw, _InSituSize);
    }

    CONSTEXPR TInSituPtr& operator =(const TInSituPtr& impl) {
        FPlatformMemory::Memcpy(Raw, impl.Raw, _InSituSize);
        return (*this);
    }

    CONSTEXPR TInSituPtr(TInSituPtr&& rimpl) NOEXCEPT {
        FPlatformMemory::Memcpy(Raw, rimpl.Raw, _InSituSize);
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(rimpl.Raw, _InSituSize));
    }

    CONSTEXPR TInSituPtr& operator =(TInSituPtr&& rimpl) NOEXCEPT {
        FPlatformMemory::Memcpy(Raw, rimpl.Raw, _InSituSize);
        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(rimpl.Raw, _InSituSize));
        return (*this);
    }

    NODISCARD CONSTEXPR FAllocatorBlock Block() NOEXCEPT {
        return {&Raw[0], _InSituSize};
    }

    NODISCARD T* get() NOEXCEPT {
        return reinterpret_cast<T*>(&Raw[0]);
    }
    NODISCARD const T* get() const NOEXCEPT {
        return reinterpret_cast<const T*>(&Raw[0]);
    }

    NODISCARD operator T* () NOEXCEPT {
        return get();
    }
    NODISCARD operator T& () NOEXCEPT {
        return (*get());
    }

    NODISCARD operator const T* () NOEXCEPT {
        return get();
    }
    NODISCARD operator const T& () NOEXCEPT {
        return (*get());
    }

    NODISCARD T* operator ->() NOEXCEPT {
        return get();
    }
    NODISCARD T& operator *() NOEXCEPT {
        return (*get());
    }

    NODISCARD const T* operator ->() const NOEXCEPT {
        return get();
    }
    NODISCARD const T& operator *() const NOEXCEPT {
        return (*get());
    }

    // Consider TInSituPtr<T> as POD if and only if T is considered as POD
    NODISCARD friend CONSTEXPR bool is_pod_type(TInSituPtr*) {
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
