#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"
#include "Memory/MemoryView.h"

#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericMemoryTypeFlags : u32 {
    None          = 0,
    DeviceLocal   = 1<<0, // sit in GPU VRam, no CPU access
    HostVisible   = 1<<1, // CPU needs to write GPU data
    HostCoherent  = 1<<2, // no need to flush CPU<->GPU to make visible
    HostCached    = 1<<3, // CPU needs to read-back GPU data
};
ENUM_FLAGS(EGenericMemoryTypeFlags);
//----------------------------------------------------------------------------
struct FGenericMemoryBlock {
    FGenericDeviceMemory DeviceMemory;
    u32 Size;
    u32 MemoryType;
    EGenericMemoryTypeFlags Flags;
};
//----------------------------------------------------------------------------
class FGenericMemoryAllocator : Meta::FNonCopyable {
public:
    FGenericMemoryBlock Allocate(
        u32 size,
        u32 memoryTypeBits,
        EGenericMemoryTypeFlags requiredFlags,
        EGenericMemoryTypeFlags preferredFlags = EGenericMemoryTypeFlags(0) ) = delete;

    void Deallocate(const FGenericMemoryBlock& block) NOEXCEPT = delete;

    FRawMemory MapMemory(const FGenericMemoryBlock& block, u32 offset = 0, u32 size = 0) = delete;
    void UnmapMemory(const FGenericMemoryBlock& block) NOEXCEPT = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
