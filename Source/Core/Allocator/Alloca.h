#pragma once

#include "Core/Core.h"

#include "Core/Memory/MemoryView.h"
#include "Core/Memory/UniquePtr.h"

#include <malloc.h>

#define WITH_CORE_USE_SYSALLOCA 1 //%__NOCOMMIT%

#if 0
//  16k means you have changed your default stack size in your compiler settings
#   define CORE_SYSALLOCA_SIZELIMIT (16<<10) // bytes
#elif 1
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
void* RelocateAlloca(void* ptr, size_t newSizeInBytes);
//----------------------------------------------------------------------------
void FreeAlloca(void *ptr);
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T *TypedAlloca(size_t count) {
    return reinterpret_cast<T *>( Alloca(count * sizeof(T)) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
struct AllocaBlock {
    T* RawData;

#ifdef ARCH_X64
    u64 Count : 63;
    u64 ExternalAlloc : 1;
#else
    u32 Count : 31;
    u32 ExternalAlloc : 1;
#endif

    AllocaBlock(T* rawData, size_t count)
    :   RawData(rawData), Count(count), ExternalAlloc(1) {
        if (nullptr == RawData) {
            RawData = TypedAlloca< T >(Count);
            ExternalAlloc = 0;
        }
        Assert(RawData);
    }

    ~AllocaBlock() {
        if (0 == ExternalAlloc) {
            Assert(RawData);
            FreeAlloca(RawData);
        }
    }

    MemoryView<T> MakeView() const { return MemoryView<T>(RawData, Count); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if WITH_CORE_USE_SYSALLOCA
#   define SYSALLOCA_IFP(_SIZEINBYTES) ((CORE_SYSALLOCA_SIZELIMIT >= (_SIZEINBYTES) ) ? _alloca(_SIZEINBYTES) : nullptr )
#else
#   define SYSALLOCA_IFP(_SIZEINBYTES) nullptr
#endif
//----------------------------------------------------------------------------
#define MALLOCA(T, _NAME, _COUNT) \
    const size_t CONCAT(_Count_, _NAME) = (_COUNT); \
    const Core::AllocaBlock<T> _NAME( static_cast< T* >( \
        SYSALLOCA_IFP(sizeof(T) * CONCAT(_Count_, _NAME))), \
        CONCAT(_Count_, _NAME) )
//----------------------------------------------------------------------------
#define STACKLOCAL_POD_ARRAY(T, _NAME, _COUNT) \
    MALLOCA(T, CONCAT(_Alloca_, _NAME), _COUNT ); \
    const Core::MemoryView< T > _NAME( CONCAT(_Alloca_, _NAME).MakeView() )
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class AllocaStartup {
public:
    static void Start(bool mainThread);
    static void Shutdown();

    AllocaStartup(bool mainThread) { Start(mainThread); }
    ~AllocaStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
