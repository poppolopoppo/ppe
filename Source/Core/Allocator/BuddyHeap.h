#pragma once

#include "Core/Core.h"

#include <thread>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct BuddyBlock;
class BuddyChunk {
public:
    BuddyChunk(const BuddyChunk& ) = delete;
    BuddyChunk& operator =(const BuddyChunk& ) = delete;

    size_t CapacityInBytes() const { return _capacityInBytes; }
    size_t ConsumedInBytes() const { return _consumedInBytes; }
    size_t BlockCount() const { return _blockCount; }

    BuddyChunk *Sibling() const { return _sibling; }
    void SetSibling(BuddyChunk *chunk) { _sibling = chunk; }

    void *Allocate(size_t sizeInBytes);
    void Deallocate(void *ptr);

    bool Contains(void *ptr) const;

    static BuddyChunk *Create(size_t capacityInBytes, BuddyChunk *sibling = nullptr);
    static void Destroy(BuddyChunk *chunk);

protected:
    BuddyChunk(size_t capacityInBytes, BuddyChunk *sibling);
    ~BuddyChunk();

private:
    u32 _capacityInBytes;
    u32 _consumedInBytes;
    u32 _blockCount : 31;
    u32 _available  : 1;

    BuddyBlock *_blocks;
    BuddyChunk *_sibling;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class BuddyHeap {
public:
    BuddyHeap(size_t capacityInBytes, bool growable);
    ~BuddyHeap();

    BuddyHeap(const BuddyHeap& ) = delete;
    BuddyHeap& operator =(const BuddyHeap& ) = delete;

    size_t ChunkSizeInBytes() const { return _chunkSizeInBytes; }
    bool Growable() const { return _growable; }

    size_t CapacityInBytes() const { return _capacityInBytes; }
    size_t ConsumedInBytes() const { return _consumedInBytes; }
    size_t BlockCount() const { return _blockCount; }

    void *Allocate(size_t sizeInBytes);
    void Deallocate(void *ptr);

    void Start();
    void Shutdown();

private:
    const u32 _chunkSizeInBytes : 31;
    const u32 _growable : 1;

    size_t _capacityInBytes;
    size_t _consumedInBytes;
    size_t _blockCount;

    BuddyChunk *_chunks;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ThreadSafeBuddyHeap : BuddyHeap {
public:
    ThreadSafeBuddyHeap(size_t capacityInBytes, bool growable) : BuddyHeap(capacityInBytes, growable) {}
    ~ThreadSafeBuddyHeap() {}

    using BuddyHeap::ChunkSizeInBytes;
    using BuddyHeap::Growable;

    using BuddyHeap::CapacityInBytes;
    using BuddyHeap::ConsumedInBytes;
    using BuddyHeap::BlockCount;

    void *Allocate(size_t sizeInBytes);
    void Deallocate(void *ptr);

    void Start();
    void Shutdown();

private:
    std::mutex _barrier;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
