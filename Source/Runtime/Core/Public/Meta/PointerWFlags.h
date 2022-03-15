#pragma once

#include "Meta/Cast.h"

#include <type_traits>

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// use a macro instead of inheritance to keep TPointerWFlags<> as a pod
#define POINTERWFLAGS_BASE_DEF() \
    uintptr_t Flag01() const { return _packed._flags; } \
    void SetFlag01(uintptr_t value) { \
        _packed._flags = value; \
        Assert(value == _packed._flags); \
    } \
    \
    bool Flag0() const { return (0 != (_packed._flags & 1)); } \
    void SetFlag0(bool value) { _packed._flags = (value ? _packed._flags | 1 : _packed._flags & ~1); } \
    \
    bool Flag1() const { return (0 != (_packed._flags & 2)); } \
    void SetFlag1(bool value) { _packed._flags = (value ? _packed._flags | 2 : _packed._flags & ~2); } \
    \
    void *RawPointer() const { return (void*)(_packed._ptr << 2); } \
    void SetRawPointer(const void *ptr) { \
        _packed._ptr = ((uintptr_t)ptr >> 2); \
        Assert_NoAssume(RawPointer() == ptr); \
    } \
    \
    void Reset() { \
        _raw = nullptr; \
    } \
    \
    friend inline void swap(TPointerWFlags& lhs, TPointerWFlags& rhs) { \
        std::swap(lhs._raw, rhs._raw); \
    } \
    \
    friend inline bool operator ==(const TPointerWFlags& lhs, const TPointerWFlags& rhs) { \
        return (lhs._raw == rhs._raw); \
    } \
    \
    friend inline bool operator !=(const TPointerWFlags& lhs, const TPointerWFlags& rhs) { \
        return (lhs._raw != rhs._raw); \
    } \
    \
    union { \
        void* _raw; \
        struct { \
            uintptr_t _flags : 2; \
            uintptr_t _ptr : CODE3264(30, 62); \
        }   _packed; \
    }

//----------------------------------------------------------------------------
template <typename T>
struct TPointerWFlags {
    POINTERWFLAGS_BASE_DEF();

    FORCE_INLINE T *Get() const { return reinterpret_cast<T*>(RawPointer()); }
    FORCE_INLINE void Set(T *ptr) { SetRawPointer(ptr); }

    T& Reference() const {
        T* ptr = Get();
        Assert(ptr);
        return *ptr;
    }

    void Reset(T* p, bool flag0 = false, bool flag1 = false) {
        SetRawPointer(p);
        SetFlag0(flag0);
        SetFlag1(flag1);
    }

    void Reset(T* p, uintptr_t flag01) {
        SetRawPointer(p);
        SetFlag01(flag01);
    }

    T* operator ->() const { return Get(); }
    T& operator *() const { return *Get(); }
};
//----------------------------------------------------------------------------
template <>
struct TPointerWFlags<void> {
    POINTERWFLAGS_BASE_DEF();

    FORCE_INLINE void *Get() const { return RawPointer(); }
    FORCE_INLINE void Set(void *ptr) { SetRawPointer(ptr); }

    void Reset(void *p, bool flag0 = false, bool flag1 = false) {
        SetRawPointer(p);
        SetFlag0(flag0);
        SetFlag1(flag1);
    }

    void Reset(void *p, uintptr_t flag01) {
        SetRawPointer(p);
        SetFlag01(flag01);
    }
};
//----------------------------------------------------------------------------
template <>
struct TPointerWFlags<const void> {
    POINTERWFLAGS_BASE_DEF();

    FORCE_INLINE void* Get() const { return RawPointer(); }
    FORCE_INLINE void Set(const void* ptr) { SetRawPointer(ptr); }

    void Reset(const void* p, bool flag0 = false, bool flag1 = false) {
        SetRawPointer(p);
        SetFlag0(flag0);
        SetFlag1(flag1);
    }

    void Reset(const void* p, uintptr_t flag01) {
        SetRawPointer(p);
        SetFlag01(flag01);
    }
};
//----------------------------------------------------------------------------
#undef POINTERWFLAGS_BASE_DEF
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(TPointerWFlags<int>) == sizeof(void*));
STATIC_ASSERT(Meta::is_pod_v< TPointerWFlags<int> >);
STATIC_ASSERT(Meta::is_pod_v< TPointerWFlags<void> >);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FHeapPtrWCounter { // assumed aligned on 16
    uintptr_t Data;

    CONSTEXPR void Reset() NOEXCEPT {
        Data = 0;
    }
    void Reset(const void* ptr, u32 n) NOEXCEPT {
        Assert_NoAssume(Meta::IsAlignedPow2(16, ptr));
        Data = (uintptr_t(ptr) | n);
        Assert_NoAssume(Ptr<const void>() == ptr);
        Assert_NoAssume(Counter() == n);
    }

    CONSTEXPR u8 Counter() const NOEXCEPT {
        return u8(Data & 0xF);
    }
    void SetCounter(size_t n) NOEXCEPT {
        Data = ((Data & ~uintptr_t(0xF)) | uintptr_t(n));
        Assert_NoAssume(Counter() == n);
    }

    template <typename T>
    T* Ptr() const NOEXCEPT { return reinterpret_cast<T*>(Data & ~uintptr_t(0xF)); }
    void SetPtr(const void* ptr) NOEXCEPT {
        Assert_NoAssume(Meta::IsAlignedPow2(16, ptr));
        Data = ((Data & uintptr_t(0xF)) | uintptr_t(ptr));
        Assert_NoAssume(Ptr<const void>() == ptr);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
