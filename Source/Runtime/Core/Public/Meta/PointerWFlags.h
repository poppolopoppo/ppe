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
        Assert(RawPointer() == ptr); \
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
#undef POINTERWFLAGS_BASE_DEF
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(TPointerWFlags<int>) == sizeof(void*));
STATIC_ASSERT(Meta::TIsPod< TPointerWFlags<int> >::value);
STATIC_ASSERT(Meta::TIsPod< TPointerWFlags<void> >::value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FHeapPtrWCounter { // assumed aligned on 16
    uintptr_t Counter : 4;
    uintptr_t Data : (sizeof(uintptr_t)<<3) - 4;

    CONSTEXPR void Reset() NOEXCEPT {
        Counter = Data = 0;
    }

    CONSTEXPR void Reset(const void* data, size_t n) NOEXCEPT {
        SetData(data);
        SetCounter(n);
    }

    CONSTEXPR void SetCounter(size_t n) NOEXCEPT {
        Counter = n;
        Assert_NoAssume(n == Counter);
    }

    template <typename T>
    CONSTEXPR T* GetData() const NOEXCEPT { return reinterpret_cast<T*>(Data << 4); }
    CONSTEXPR void SetData(const void* data) NOEXCEPT {
        Assert_NoAssume(Meta::IsAligned(16, data));
        Data = (uintptr_t(data) >> 4);
        Assert_NoAssume(GetData<void>() == data);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
