#pragma once

#include "Meta/Cast.h"

#include <type_traits>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FPointerWFlags {
    uintptr_t _packed;

    NODISCARD CONSTEXPR void* Get() const {
        return bit_cast<void*>(_packed & ~0x3);
    }
    CONSTEXPR void Set(const void* ptr) {
        _packed = bit_cast<uintptr_t>(ptr) | (_packed & 0x3);
        Assert_NoAssume(Get() == ptr);
    }

    NODISCARD CONSTEXPR uintptr_t Flag01() const { return (_packed & 0x3); }
    CONSTEXPR void SetFlag01(uintptr_t value) {
        _packed = (_packed & ~0x3) | (value & 0x3);
        Assert_NoAssume(Flag01() == value);
    }

    NODISCARD CONSTEXPR bool Flag0() const {
        return (0 != (_packed & 1));
    }
    CONSTEXPR void SetFlag0(bool value) {
        _packed = (value ? _packed | 1 : _packed & ~1);
    }

    NODISCARD CONSTEXPR bool Flag1() const {
        return (0 != (_packed & 2));
    }
    CONSTEXPR void SetFlag1(bool value) {
        _packed = (value ? _packed | 2 : _packed & ~2);
    }

    CONSTEXPR void Reset() {
        _packed = 0;
    }
    CONSTEXPR void Reset(const void* p, bool flag0 = false, bool flag1 = false) {
        _packed = (bit_cast<uintptr_t>(p) | (flag0 ? 0x1 : 0) | (flag1 ? 0x2 : 0));
        Assert_NoAssume(Get() == p && Flag0() == flag0 && Flag1() == flag1);
    }
    CONSTEXPR void Reset(const void* p, uintptr_t flag01) {
        _packed = (bit_cast<uintptr_t>(p) | (flag01 & 0x3));
        Assert_NoAssume(Get() == p && Flag01() == flag01);
    }

    CONSTEXPR friend void swap(FPointerWFlags& lhs, FPointerWFlags& rhs) NOEXCEPT {
        std::swap(lhs._packed, rhs._packed);
    }

    NODISCARD CONSTEXPR friend bool operator ==(FPointerWFlags lhs, FPointerWFlags rhs) {
        return (lhs._packed == rhs._packed);
    }

#if PPE_HAS_CXX20
    NODISCARD CONSTEXPR friend std::strong_ordering operator <=>(FPointerWFlags lhs, FPointerWFlags rhs) {
        return (lhs._packed <=> rhs._packed);
    }
#else
    NODISCARD CONSTEXPR friend bool operator !=(FPointerWFlags lhs, FPointerWFlags rhs) {
        return (lhs._packed != rhs._packed);
    }
#endif
};
PPE_ASSERT_TYPE_IS_POD(FPointerWFlags);
//----------------------------------------------------------------------------
template <typename T>
struct TPointerWFlags {
    FPointerWFlags _base;

    NODISCARD CONSTEXPR uintptr_t Packed() const { return _base._packed; }

    NODISCARD CONSTEXPR T* Get() const { return static_cast<T*>(_base.Get()); }
    CONSTEXPR void Set(const T* ptr) { _base.Set(ptr); }

    T* operator ->() const { return Get(); }
    T& operator *() const { return *Get(); }

    NODISCARD CONSTEXPR T& Reference() const {
        T* ptr = Get();
        Assert(ptr);
        return *ptr;
    }

    NODISCARD CONSTEXPR uintptr_t Flag01() const { return _base.Flag01(); }
    CONSTEXPR void SetFlag01(uintptr_t value) { _base.SetFlag01(value); }

    NODISCARD CONSTEXPR bool Flag0() const { return _base.Flag0(); }
    CONSTEXPR void SetFlag0(bool value) { _base.SetFlag0(value); }

    NODISCARD CONSTEXPR bool Flag1() const { return _base.Flag1(); }
    CONSTEXPR void SetFlag1(bool value) { _base.SetFlag1(value); }

    CONSTEXPR void Reset() { _base.Reset(); }
    CONSTEXPR void Reset(T* p, bool flag0 = false, bool flag1 = false) { _base.Reset(p, flag0, flag1); }
    CONSTEXPR void Reset(T* p, uintptr_t flag01) { _base.Reset(p, flag01); }

    CONSTEXPR friend void swap(TPointerWFlags& lhs, TPointerWFlags& rhs) NOEXCEPT {
        swap(lhs._base, rhs._base);
    }

    NODISCARD CONSTEXPR friend bool operator ==(TPointerWFlags lhs, TPointerWFlags rhs) {
        return (lhs._base == rhs._base);
    }

#if PPE_HAS_CXX20
    NODISCARD CONSTEXPR friend std::strong_ordering operator <=>(TPointerWFlags lhs, TPointerWFlags rhs) {
        return (lhs._base <=> rhs._base);
    }
#else
    NODISCARD CONSTEXPR friend bool operator !=(TPointerWFlags lhs, TPointerWFlags rhs) {
        return (lhs._base != rhs._base);
    }
#endif
};
//----------------------------------------------------------------------------
template <>
struct TPointerWFlags<void> {
    FPointerWFlags _base;

    NODISCARD CONSTEXPR uintptr_t Packed() const { return _base._packed; }

    NODISCARD CONSTEXPR void* Get() const { return _base.Get(); }
    CONSTEXPR void Set(void* ptr) { _base.Set(ptr); }

    NODISCARD CONSTEXPR uintptr_t Flag01() const { return _base.Flag01(); }
    CONSTEXPR void SetFlag01(uintptr_t value) { _base.SetFlag01(value); }

    NODISCARD CONSTEXPR bool Flag0() const { return _base.Flag0(); }
    CONSTEXPR void SetFlag0(bool value) { _base.SetFlag0(value); }

    NODISCARD CONSTEXPR bool Flag1() const { return _base.Flag1(); }
    CONSTEXPR void SetFlag1(bool value) { _base.SetFlag1(value); }

    CONSTEXPR void Reset() { _base.Reset(); }
    CONSTEXPR void Reset(void* p, bool flag0 = false, bool flag1 = false) { _base.Reset(p, flag0, flag1); }
    CONSTEXPR void Reset(void* p, uintptr_t flag01) { _base.Reset(p, flag01); }

    CONSTEXPR friend void swap(TPointerWFlags& lhs, TPointerWFlags& rhs) NOEXCEPT {
        swap(lhs._base, rhs._base);
    }

    NODISCARD CONSTEXPR friend bool operator ==(TPointerWFlags lhs, TPointerWFlags rhs) {
        return (lhs._base == rhs._base);
    }

#if PPE_HAS_CXX20
    NODISCARD CONSTEXPR friend std::strong_ordering operator <=>(TPointerWFlags lhs, TPointerWFlags rhs) {
        return (lhs._base <=> rhs._base);
    }
#else
    NODISCARD CONSTEXPR friend bool operator !=(TPointerWFlags lhs, TPointerWFlags rhs) {
        return (lhs._base != rhs._base);
    }
#endif
};
//----------------------------------------------------------------------------
template <>
struct TPointerWFlags<const void> {
    FPointerWFlags _base;

    NODISCARD CONSTEXPR uintptr_t Packed() const { return _base._packed; }

    NODISCARD CONSTEXPR const void* Get() const { return _base.Get(); }
    CONSTEXPR void Set(const void* ptr) { _base.Set(ptr); }

    NODISCARD CONSTEXPR uintptr_t Flag01() const { return _base.Flag01(); }
    CONSTEXPR void SetFlag01(uintptr_t value) { _base.SetFlag01(value); }

    NODISCARD CONSTEXPR bool Flag0() const { return _base.Flag0(); }
    CONSTEXPR void SetFlag0(bool value) { _base.SetFlag0(value); }

    NODISCARD CONSTEXPR bool Flag1() const { return _base.Flag1(); }
    CONSTEXPR void SetFlag1(bool value) { _base.SetFlag1(value); }

    CONSTEXPR void Reset() { _base.Reset(); }
    CONSTEXPR void Reset(const void* p, bool flag0 = false, bool flag1 = false) { _base.Reset(p, flag0, flag1); }
    CONSTEXPR void Reset(const void* p, uintptr_t flag01) { _base.Reset(p, flag01); }

    CONSTEXPR friend void swap(TPointerWFlags& lhs, TPointerWFlags& rhs) NOEXCEPT {
        swap(lhs._base, rhs._base);
    }

    NODISCARD CONSTEXPR friend bool operator ==(TPointerWFlags lhs, TPointerWFlags rhs) {
        return (lhs._base == rhs._base);
    }

#if PPE_HAS_CXX20
    NODISCARD CONSTEXPR friend std::strong_ordering operator <=>(TPointerWFlags lhs, TPointerWFlags rhs) {
        return (lhs._base <=> rhs._base);
    }
#else
    NODISCARD CONSTEXPR friend bool operator !=(TPointerWFlags lhs, TPointerWFlags rhs) {
        return (lhs._base != rhs._base);
    }
#endif
};
//----------------------------------------------------------------------------
template <typename T>
NODISCARD CONSTEXPR inline bool operator ==(TPointerWFlags<T> lhs, T* rhs) {
    return (lhs.Get() == rhs);
}
template <typename T>
NODISCARD CONSTEXPR inline bool operator ==(T* lhs, TPointerWFlags<T> rhs) {
    return (lhs == rhs.Get());
}
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FPointerWFlags) == sizeof(void*));
STATIC_ASSERT(Meta::is_pod_v< FPointerWFlags >);
STATIC_ASSERT(sizeof(TPointerWFlags<int>) == sizeof(void*));
STATIC_ASSERT(Meta::is_pod_v< TPointerWFlags<int> >);
STATIC_ASSERT(Meta::is_pod_v< TPointerWFlags<void> >);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FHeapPtrWCounter { // assumed aligned on ALLOCATION_BOUNDARY (eg. 16)
    uintptr_t Data;

    CONSTEXPR void Reset() NOEXCEPT {
        Data = 0;
    }
    void Reset(const void* ptr, u32 n) NOEXCEPT {
        AssertRelease_NoAssume(Meta::IsAlignedPow2(16, ptr));
        Data = (bit_cast<uintptr_t>(ptr) | n);
        Assert_NoAssume(Ptr<const void>() == ptr);
        Assert_NoAssume(Counter() == n);
    }

    CONSTEXPR u8 Counter() const NOEXCEPT {
        return u8(Data & 0xF);
    }
    void SetCounter(size_t n) NOEXCEPT {
        Data = ((Data & ~static_cast<uintptr_t>(0xF)) | static_cast<uintptr_t>(n));
        Assert_NoAssume(Counter() == n);
    }

    template <typename T>
    T* Ptr() const NOEXCEPT { return reinterpret_cast<T*>(Data & ~static_cast<uintptr_t>(0xF)); }
    void SetPtr(const void* ptr) NOEXCEPT {
        Assert_NoAssume(Meta::IsAlignedPow2(16, ptr));
        Data = ((Data & static_cast<uintptr_t>(0xF)) | bit_cast<uintptr_t>(ptr));
        Assert_NoAssume(Ptr<const void>() == ptr);
    }

    friend void swap(FHeapPtrWCounter& lhs, FHeapPtrWCounter& rhs) NOEXCEPT {
        std::swap(lhs.Data, rhs.Data);
    }
};
PPE_ASSERT_TYPE_IS_POD(FHeapPtrWCounter);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
