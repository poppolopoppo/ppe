#pragma once

#include "Core/Memory/MemoryPool.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
MemoryPool<_Lock, _Allocator>::MemoryPool(size_t blockSize, size_t chunkSize)
:   MemoryPoolBase(blockSize, chunkSize) {}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
MemoryPool<_Lock, _Allocator>::MemoryPool(size_t blockSize, size_t chunkSize, allocator_type&& allocator)
:   MemoryPoolBase(blockSize, chunkSize),
    allocator_type(std::move(allocator)) {}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
MemoryPool<_Lock, _Allocator>::MemoryPool(size_t blockSize, size_t chunkSize, const allocator_type& allocator)
:   MemoryPoolBase(blockSize, chunkSize),
    allocator_type(allocator) {}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
MemoryPool<_Lock, _Allocator>::~MemoryPool() {
    Clear_AssertCompletelyFree();
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void *MemoryPool<_Lock, _Allocator>::Allocate(MemoryTrackingData *trackingData = nullptr) {
    threadlock_type::ScopeLock lock(*this);

    void *ptr;
    if (nullptr == (ptr = TryAllocate_FailIfNoBlockAvailable())) {
        MemoryPoolChunk *const newChunk = AllocateChunk_();
        ptr = newChunk->AllocateBlock(BlockSize(), 1);
        AddChunk(newChunk);

#ifdef USE_MEMORY_DOMAINS
        if (trackingData && trackingData->Parent())
            trackingData->Parent()->Pool_AllocateOneChunk(ChunkSize());
#endif
    }
    Assert(ptr);

#ifdef USE_MEMORY_DOMAINS
    if (trackingData)
        trackingData->Pool_AllocateOneBlock(BlockSize());
#endif

    return ptr;
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void MemoryPool<_Lock, _Allocator>::Deallocate(void *ptr, MemoryTrackingData *trackingData = nullptr) {
    Assert(ptr);

    threadlock_type::ScopeLock lock(*this);

#ifdef USE_MEMORY_DOMAINS
    if (trackingData)
        trackingData->Pool_DeallocateOneBlock(BlockSize());
#endif

    MemoryPoolChunk *chunk;
    if (nullptr == (chunk = Deallocate_ReturnChunkToRelease(ptr)))
        return;

    DeallocateChunk_(chunk);

#ifdef USE_MEMORY_DOMAINS
    if (trackingData && trackingData->Parent())
        trackingData->Parent()->Pool_DeallocateOneChunk(ChunkSize());
#endif
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void MemoryPool<_Lock, _Allocator>::Clear_AssertCompletelyFree() {
    threadlock_type::ScopeLock lock(*this);

    MemoryPoolChunk *chunk;
    while ((chunk = ClearOneChunk_AssertCompletelyFree())) {
        DeallocateChunk_(chunk);
    }
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void MemoryPool<_Lock, _Allocator>::Clear_IgnoreLeaks() {
    threadlock_type::ScopeLock lock(*this);

    MemoryPoolChunk *chunk;
    while ((chunk = ClearOneChunk_IgnoreLeaks())) {
        DeallocateChunk_(chunk);
    }
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void MemoryPool<_Lock, _Allocator>::Clear_UnusedMemory() {
    threadlock_type::ScopeLock lock(*this);

    MemoryPoolChunk *chunk;
    while ((chunk = ClearOneChunk_UnusedMemory())) {
        DeallocateChunk_(chunk);
    }
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
MemoryPoolChunk *MemoryPool<_Lock, _Allocator>::AllocateChunk_() {
    return new (allocator_type::allocate(ChunkSize() / sizeof(allocator_type::value_type))) MemoryPoolChunk();
}
//----------------------------------------------------------------------------
template <bool _Lock, typename _Allocator>
void MemoryPool<_Lock, _Allocator>::DeallocateChunk_(MemoryPoolChunk *chunk) {
    Assert(chunk);

    chunk->~MemoryPoolChunk();
    allocator_type::deallocate(chunk, ChunkSize() / sizeof(allocator_type::value_type));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
