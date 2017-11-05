#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryView.h"
#include "Core/Memory/UniquePtr.h"

#include <malloc.h>

#define USE_CORE_SYSALLOCA (!USE_CORE_MEMORY_DEBUGGING) //%__NOCOMMIT%

#if 1
//  16k means you have changed your default stack size in your compiler settings
#   define CORE_SYSALLOCA_SIZELIMIT (16<<10) // bytes
#elif 0
//  2k is a good trade off with default compiler settings and a wise user
#   define CORE_SYSALLOCA_SIZELIMIT (2<<10) // bytes
#else
//  1k is recommended with default compiler settings
#   define CORE_SYSALLOCA_SIZELIMIT (1<<10) // bytes
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* Alloca(size_t sizeInBytes);
//----------------------------------------------------------------------------
void* RelocateAlloca(void* ptr, size_t newSizeInBytes, bool keepData);
//----------------------------------------------------------------------------
void FreeAlloca(void *ptr);
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T *TypedAlloca(size_t count) {
    return reinterpret_cast<T *>( Alloca(count * sizeof(T)) );
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T *TypedRelocateAlloca(T* ptr, size_t count, bool keepData) {
    return reinterpret_cast<T *>( RelocateAlloca(ptr, count * sizeof(T), keepData) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct TAllocaBlock {
    T* RawData;

#ifdef ARCH_X64
    u64 Count : 63;
    u64 UsingSysAlloca : 1;
#else
    u32 Count : 31;
    u32 UsingSysAlloca : 1;
#endif

    TAllocaBlock() : RawData(nullptr), Count(0), UsingSysAlloca(0) {}

    TAllocaBlock(T* rawData, size_t count)
        : RawData(rawData), Count(count), UsingSysAlloca(1) {
        if (nullptr == RawData) {
            RawData = TypedAlloca< T >(Count);
            UsingSysAlloca = 0;
        }
        Assert(RawData);
    }

    ~TAllocaBlock() {
        if (not UsingSysAlloca && RawData)
            FreeAlloca(RawData);
    }

    TAllocaBlock(const TAllocaBlock& ) = delete;
    TAllocaBlock& operator =(const TAllocaBlock& ) = delete;

    TAllocaBlock(TAllocaBlock&& rvalue)
        : TAllocaBlock() {
        std::swap(*this, rvalue);
    }

    TAllocaBlock& operator =(TAllocaBlock&& rvalue) {
        if (not UsingSysAlloca && RawData)
            FreeAlloca(RawData);

        RawData = nullptr;
        Count = 0;
        UsingSysAlloca = 0;
        std::swap(*this, rvalue);

        return (*this);
    }

    void Relocate(size_t newCount, bool keepData = true) {
        Assert(newCount != Count);
        Assert(!UsingSysAlloca);
        RawData = TypedRelocateAlloca(RawData, newCount, keepData);
        Count = newCount;
    }

    void RelocateIFP(size_t newCount, bool keepData = true) {
        if (newCount > Count)
            Relocate(newCount, keepData);
    }

    operator TMemoryView<T> () const { return MakeView(); }
    TMemoryView<T> MakeView() const { return TMemoryView<T>(RawData, Count); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_CORE_SYSALLOCA
#   define SYSALLOCA_IFP(_SIZEINBYTES) ((CORE_SYSALLOCA_SIZELIMIT >= (_SIZEINBYTES) ) ? _alloca(_SIZEINBYTES) : nullptr )
#else
#   define SYSALLOCA_IFP(_SIZEINBYTES) nullptr
#endif
//----------------------------------------------------------------------------
#define MALLOCA(T, _NAME, _COUNT) \
    const size_t CONCAT(_Count_, _NAME) = (_COUNT); \
    const ::Core::TAllocaBlock<T> _NAME( static_cast< T* >( \
        SYSALLOCA_IFP(sizeof(T) * CONCAT(_Count_, _NAME))), \
        CONCAT(_Count_, _NAME) )
//----------------------------------------------------------------------------
#define INLINE_MALLOCA(T, _COUNT) \
    ::Core::TAllocaBlock<T>( static_cast< T* >(SYSALLOCA_IFP(sizeof(T) * _COUNT)), _COUNT )
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_ARRAY(T, _NAME, _COUNT) \
    MALLOCA(T, CONCAT(_Alloca_, _NAME), _COUNT ); \
    const ::Core::TMemoryView< T > _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FAllocaStartup {
public:
    static void Start(bool mainThread);
    static void Shutdown();

    FAllocaStartup(bool mainThread) { Start(mainThread); }
    ~FAllocaStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
