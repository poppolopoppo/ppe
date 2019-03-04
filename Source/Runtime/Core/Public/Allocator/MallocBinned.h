#pragma once

#include "Core.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FMallocBinned {
public:
    static void*    Malloc(size_t size);
    static void     Free(void* ptr);
    static void*    Realloc(void* ptr, size_t size);

    static void*    AlignedMalloc(size_t size, size_t alignment);
    static void     AlignedFree(void* ptr);
    static void*    AlignedRealloc(void* ptr, size_t size, size_t alignment);

    static void     ReleaseCacheMemory();
    static void     ReleasePendingBlocks();

    static size_t   SnapSize(size_t size);

    static size_t   RegionSize(void* ptr);

public:
    // made public so that external code can align on block sizes
    static constexpr size_t MinSizeClass = 19;
    static constexpr size_t NumSizeClasses = 45;
    static constexpr u16 SizeClasses[NumSizeClasses] = {
        /*  0 */    16    , 0     , 0     , 0     ,
        /*  4 */    32    , 0     , 48    , 0     ,
        /*  8 */    64    , 80    , 96    , 112   ,
        /* 12 */    128   , 160   , 192   , 224   ,
        /* 16 */    256   , 320   , 384   , 448   ,
        /* 20 */    512   , 640   , 768   , 896   ,
        /* 24 */    1024  , 1280  , 1536  , 1792  ,
        /* 28 */    2048  , 2560  , 3072  , 3584  ,
        /* 32 */    4096  , 5120  , 6144  , 7168  ,
        /* 36 */    8192  , 10240 , 12288 , 14336 ,
        /* 40 */    16384 , 20480 , 24576 , 28672 ,
        /* 44 */    32768
    };

    static constexpr size_t SnapSizeConstexpr(size_t size, size_t index = 0) {
        return (index < NumSizeClasses
            ? (SizeClasses[index] < size
                ? SnapSizeConstexpr(size, index + 1)
                : SizeClasses[index])
            : ROUND_TO_NEXT_64(size) );
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
