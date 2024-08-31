#pragma once

#include "HAL/PlatformMemory.h"
#include "Meta/AlignedStorage.h"
#include "Meta/Clonable.h"
#include "Meta/Optional.h"
#include "Meta/TypeTraits.h"

#include <memory>

#define _FWD_INSITUPTR_IMPL(T, _PREFIX)                                     \
    class CONCAT(_PREFIX, T);                                               \
    typedef ::PPE::TInSituPtr<const CONCAT(_PREFIX, T)> CONCAT(U,  T);

#define FWD_INSITUPTR(T_WITHOUT_F)              _FWD_INSITUPTR_IMPL(T_WITHOUT_F, F)
#define FWD_INTEFACE_INSITUPTR(T_WITHOUT_I)     _FWD_INSITUPTR_IMPL(T_WITHOUT_I, I)

#define FWD_PUREINTEFACE_PTR(T_WITHOUT_I)                                   \
    class CONCAT(I, T_WITHOUT_I);                                           \
    typedef ::PPE::TPureInterfacePtr<const CONCAT(I, T_WITHOUT_I)> CONCAT(U, T_WITHOUT_I);

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _Base, typename T, size_t _InSituSize>
struct TInSituPtrBase {
    STATIC_ASSERT(_InSituSize >= sizeof(T));
    STATIC_ASSERT(std::has_virtual_destructor_v<T>);

    alignas(T) std::byte Raw[_InSituSize];

    template <typename _Impl, typename... _Args>
    static constexpr bool is_constructible_v{
        std::is_base_of_v<T, _Impl> and
        sizeof(_Impl) <= sizeof(Raw) and
        std::is_constructible_v<_Impl, _Args...> 
    };

    CONSTEXPR TInSituPtrBase(Meta::FNoInit) NOEXCEPT 
    {}

    template <typename _Impl, std::enable_if_t<std::is_base_of_v<T, _Impl> && sizeof(_Impl) <= _InSituSize>* = nullptr>
    CONSTEXPR TInSituPtrBase(_Impl&& impl) NOEXCEPT_IF(std::is_nothrow_move_constructible_v<_Impl>) {
        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        new (static_cast<void*>(&Raw)) _Impl(std::move(impl));
    }

    ~TInSituPtrBase() {
        Meta::Destroy(const_cast<std::remove_const_t<T>*>(get()));

        ONLY_IF_ASSERT(FPlatformMemory::Memdeadbeef(Raw, _InSituSize));
    }

    template <typename _Impl, typename... _Args>
    NODISCARD static CONSTEXPR std::enable_if_t<is_constructible_v<_Impl, _Args&&...>, _Base> 
        Make(_Args&&... args) NOEXCEPT_IF(std::is_nothrow_constructible_v<_Impl, _Args&&...>) {
        return _Impl{ std::forward<_Args>(args)... };
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

    NODISCARD operator const T* () const NOEXCEPT {
        return get();
    }
    NODISCARD operator const T& () const NOEXCEPT {
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

};
} //!details
//----------------------------------------------------------------------------
template <typename T, size_t _InSituSize = sizeof(T), bool _bAlwaysTrivial = false>
struct TInSituPtr : details::TInSituPtrBase<TInSituPtr<T, _InSituSize, _bAlwaysTrivial>, T, _InSituSize> {
    STATIC_ASSERT(std::is_base_of_v<Meta::IClonable, T>);

    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::TInSituPtrBase;
    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::get;
    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::Block;
    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::Raw;
    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::operator *;
    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::operator ->;

    CONSTEXPR TInSituPtr(const TInSituPtr& impl) 
        : details::TInSituPtrBase<TInSituPtr, T, _InSituSize>(Meta::NoInit) {
        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        static_cast<const Meta::IClonable*>(impl.get())->ConstructCopy(Block());
    }

    CONSTEXPR TInSituPtr& operator =(const TInSituPtr& impl) {
        Meta::Destroy(const_cast<std::remove_const_t<T>*>(get()));

        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        static_cast<const Meta::IClonable*>(impl.get())->ConstructCopy(Block());
        return (*this);
    }

    CONSTEXPR TInSituPtr(TInSituPtr&& rimpl) NOEXCEPT
        : details::TInSituPtrBase<TInSituPtr, T, _InSituSize>(Meta::NoInit) {
        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        static_cast<Meta::IClonable*>(rimpl.get())->ConstructMove(Block());
    }

    CONSTEXPR TInSituPtr& operator =(TInSituPtr&& rimpl) NOEXCEPT {
        Meta::Destroy(const_cast<std::remove_const_t<T>*>(get()));

        ONLY_IF_ASSERT(FPlatformMemory::Memuninitialized(Raw, _InSituSize));

        static_cast<Meta::IClonable*>(rimpl.get())->ConstructMove(Block());
        return (*this);
    }

    // Consider TInSituPtr<T> as POD if and only if T is considered as POD
    NODISCARD friend CONSTEXPR bool is_pod_type(TInSituPtr*) {
        return Meta::is_pod_v<T>;
    }
};
//----------------------------------------------------------------------------
template <typename T, size_t _InSituSize>
struct TInSituPtr<T, _InSituSize, true> : details::TInSituPtrBase<TInSituPtr<T, _InSituSize, true>, T, _InSituSize> {
    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::TInSituPtrBase;
    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::get;
    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::Raw; 
    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::operator *;
    using details::TInSituPtrBase<TInSituPtr, T, _InSituSize>::operator ->;

    CONSTEXPR TInSituPtr(const TInSituPtr& impl)
        : details::TInSituPtrBase<TInSituPtr, T, _InSituSize>(Meta::NoInit) {
        FPlatformMemory::Memcpy(Raw, impl.Raw, _InSituSize);
    }

    CONSTEXPR TInSituPtr& operator =(const TInSituPtr& impl) {
        FPlatformMemory::Memcpy(Raw, impl.Raw, _InSituSize);
        return (*this);
    }

    CONSTEXPR TInSituPtr(TInSituPtr&& rimpl) NOEXCEPT
        : details::TInSituPtrBase<TInSituPtr, T, _InSituSize>(Meta::NoInit) {
        FPlatformMemory::Memcpy(Raw, rimpl.Raw, _InSituSize);
    }

    CONSTEXPR TInSituPtr& operator =(TInSituPtr&& rimpl) NOEXCEPT {
        FPlatformMemory::Memcpy(Raw, rimpl.Raw, _InSituSize);
        return (*this);
    }

    // Always considered as pod, since it's always trivial
    NODISCARD friend CONSTEXPR bool is_pod_type(TInSituPtr*) {
        return true;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TPureInterfacePtr {
    using insitu_type = TInSituPtr<T, sizeof(T), not std::is_base_of_v<Meta::IClonable, T>>;

    Meta::TOptional<insitu_type> InSitu;

    CONSTEXPR TPureInterfacePtr() NOEXCEPT = default;

    CONSTEXPR TPureInterfacePtr(Meta::FDefaultValue) NOEXCEPT
    {}

    template <typename _Impl, std::enable_if_t<std::is_constructible_v<insitu_type, _Impl&&>>* = nullptr>
    CONSTEXPR TPureInterfacePtr(_Impl&& impl) NOEXCEPT_IF(std::is_nothrow_constructible_v<insitu_type, _Impl&&>)
        : InSitu(std::move(impl)) 
    {}

    TPureInterfacePtr(const TPureInterfacePtr& ) = default;
    TPureInterfacePtr& operator =(const TPureInterfacePtr& ) = default;

    TPureInterfacePtr(TPureInterfacePtr&& ) = default;
    TPureInterfacePtr& operator =(TPureInterfacePtr&& ) = default;

    template <typename _Impl, typename... _Args>
    NODISCARD static CONSTEXPR std::enable_if_t<insitu_type::template is_constructible_v<_Impl, _Args&&...>, TPureInterfacePtr> 
        Make(_Args&&... args) NOEXCEPT_IF(std::is_nothrow_constructible_v<_Impl, _Args&&...>) {
        return _Impl{ std::forward<_Args>(args)... };
    }

    NODISCARD CONSTEXPR bool Valid() const {
        return InSitu.has_value();
    }

    NODISCARD T* get() NOEXCEPT {
        return (InSitu.has_value() ? InSitu->get() : nullptr);
    }
    NODISCARD const T* get() const NOEXCEPT {
        return const_cast<TPureInterfacePtr*>(this)->get();
    }

    NODISCARD operator T* () NOEXCEPT {
        return get();
    }
    NODISCARD operator T& () NOEXCEPT {
        return InSitu.value();
    }

    NODISCARD operator const T* () const NOEXCEPT {
        return get();
    }
    NODISCARD operator const T& () const NOEXCEPT {
        return InSitu.value();
    }

    NODISCARD T* operator ->() NOEXCEPT {
        return InSitu->get();
    }
    NODISCARD T& operator *() NOEXCEPT {
        return (*InSitu.value());
    }

    NODISCARD const T* operator ->() const NOEXCEPT {
        return InSitu->get();
    }
    NODISCARD const T& operator *() const NOEXCEPT {
        return (*InSitu.value());
    }

    // Forward to insitu ptr (like everything else)
    NODISCARD friend CONSTEXPR bool is_pod_type(TPureInterfacePtr*) {
        return is_pod_type(static_cast<insitu_type*>(nullptr));
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
