#pragma once

#include "Core/Core.h"

#include <thread>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBuddyBlock;
class FBuddyChunk {
public:
    FBuddyChunk(const FBuddyChunk& ) = delete;
    FBuddyChunk& operator =(const FBuddyChunk& ) = delete;

    size_t CapacityInBytes() const { return _capacityInBytes; }
    size_t ConsumedInBytes() const { return _consumedInBytes; }
    size_t BlockCount() const { return _blockCount; }

    FBuddyChunk *Sibling() const { return _sibling; }
    void SetSibling(FBuddyChunk *chunk) { _sibling = chunk; }

    void *Allocate(size_t sizeInBytes);
    void Deallocate(void *ptr);

    bool Contains(void *ptr) const;

    static FBuddyChunk *Create(size_t capacityInBytes, FBuddyChunk *sibling = nullptr);
    static void Destroy(FBuddyChunk *chunk);

protected:
    FBuddyChunk(size_t capacityInBytes, FBuddyChunk *sibling);
    ~FBuddyChunk();

private:
    u32 _capacityInBytes;
    u32 _consumedInBytes;
    u32 _blockCount : 31;
    u32 _available  : 1;

    FBuddyBlock *_blocks;
    FBuddyChunk *_sibling;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBuddyHeap {
public:
    FBuddyHeap(size_t capacityInBytes, bool growable);
    ~FBuddyHeap();

    FBuddyHeap(const FBuddyHeap& ) = delete;
    FBuddyHeap& operator =(const FBuddyHeap& ) = delete;

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

    FBuddyChunk *_chunks;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FThreadSafeBuddyHeap : FBuddyHeap {
public:
    FThreadSafeBuddyHeap(size_t capacityInBytes, bool growable) : FBuddyHeap(capacityInBytes, growable) {}
    ~FThreadSafeBuddyHeap() {}

    using FBuddyHeap::ChunkSizeInBytes;
    using FBuddyHeap::Growable;

    using FBuddyHeap::CapacityInBytes;
    using FBuddyHeap::ConsumedInBytes;
    using FBuddyHeap::BlockCount;

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
