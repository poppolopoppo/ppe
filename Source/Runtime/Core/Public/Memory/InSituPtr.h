#pragma once

#include "HAL/PlatformMemory.h"
#include "Meta/AlignedStorage.h"
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
template <typename T>
struct TInSituPtr {
#ifdef ARCH_X86
    STATIC_CONST_INTEGRAL(intptr_t, NullMagick, 0xDEADF001ul);
#else
    STATIC_CONST_INTEGRAL(intptr_t, NullMagick, 0xDEADF001DEADF001ull);
#endif
    union {
        intptr_t VTable;
        POD_STORAGE(T) InSitu;
    };

    CONSTEXPR explicit TInSituPtr(Meta::FNoInit) NOEXCEPT {}

    CONSTEXPR TInSituPtr() NOEXCEPT : VTable(NullMagick) {}

    NODISCARD CONSTEXPR bool Valid() const { return (VTable != NullMagick); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    NODISCARD CONSTEXPR T* get() {
        Assert(Valid());
        return reinterpret_cast<T*>(std::addressof(InSitu));
    }
    NODISCARD CONSTEXPR const T* get() const { return const_cast<TInSituPtr*>(this)->get(); }

    NODISCARD CONSTEXPR T& operator *() { return *get(); }
    NODISCARD CONSTEXPR const T& operator *() const { return *get(); }

    CONSTEXPR T* operator ->() { return get(); }
    CONSTEXPR const T* operator ->() const { return get(); }

    template <typename U, typename... _Args>
    CONSTEXPR U* Create(_Args&&... args) {
        Assert(not Valid());
        return Create_AssumeNotValid(std::forward<_Args>(args)...);
    }

    template <typename U, typename... _Args>
    CONSTEXPR U* Create_AssumeNotValid(_Args&&... args) {
        STATIC_ASSERT(std::is_base_of<T, U>::value);
        STATIC_ASSERT(sizeof(U) == sizeof(T));
        U* const result = INPLACE_NEW(std::addressof(InSitu), U) { std::forward<_Args>(args)... };
        Assert(Valid());
        return result;
    }

    CONSTEXPR void CreateRawCopy(const T& src) {
        STATIC_ASSERT(std::is_trivially_destructible_v<T>);
        Assert(not Valid());
        FPlatformMemory::Memcpy(&InSitu, static_cast<void*>(&src), sizeof(T));
        Assert(Valid());
    }

    CONSTEXPR void CreateRawCopy_AssumeNotInitialized(const T& src) {
        FPlatformMemory::Memcpy(&InSitu, static_cast<void*>(&src), sizeof(T));
        Assert(Valid());
    }

    CONSTEXPR void Destroy() {
        Assert(Valid());
        get()->~T();
        VTable = NullMagick;
    }

    template <typename U, typename... _Args>
    NODISCARD CONSTEXPR static TInSituPtr Make(_Args&&... args) {
        TInSituPtr p(Meta::NoInit);
        p.template Create_AssumeNotValid<U>(std::forward<_Args>(args)...);
        return p;
    }

    NODISCARD inline friend bool operator ==(const TInSituPtr& lhs, const TInSituPtr& rhs) {
#if 0
        STATIC_ASSERT(sizeof(lhs) == sizeof(intptr_t));
        STATIC_ASSERT(sizeof(rhs) == sizeof(intptr_t));
        return (lhs.VTable == rhs.VTable);
#else
        return (FPlatformMemory::Memcmp(&lhs, &rhs, sizeof(TInSituPtr)) == 0);

#endif
    }
    NODISCARD inline friend bool operator !=(const TInSituPtr& lhs, const TInSituPtr& rhs) {
        return (not operator ==(lhs, rhs));
    }

    // Consider TInSituPtr<T> as POD if and only if T is considered as POD
    NODISCARD friend CONSTEXPR bool is_pod_type(TInSituPtr*) { return Meta::is_pod_v<T>; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
