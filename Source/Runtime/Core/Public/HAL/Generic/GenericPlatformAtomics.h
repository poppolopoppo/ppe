#pragma once

#include "HAL/TargetPlatform.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformAtomics {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, HasAtomic32, false);
    STATIC_CONST_INTEGRAL(bool, HasAtomic64, false);
    STATIC_CONST_INTEGRAL(bool, HasAtomicPtr, false);

    static i8 Increment(volatile i8* dst) NOEXCEPT = delete;
    static i16 Increment(volatile i16* dst) NOEXCEPT = delete;
    static i32 Increment(volatile i32* dst) NOEXCEPT = delete;
    static i64 Increment(volatile i64* dst) NOEXCEPT = delete;

    static i8 Decrement(volatile i8* dst) NOEXCEPT = delete;
    static i16 Decrement(volatile i16* dst) NOEXCEPT = delete;
    static i32 Decrement(volatile i32* dst) NOEXCEPT = delete;
    static i64 Decrement(volatile i64* dst) NOEXCEPT = delete;

    static i8 Add(volatile i8* dst, i8 val) NOEXCEPT = delete;
    static i16 Add(volatile i16* dst, i16 val) NOEXCEPT = delete;
    static i32 Add(volatile i32* dst, i32 val) NOEXCEPT = delete;
    static i64 Add(volatile i64* dst, i64 val) NOEXCEPT = delete;

    static i8 Exchange(volatile i8* dst, i8 exg) NOEXCEPT = delete;
    static i16 Exchange(volatile i16* dst, i16 exg) NOEXCEPT = delete;
    static i32 Exchange(volatile i32* dst, i32 exg) NOEXCEPT = delete;
    static i64 Exchange(volatile i64* dst, i64 exg) NOEXCEPT = delete;

    static i8 CompareExchange(volatile i8* val, i8 exg, i8 cmp) NOEXCEPT = delete;
    static i16 CompareExchange(volatile i16* val, i16 exg, i16 cmp) NOEXCEPT = delete;
    static i32 CompareExchange(volatile i32* val, i32 exg, i32 cmp) NOEXCEPT = delete;
    static i64 CompareExchange(volatile i64* val, i64 exg, i64 cmp) NOEXCEPT = delete;

    static i8 Fetch(volatile const i8* src) NOEXCEPT = delete;
    static i16 Fetch(volatile const i16* src) NOEXCEPT = delete;
    static i32 Fetch(volatile const i32* src) NOEXCEPT = delete;
    static i64 Fetch(volatile const i64* src) NOEXCEPT = delete;

    static i8 Fetch_Relaxed(volatile const i8* src) NOEXCEPT = delete;
    static i16 Fetch_Relaxed(volatile const i16* src) NOEXCEPT = delete;
    static i32 Fetch_Relaxed(volatile const i32* src) NOEXCEPT = delete;
    static i64 Fetch_Relaxed(volatile const i64* src) NOEXCEPT = delete;

    static void Store(volatile i8* dst, i8 val) NOEXCEPT = delete;
    static void Store(volatile i16* dst, i16 val) NOEXCEPT = delete;
    static void Store(volatile i32* dst, i32 val) NOEXCEPT = delete;
    static void Store(volatile i64* dst, i64 val) NOEXCEPT = delete;

    static void Store_Relaxed(volatile i8* dst, i8 val) NOEXCEPT = delete;
    static void Store_Relaxed(volatile i16* dst, i16 val) NOEXCEPT = delete;
    static void Store_Relaxed(volatile i32* dst, i32 val) NOEXCEPT = delete;
    static void Store_Relaxed(volatile i64* dst, i64 val) NOEXCEPT = delete;

    static void MemoryBarrier() NOEXCEPT = delete;
    static void ShortSyncWait() NOEXCEPT = delete;

public: // generic helpers

    template <typename T>
    static FORCE_INLINE T* ExchangePtr(volatile T** dst, T* exg) NOEXCEPT {
        Assert(Meta::IsAligned(sizeof(void*), dst));
        return (T*)FPlatformAtomics::Exchange((volatile intptr_t*)dst, (intptr_t)exg);
    }

    template <typename T>
    static FORCE_INLINE T* CompareExchangePtr(volatile T** dst, T* exg, T* cmp) NOEXCEPT {
        Assert(Meta::IsAligned(sizeof(void*), dst));
        return (T*)FPlatformAtomics::CompareExchange((volatile intptr_t*)dst, (intptr_t)exg, (intptr_t)cmp);
    }

    template <typename T>
    static FORCE_INLINE T* FetchPtr(const T** src) NOEXCEPT {
        Assert(Meta::IsAligned(sizeof(void*), src));
        return (T*)FPlatformAtomics::Fetch((volatile const intptr_t*)src);
    }

    template <typename T>
    static FORCE_INLINE T* FetchPtr_Relaxed(const T** src) NOEXCEPT {
        Assert(Meta::IsAligned(sizeof(void*), src));
        return (T*)FPlatformAtomics::Fetch_Relaxed((volatile const intptr_t*)src);
    }

    template <typename T>
    static FORCE_INLINE void StorePtr(T** dst, T* val) NOEXCEPT {
        Assert(Meta::IsAligned(sizeof(void*), dst));
        FPlatformAtomics::Store((volatile intptr_t**)dst, (intptr_t)val);
    }

    template <typename T>
    static FORCE_INLINE void StorePtr_Relaxed(T** dst, T* val) NOEXCEPT {
        Assert(Meta::IsAligned(sizeof(void*), dst));
        FPlatformAtomics::Store_Relaxed((volatile intptr_t*)dst, (intptr_t)val);
    }

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
