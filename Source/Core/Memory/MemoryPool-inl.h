#pragma once

#include "Core/Memory/MemoryPool.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
MemoryPool<_Lock, _Allocator>::MemoryPool(size_t blockSize, size_t baseChunkSize, size_t maxChunkSize)
:   MemoryPoolBase(blockSize, baseChunkSize, maxChunkSize) {}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
MemoryPool<_Lock, _Allocator>::MemoryPool(size_t blockSize, size_t baseChunkSize, size_t maxChunkSize, allocator_type&& allocator)
:   MemoryPoolBase(blockSize, baseChunkSize, maxChunkSize),
    allocator_type(std::move(allocator)) {}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
MemoryPool<_Lock, _Allocator>::MemoryPool(size_t blockSize, size_t baseChunkSize, size_t maxChunkSize, const allocator_type& allocator)
:   MemoryPoolBase(blockSize, baseChunkSize, maxChunkSize),
    allocator_type(allocator) {}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
MemoryPool<_Lock, _Allocator>::~MemoryPool() {
    Clear_AssertCompletelyFree();
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void *MemoryPool<_Lock, _Allocator>::Allocate(MemoryTrackingData *trackingData /* = nullptr */) {
#ifndef USE_MEMORY_DOMAINS
    UNUSED(trackingData);
#endif
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    if (trackingData)
        trackingData->Allocate(1, BlockSize());

    return Core::aligned_malloc(BlockSize(), 16);

#else
    typename threadlock_type::ScopeLock lock(*this);

    void *ptr = TryAllocate_FailIfNoBlockAvailable();
    if (nullptr == ptr) {

        if (MemoryPoolBase::Chunks())
            MemoryPoolBase::GrowChunkSizeIFP();

        MemoryPoolChunk *const newChunk = AllocateChunk_();
        AddChunk(newChunk);

        ptr = TryAllocate_FailIfNoBlockAvailable();
        Assert(ptr);

#ifdef USE_MEMORY_DOMAINS
        if (trackingData && trackingData->Parent())
            trackingData->Parent()->Pool_AllocateOneChunk(newChunk->ChunkSize());
#endif
    }

#ifdef USE_MEMORY_DOMAINS
    if (trackingData)
        trackingData->Pool_AllocateOneBlock(BlockSize());
#endif

    return ptr;

#endif
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void MemoryPool<_Lock, _Allocator>::Deallocate(void *ptr, MemoryTrackingData *trackingData /* = nullptr */) {
#ifndef USE_MEMORY_DOMAINS
    UNUSED(trackingData);
#endif
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    Core::aligned_free(ptr);

    if (trackingData)
        trackingData->Deallocate(1, BlockSize());

#else
    Assert(ptr);

    typename threadlock_type::ScopeLock lock(*this);

#ifdef USE_MEMORY_DOMAINS
    if (trackingData)
        trackingData->Pool_DeallocateOneBlock(BlockSize());
#endif

    MemoryPoolChunk *chunk;
    if (nullptr == (chunk = Deallocate_ReturnChunkToRelease(ptr)))
        return;

#ifdef USE_MEMORY_DOMAINS
    if (trackingData && trackingData->Parent())
        trackingData->Parent()->Pool_DeallocateOneChunk(chunk->ChunkSize());
#endif

    DeallocateChunk_(chunk);

#endif
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void MemoryPool<_Lock, _Allocator>::Clear_AssertCompletelyFree() {
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertRelease(nullptr == Chunks());

#else
    typename threadlock_type::ScopeLock lock(*this);

    MemoryPoolChunk *chunk;
    while (nullptr != (chunk = ClearOneChunk_AssertCompletelyFree())) {
        DeallocateChunk_(chunk);
    }

#endif
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void MemoryPool<_Lock, _Allocator>::Clear_IgnoreLeaks() {
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertRelease(nullptr == Chunks());

#else
    typename threadlock_type::ScopeLock lock(*this);

    MemoryPoolChunk *chunk;
    while (nullptr != (chunk = ClearOneChunk_IgnoreLeaks())) {
        DeallocateChunk_(chunk);
    }

#endif
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void MemoryPool<_Lock, _Allocator>::Clear_UnusedMemory() {
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertRelease(nullptr == Chunks());

#else
    typename threadlock_type::ScopeLock lock(*this);

    MemoryPoolChunk *chunk;
    while (nullptr != (chunk = ClearOneChunk_UnusedMemory())) {
        DeallocateChunk_(chunk);
    }

#endif
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
MemoryPoolChunk *MemoryPool<_Lock, _Allocator>::AllocateChunk_() {
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertNotReached();
#else
    const size_t currentChunkSize = CurrentChunkSize();
    const size_t currentBlockCount = BlockCountPerChunk(currentChunkSize);
    void* const storage = allocator_type::allocate(currentChunkSize / sizeof(typename allocator_type::value_type));
    return new (storage) MemoryPoolChunk(currentChunkSize, currentBlockCount);
#endif
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void MemoryPool<_Lock, _Allocator>::DeallocateChunk_(MemoryPoolChunk *chunk) {
#ifdef WITH_CORE_MEMORYPOOL_FALLBACK_TO_MALLOC
    AssertNotReached();

#else
    Assert(chunk);
    const size_t chunkSize = chunk->ChunkSize();
    chunk->~MemoryPoolChunk();
    allocator_type::deallocate(chunk, chunkSize / sizeof(typename allocator_type::value_type));
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
