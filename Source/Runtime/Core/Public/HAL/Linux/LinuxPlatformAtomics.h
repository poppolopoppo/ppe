#pragma once

#include "HAL/Generic/GenericPlatformAtomics.h"

#ifdef PLATFORM_LINUX

#include "HAL/Linux/LinuxPlatformIncludes.h"

#include <emmintrin.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FLinuxPlatformAtomics : FGenericPlatformAtomics {
public:
    STATIC_CONST_INTEGRAL(bool, HasAtomic32, true);
    STATIC_CONST_INTEGRAL(bool, HasAtomic64, sizeof(i64) == sizeof(void*));
    STATIC_CONST_INTEGRAL(bool, HasAtomicPtr, true);

    //------------------------------------------------------------------------
    // Increment

    static FORCE_INLINE i8 Increment(volatile i8* dst) {
        return __sync_fetch_and_add(dst, 1) + 1;
    }

    static FORCE_INLINE i16 Increment(volatile i16* dst) {
        return __sync_fetch_and_add(dst, 1) + 1;
    }

    static FORCE_INLINE i32 Increment(volatile i32* dst) {
        return __sync_fetch_and_add(dst, 1) + 1;
    }

    static FORCE_INLINE i64 Increment(volatile i64* dst) {
        return __sync_fetch_and_add(dst, 1) + 1;
    }

    //------------------------------------------------------------------------
    // Decrement

    static FORCE_INLINE i8 Decrement(volatile i8* dst) {
        return __sync_fetch_and_sub(dst, 1) - 1;
    }

    static FORCE_INLINE i16 Decrement(volatile i16* dst) {
        return __sync_fetch_and_sub(dst, 1) - 1;
    }

    static FORCE_INLINE i32 Decrement(volatile i32* dst) {
        return __sync_fetch_and_sub(dst, 1) - 1;
    }

    static FORCE_INLINE i64 Decrement(volatile i64* dst) {
        return __sync_fetch_and_sub(dst, 1) - 1;
    }

    //------------------------------------------------------------------------
    // Add

    static FORCE_INLINE i8 Add(volatile i8* dst, i8 val) {
        return __sync_fetch_and_add(dst, val);
    }

    static FORCE_INLINE i16 Add(volatile i16* dst, i16 val) {
        return __sync_fetch_and_add(dst, val);
    }

    static FORCE_INLINE i32 Add(volatile i32* dst, i32 val) {
        return __sync_fetch_and_add(dst, val);
    }

    static FORCE_INLINE i64 Add(volatile i64* dst, i64 val) {
        return __sync_fetch_and_add(dst, val);
    }

    //------------------------------------------------------------------------
    // Exchange

    static FORCE_INLINE i8 Exchange(volatile i8* dst, i8 exg) {
        return __sync_lock_test_and_set(dst, exg);
    }

    static FORCE_INLINE i16 Exchange(volatile i16* dst, i16 exg) {
        return __sync_lock_test_and_set(dst, exg);
    }

    static FORCE_INLINE i32 Exchange(volatile i32* dst, i32 exg) {
        return __sync_lock_test_and_set(dst, exg);
    }

    static FORCE_INLINE i64 Exchange(volatile i64* dst, i64 exg) {
        return __sync_lock_test_and_set(dst, exg);
    }

    //------------------------------------------------------------------------
    // CompareExchange

    static FORCE_INLINE i8 CompareExchange(volatile i8* val, i8 exg, i8 cmp) {
        return __sync_val_compare_and_swap(val, cmp, exg);
    }

    static FORCE_INLINE i16 CompareExchange(volatile i16* val, i16 exg, i16 cmp) {
        return __sync_val_compare_and_swap(val, cmp, exg);
    }

    static FORCE_INLINE i32 CompareExchange(volatile i32* val, i32 exg, i32 cmp) {
        return __sync_val_compare_and_swap(val, cmp, exg);
    }

    static FORCE_INLINE i64 CompareExchange(volatile i64* val, i64 exg, i64 cmp) {
        return __sync_val_compare_and_swap(val, cmp, exg);
    }

    //------------------------------------------------------------------------
    // Fetch

    static FORCE_INLINE i8 Fetch(volatile const i8* src) {
        i8 result;
		__atomic_load((volatile i8*)src, &result, __ATOMIC_SEQ_CST);
		return result;
    }

    static FORCE_INLINE i16 Fetch(volatile const i16* src) {
        i16 result;
		__atomic_load((volatile i16*)src, &result, __ATOMIC_SEQ_CST);
		return result;
    }

    static FORCE_INLINE i32 Fetch(volatile const i32* src) {
        i32 result;
		__atomic_load((volatile i32*)src, &result, __ATOMIC_SEQ_CST);
		return result;
    }

    static FORCE_INLINE i64 Fetch(volatile const i64* src) {
        i64 result;
		__atomic_load((volatile i64*)src, &result, __ATOMIC_SEQ_CST);
		return result;
    }

    //------------------------------------------------------------------------
    // Fetch_Relaxed

    static FORCE_INLINE i8 Fetch_Relaxed(volatile const i8* src) {
        i8 result;
		__atomic_load((volatile i8*)src, &result, __ATOMIC_RELAXED);
		return result;
    }

    static FORCE_INLINE i16 Fetch_Relaxed(volatile const i16* src) {
        i16 result;
		__atomic_load((volatile i16*)src, &result, __ATOMIC_RELAXED);
		return result;
    }

    static FORCE_INLINE i32 Fetch_Relaxed(volatile const i32* src) {
        i32 result;
		__atomic_load((volatile i32*)src, &result, __ATOMIC_RELAXED);
		return result;
    }

    static FORCE_INLINE i64 Fetch_Relaxed(volatile const i64* src) {
        i64 result;
		__atomic_load((volatile i64*)src, &result, __ATOMIC_RELAXED);
		return result;
    }

    //------------------------------------------------------------------------
    // Store

    static void Store(volatile i8* dst, i8 val) { 
        __atomic_store((volatile i8*)dst, &val, __ATOMIC_SEQ_CST);
    }

    static void Store(volatile i16* dst, i16 val) { 
        __atomic_store((volatile i16*)dst, &val, __ATOMIC_SEQ_CST);
    }

    static void Store(volatile i32* dst, i32 val) { 
        __atomic_store((volatile i32*)dst, &val, __ATOMIC_SEQ_CST);
    }

    static void Store(volatile i64* dst, i64 val) { 
        __atomic_store((volatile i64*)dst, &val, __ATOMIC_SEQ_CST);
    }

    //------------------------------------------------------------------------
    // Store_Relaxed

    static void Store_Relaxed(volatile i8* dst, i8 val) { 
        __atomic_store((volatile i8*)dst, &val, __ATOMIC_RELAXED);
    }

    static void Store_Relaxed(volatile i16* dst, i16 val) { 
        __atomic_store((volatile i16*)dst, &val, __ATOMIC_RELAXED);
    }

    static void Store_Relaxed(volatile i32* dst, i32 val) { 
        __atomic_store((volatile i32*)dst, &val, __ATOMIC_RELAXED);
    }

    static void Store_Relaxed(volatile i64* dst, i64 val) { 
        __atomic_store((volatile i64*)dst, &val, __ATOMIC_RELAXED);
    }

    //------------------------------------------------------------------------
    // Misc

    static FORCE_INLINE void MemoryBarrier() {
        ::_mm_mfence();
    }

    static FORCE_INLINE void ShortSyncWait() {
        ::_mm_pause();
    }

private: // private helpers
    static FORCE_INLINE bool IsAlignedForAtomics(void* p) {
        return Meta::IsAlignedPow2(sizeof(void*), p);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
