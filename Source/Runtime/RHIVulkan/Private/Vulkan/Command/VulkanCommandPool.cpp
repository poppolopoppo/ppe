
#include "stdafx.h"

#include "Vulkan/Command/VulkanCommandPool.h"

#include "Vulkan/Instance/VulkanDevice.h"

#if USE_PPE_RHIDEBUG
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#endif

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FVulkanCommandPool::~FVulkanCommandPool() {
    Assert_NoAssume(_pool == VK_NULL_HANDLE); // must call TearDown() !
}
//----------------------------------------------------------------------------
bool FVulkanCommandPool::Construct(
    const FVulkanDevice& device,
    const PVulkanDeviceQueue& queue
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    Assert(queue);

    const FCriticalScope scopeLock{ &_poolCS };
    Assert_NoAssume(VK_NULL_HANDLE == _pool);

    VkCommandPoolCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.queueFamilyIndex = static_cast<u32>(queue->FamilyIndex);

    VK_CHECK( device.vkCreateCommandPool( device.vkDevice(), &info, device.vkAllocator(), &_pool ) );

#if USE_PPE_RHIDEBUG
    _debugName = debugName;
    if (not _debugName.empty())
        device.SetObjectName(_pool, _debugName.c_str(), VK_OBJECT_TYPE_COMMAND_POOL);
#endif

    return true;
}
//----------------------------------------------------------------------------
void FVulkanCommandPool::TearDown(const FVulkanDevice& device) {
    const FCriticalScope scopeLock{ &_poolCS };
    Assert(VK_NULL_HANDLE != _pool);

    _freePrimaries.clear();
    _freeSecondaries.clear();

    device.vkDestroyCommandPool( device.vkDevice(), _pool, device.vkAllocator() );
    _pool = VK_NULL_HANDLE;

#if USE_PPE_RHIDEBUG
    LOG(RHI, Info, L"Max live command buffers in <{0}> pool: {1}", _debugName.Str(), _maxBuffersAllocated.load(std::memory_order_relaxed));
    _maxBuffersAllocated.store(0);
#endif
}
//----------------------------------------------------------------------------
void FVulkanCommandPool::ResetAll(const FVulkanDevice& device, VkCommandPoolResetFlags flags) {
    const FCriticalScope scopeLock{ &_poolCS };
    Assert(VK_NULL_HANDLE != _pool);

    VK_CALL( device.vkResetCommandPool( device.vkDevice(), _pool, flags ) );
}
//----------------------------------------------------------------------------
void FVulkanCommandPool::TrimAll(const FVulkanDevice& device, VkCommandPoolTrimFlags flags) {
    const FCriticalScope scopeLock{ &_poolCS };
    Assert(VK_NULL_HANDLE != _pool);

    device.vkTrimCommandPool( device.vkDevice(), _pool, flags );
}
//----------------------------------------------------------------------------
void FVulkanCommandPool::ResetBuffer(const FVulkanDevice& device, VkCommandBuffer cmd, VkCommandBufferResetFlags flags) {
    Assert(VK_NULL_HANDLE != cmd);
    //const FCriticalScope scopeLock{ &_poolCS }; // #TODO: not necessary ?
    Assert(VK_NULL_HANDLE != _pool);

    VK_CALL( device.vkResetCommandBuffer( cmd, flags ) );
}
//----------------------------------------------------------------------------
void FVulkanCommandPool::RecyclePrimary(VkCommandBuffer cmd) {
    Assert(VK_NULL_HANDLE != cmd);
    const FCriticalScope scopeLock{ &_poolCS };
    Assert(VK_NULL_HANDLE != _pool);

    _freePrimaries.Queue(cmd);
}
//----------------------------------------------------------------------------
void FVulkanCommandPool::RecycleSecondary(VkCommandBuffer cmd) {
    Assert(VK_NULL_HANDLE != cmd);
    const FCriticalScope scopeLock{ &_poolCS };
    Assert(VK_NULL_HANDLE != _pool);

    _freeSecondaries.Queue(cmd);
}
//----------------------------------------------------------------------------
VkCommandBuffer FVulkanCommandPool::AllocBuffer_(FFreePool* pPool, const FVulkanDevice& device, VkCommandBufferLevel level) {
    Assert(pPool);
    Assert(VK_NULL_HANDLE != _pool);

    VkCommandBuffer cmd = VK_NULL_HANDLE;

    // first look in cache
    {
        const FCriticalScope scopeLock{ &_poolCS };
        if (pPool->Dequeue(&cmd)) {
            Assert_NoAssume(VK_NULL_HANDLE != cmd);
            return cmd;
        }
    }

    // allocate a new command buffer
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.pNext = nullptr;
    info.commandPool = _pool;
    info.level = level;
    info.commandBufferCount = 1;

    VK_CHECK( device.vkAllocateCommandBuffers( device.vkDevice(), &info, &cmd ) );

    ONLY_IF_RHIDEBUG( ++_maxBuffersAllocated );
    return cmd;
}
//----------------------------------------------------------------------------
VkCommandBuffer FVulkanCommandPool::AllocPrimary(const FVulkanDevice& device) {
    return AllocBuffer_(&_freePrimaries, device, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}
//----------------------------------------------------------------------------
VkCommandBuffer FVulkanCommandPool::AllocSecondary(const FVulkanDevice& device) {
    return AllocBuffer_(&_freeSecondaries, device, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
}
//----------------------------------------------------------------------------
void FVulkanCommandPool::Deallocate(const FVulkanDevice& device, VkCommandBuffer cmd) {
    Assert(VK_NULL_HANDLE != cmd);
    Assert(VK_NULL_HANDLE != _pool);

    device.vkFreeCommandBuffers( device.vkDevice(), _pool, 1, &cmd );

    ONLY_IF_RHIDEBUG( Verify( _maxBuffersAllocated.fetch_sub(1, std::memory_order_relaxed) > 0) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
