#pragma once

#include "Core.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMallocBinned {
public:
    static void*    Malloc(size_t size);
    static void     Free(void* ptr);
    static void*    Realloc(void* ptr, size_t size);

    static void*    AlignedMalloc(size_t size, size_t alignment);
    static void     AlignedFree(void* ptr);
    static void*    AlignedRealloc(void* ptr, size_t size, size_t alignment);

    static void     ReleasePendingBlocks();

    static size_t   SnapSize(size_t size);

    static size_t   RegionSize(void* ptr);

public:
    static constexpr size_t NumSizeClasses = 45;
    static constexpr u16 SizeClasses[NumSizeClasses] = {
        16,     0,      0,      0,      32,     0,
        48,     0,      64,     80,     96,     112,
        128,    160,    192,    224,    256,    320,
        384,    448,    512,    640,    768,    896,
        1024,   1280,   1536,   1792,   2048,   2560,
        3072,   3584,   4096,   5120,   6144,   7168,
        8192,   10240,  12288,  14336,  16384,  20480,
        24576,  28672,  32736,
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
