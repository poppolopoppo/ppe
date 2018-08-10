#pragma once

#include "HAL/Generic/GenericPlatformAtomics.h"

#ifdef PLATFORM_WINDOWS

#include "HAL/Windows/WindowsPlatformIncludes.h"

#include <intrin.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformAtomics : FGenericPlatformAtomics {
public:
    STATIC_CONST_INTEGRAL(bool, HasAtomic32, true);
    STATIC_CONST_INTEGRAL(bool, HasAtomic64, sizeof(i64) == sizeof(void*));
    STATIC_CONST_INTEGRAL(bool, HasAtomicPtr, true);

    STATIC_ASSERT(sizeof(char) == sizeof(i8));
    STATIC_ASSERT(sizeof(short) == sizeof(i16));
    STATIC_ASSERT(sizeof(long) == sizeof(i32));
    STATIC_ASSERT(sizeof(long long) == sizeof(i64));

    //------------------------------------------------------------------------
    // Increment

    static FORCE_INLINE i8 Increment(volatile i8* dst) {
        return (i8)::_InterlockedExchangeAdd8((volatile char*)dst, 1);
    }

    static FORCE_INLINE i16 Increment(volatile i16* dst) {
        return (i16)::_InterlockedIncrement16((volatile short*)dst);
    }

    static FORCE_INLINE i32 Increment(volatile i32* dst) {
        return (i32)::_InterlockedIncrement((volatile long*)dst);
    }

    static FORCE_INLINE i64 Increment(volatile i64* dst) {
#ifdef ARCH_X64
        return (i64)::_InterlockedIncrement64((volatile long long*)dst);
#else
        // No explicit instruction for 64-bit atomic increment on 32-bit processors; has to be implemented in terms of CMPXCHG8B
        for (;;) {
            i64 old = *dst;
            if (_InterlockedCompareExchange64(dst, old + 1, old) == old)
                return (old + 1);
        }
#endif
    }

    //------------------------------------------------------------------------
    // Decrement

    static FORCE_INLINE i8 Decrement(volatile i8* dst) {
        return (i8)::_InterlockedExchangeAdd8((volatile char*)dst, -1);
    }

    static FORCE_INLINE i16 Decrement(volatile i16* dst) {
        return (i16)::_InterlockedDecrement16((volatile short*)dst);
    }

    static FORCE_INLINE i32 Decrement(volatile i32* dst) {
        return (i32)::_InterlockedDecrement((volatile long*)dst);
    }

    static FORCE_INLINE i64 Decrement(volatile i64* dst) {
#ifdef ARCH_X64
        return (i64)::_InterlockedDecrement64((volatile long long*)dst);
#else
        // No explicit instruction for 64-bit atomic increment on 32-bit processors; has to be implemented in terms of CMPXCHG8B
        for (;;) {
            i64 old = *dst;
            if (_InterlockedCompareExchange64(dst, old - 1, old) == old)
                return (old - 1);
        }
#endif
    }

    //------------------------------------------------------------------------
    // Add

    static FORCE_INLINE i8 Add(volatile i8* dst, i8 val) {
        return (i8)::_InterlockedExchangeAdd8((volatile char*)dst, char(val));
    }

    static FORCE_INLINE i16 Add(volatile i16* dst, i16 val) {
        return (i16)::_InterlockedExchangeAdd16((volatile short*)dst, short(val));
    }

    static FORCE_INLINE i32 Add(volatile i32* dst, i32 val) {
        return (i32)::_InterlockedExchangeAdd((volatile long*)dst, long(val));
    }

    static FORCE_INLINE i64 Add(volatile i64* dst, i64 val) {
#ifdef ARCH_X64
        return (i64)::_InterlockedExchangeAdd64((volatile long long*)dst, long long(val));
#else
        // No explicit instruction for 64-bit atomic increment on 32-bit processors; has to be implemented in terms of CMPXCHG8B
        for (;;) {
            i64 old = *dst;
            if (_InterlockedCompareExchange64(dst, old + val, old) == old)
                return (old + val);
        }
#endif
    }

    //------------------------------------------------------------------------
    // Exchange

    static FORCE_INLINE i8 Exchange(volatile i8* dst, i8 exg) {
        return (i8)::_InterlockedExchange8((volatile char*)dst, char(exg));
    }

    static FORCE_INLINE i16 Exchange(volatile i16* dst, i16 exg) {
        return (i16)::_InterlockedExchange16((volatile short*)dst, short(exg));
    }

    static FORCE_INLINE i32 Exchange(volatile i32* dst, i32 exg) {
        return (i32)::_InterlockedExchange((volatile long*)dst, long(exg));
    }

    static FORCE_INLINE i64 Exchange(volatile i64* dst, i64 exg) {
#ifdef ARCH_X64
        return (i64)::_InterlockedExchange64((volatile long long*)dst, long long(exg));
#else
        // No explicit instruction for 64-bit atomic increment on 32-bit processors; has to be implemented in terms of CMPXCHG8B
        for (;;) {
            i64 old = *dst;
            if (_InterlockedCompareExchange64(dst, exg, old) == old)
                return old;
        }
#endif
    }

    //------------------------------------------------------------------------
    // CompareExchange

    static FORCE_INLINE i8 CompareExchange(volatile i8* val, i8 exg, i8 cmp) {
        return (i8)::_InterlockedCompareExchange8((volatile char*)val, char(exg), char(cmp));
    }

    static FORCE_INLINE i16 CompareExchange(volatile i16* val, i16 exg, i16 cmp) {
        return (i16)::_InterlockedCompareExchange16((volatile short*)val, short(exg), short(cmp));
    }

    static FORCE_INLINE i32 CompareExchange(volatile i32* val, i32 exg, i32 cmp) {
        return (long)::_InterlockedCompareExchange((volatile long*)val, long(exg), long(cmp));
    }

    static FORCE_INLINE i64 CompareExchange(volatile i64* val, i64 exg, i64 cmp) {
        return (i64)::_InterlockedCompareExchange64((volatile long long*)val, long long(exg), long long(cmp));
    }

    //------------------------------------------------------------------------
    // Fetch

    static FORCE_INLINE i8 Fetch(volatile const i8* src) { return CompareExchange((volatile i8*)src, i8(0), i8(0)); }
    static FORCE_INLINE i16 Fetch(volatile const i16* src) { return CompareExchange((volatile i16*)src, i16(0), i16(0)); }
    static FORCE_INLINE i32 Fetch(volatile const i32* src) { return CompareExchange((volatile i32*)src, i32(0), i32(0)); }
    static FORCE_INLINE i64 Fetch(volatile const i64* src) { return CompareExchange((volatile i64*)src, i64(0), i64(0)); }

    //------------------------------------------------------------------------
    // Fetch_Relaxed

    static FORCE_INLINE i8 Fetch_Relaxed(volatile const i8* src) { return (*src); }
    static FORCE_INLINE i16 Fetch_Relaxed(volatile const i16* src) { return (*src); }
    static FORCE_INLINE i32 Fetch_Relaxed(volatile const i32* src) { return (*src); }
    static FORCE_INLINE i64 Fetch_Relaxed(volatile const i64* src) { return (*src); }

    //------------------------------------------------------------------------
    // Store

    static void Store(volatile i8* dst, i8 val) { Exchange(dst, val); }
    static void Store(volatile i16* dst, i16 val) { Exchange(dst, val); }
    static void Store(volatile i32* dst, i32 val) { Exchange(dst, val); }
    static void Store(volatile i64* dst, i64 val) { Exchange(dst, val); }

    //------------------------------------------------------------------------
    // Store_Relaxed

    static void Store_Relaxed(volatile i8* dst, i8 val) { *dst = val; }
    static void Store_Relaxed(volatile i16* dst, i16 val) { *dst = val; }
    static void Store_Relaxed(volatile i32* dst, i32 val) { *dst = val; }
    static void Store_Relaxed(volatile i64* dst, i64 val) { *dst = val; }

    //------------------------------------------------------------------------
    // Misc

    static FORCE_INLINE void Barrier() {
        ::MemoryBarrier();
    }

    static FORCE_INLINE void ShortSyncWait() {
        ::_mm_pause();
    }

private: // private helpers
    static FORCE_INLINE bool IsAlignedForAtomics(void* p) {
        return Meta::IsAligned(sizeof(void*), p);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
