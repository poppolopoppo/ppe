#include "stdafx.h"

#include "BuddyHeap.h"

#include "Allocation.h"

#ifdef WITH_CORE_ASSERT
#   define WITH_CORE_BUDDYCHUNK_CANARY
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
enum : size_t {
    BuddyBoundary   = 16,

    ChunkFootprint  = BuddyBoundary*2,

    HeaderFootprint = BuddyBoundary,
#ifdef WITH_CORE_BUDDYCHUNK_CANARY
    FooterFootprint = BuddyBoundary,
#else
    FooterFootprint = 0,
#endif

    OverheadPerBlock = HeaderFootprint + FooterFootprint,
    ReservedPerChunk = OverheadPerBlock * 32 /* reserves additional memory for 32 block headers */,
    OverheadPerChunk = ChunkFootprint + ReservedPerChunk,
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct BuddyBlock {
    u32 SizeInBytes : 31;
    u32 InUse       : 1;
    u32 PrevOffset  : 31;
    u32 NextPresent : 1;

    void *Data() const { return reinterpret_cast<void *>(size_t(this) + HeaderFootprint); }

    BuddyBlock *Prev() const {
        return reinterpret_cast<BuddyBlock *>(PrevOffset ? size_t(this) - PrevOffset : 0);
    }

    void SetPrev(BuddyBlock *block) {
        Assert(size_t(block) < size_t(this));
        PrevOffset = block ? size_t(this) - size_t(block) : 0;
    }

    BuddyBlock *Next() const {
        return reinterpret_cast<BuddyBlock *>((size_t(this) + OverheadPerBlock + SizeInBytes) * NextPresent);
    }

    void SetNext(BuddyBlock *block) {
        NextPresent = block ? 1 : 0;
        Assert(Next() == block);
    }
};
STATIC_ASSERT(sizeof(BuddyBlock) == 8);
STATIC_ASSERT(sizeof(BuddyBlock) <= HeaderFootprint);
//----------------------------------------------------------------------------
#ifdef WITH_CORE_BUDDYCHUNK_CANARY
#   define CORE_BUDDYCHUNK_CANARY_MAGICKHEADER  0xABADCAFE
#   define CORE_BUDDYCHUNK_CANARY_MAGICKFOOTER  0xDEADBEEF
#   define CORE_BUDDYCHUNK_CANARY_SET(_BLOCK)   BuddyBlock_SetCanary_(_BLOCK)
#   define CORE_BUDDYCHUNK_CANARY_CHECK(_BLOCK) BuddyBlock_CheckCanary_(_BLOCK)
#else
#   define CORE_BUDDYCHUNK_CANARY_SET(_BLOCK)   NOOP
#   define CORE_BUDDYCHUNK_CANARY_CHECK(_BLOCK) NOOP
#endif
//----------------------------------------------------------------------------
#ifdef WITH_CORE_BUDDYCHUNK_CANARY
static void BuddyBlock_SetCanary_(BuddyBlock *block) {
    STATIC_ASSERT(sizeof(BuddyBlock) == sizeof(u32)*2);

    u32 *const header = reinterpret_cast<u32 *>(block);
    header[2] = CORE_BUDDYCHUNK_CANARY_MAGICKHEADER;
    header[3] = CORE_BUDDYCHUNK_CANARY_MAGICKHEADER;

    u32 *const footer = reinterpret_cast<u32 *>(size_t(block->Data()) + block->SizeInBytes);
    footer[0] = CORE_BUDDYCHUNK_CANARY_MAGICKFOOTER;
    footer[1] = CORE_BUDDYCHUNK_CANARY_MAGICKFOOTER;
    footer[2] = CORE_BUDDYCHUNK_CANARY_MAGICKFOOTER;
    footer[3] = block->SizeInBytes;
}
#endif
//----------------------------------------------------------------------------
#ifdef WITH_CORE_BUDDYCHUNK_CANARY
static void BuddyBlock_CheckCanary_(const BuddyBlock *block) {
    STATIC_ASSERT(sizeof(BuddyBlock) == sizeof(u32)*2);

    const u32 *header = reinterpret_cast<const u32 *>(block);
    Assert(header[2] == CORE_BUDDYCHUNK_CANARY_MAGICKHEADER);
    Assert(header[3] == CORE_BUDDYCHUNK_CANARY_MAGICKHEADER);

    const u32 *footer = reinterpret_cast<const u32 *>(size_t(block->Data()) + block->SizeInBytes);
    Assert(footer[0] == CORE_BUDDYCHUNK_CANARY_MAGICKFOOTER);
    Assert(footer[1] == CORE_BUDDYCHUNK_CANARY_MAGICKFOOTER);
    Assert(footer[2] == CORE_BUDDYCHUNK_CANARY_MAGICKFOOTER);
    Assert(footer[3] == block->SizeInBytes);
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
BuddyChunk::BuddyChunk(size_t capacityInBytes, BuddyChunk *sibling)
:   _capacityInBytes(checked_cast<u32>(capacityInBytes))
,   _consumedInBytes(0)
,   _blockCount(0)
,   _available(true)
,   _sibling(sibling) {
    Assert(IS_ALIGNED(BuddyBoundary, this));
    Assert(capacityInBytes >= (32<<10)); // sanity check
    STATIC_ASSERT(sizeof(*this) <= size_t(ChunkFootprint) );

    _blocks = reinterpret_cast<BuddyBlock *>(size_t(this) + ChunkFootprint);
    _blocks->SizeInBytes = _capacityInBytes + ReservedPerChunk - OverheadPerBlock;
    _blocks->InUse = false;
    _blocks->PrevOffset = 0;
    _blocks->NextPresent = 0;

    CORE_BUDDYCHUNK_CANARY_SET(_blocks);
}
//----------------------------------------------------------------------------
BuddyChunk::~BuddyChunk() {
    Assert(_blocks);
    Assert(!_blocks->InUse);
    Assert(!_blocks->PrevOffset);
    Assert(!_blocks->NextPresent);

    CORE_BUDDYCHUNK_CANARY_CHECK(_blocks);
}
//----------------------------------------------------------------------------
void *BuddyChunk::Allocate(size_t sizeInBytes) {
    if (0 == sizeInBytes || !_available )
        return nullptr;

    STATIC_ASSERT(BuddyBoundary == 16);
    sizeInBytes = ROUND_TO_NEXT_16(sizeInBytes);

    for (BuddyBlock *block = _blocks; block; block = block->Next()) {
        CORE_BUDDYCHUNK_CANARY_CHECK(block);

        if (block->InUse || block->SizeInBytes < sizeInBytes)
            continue;

        if (block->SizeInBytes > sizeInBytes + OverheadPerBlock) {
            BuddyBlock *const nextBlock = block->Next();

            // keeps free block ahead, allows shorter iterations on next call
            BuddyBlock *const reserved = reinterpret_cast<BuddyBlock *>(size_t(block->Data()) + block->SizeInBytes - sizeInBytes - HeaderFootprint);
            reserved->InUse = false;
            reserved->SizeInBytes = sizeInBytes;
            reserved->SetPrev(block);
            reserved->SetNext(nextBlock);
            if (nextBlock)
                nextBlock->SetPrev(reserved);

            block->SizeInBytes = checked_cast<u32>(block->SizeInBytes - (sizeInBytes + OverheadPerBlock) );
            block->SetNext(reserved);

            CORE_BUDDYCHUNK_CANARY_SET(block);

            block = reserved;
        }

        Assert(!block->InUse);
        Assert(block->SizeInBytes >= sizeInBytes);
        Assert(_consumedInBytes + sizeInBytes <= _capacityInBytes + ReservedPerChunk - OverheadPerBlock);

        block->InUse = true;

        _consumedInBytes = checked_cast<u32>(_consumedInBytes + sizeInBytes);
        ++_blockCount;

        CORE_BUDDYCHUNK_CANARY_SET(block);

        void *const ptr = block->Data();

        Assert(IS_ALIGNED(BuddyBoundary, ptr));
        return ptr;
    }

    _available = false; // allows quick skip when all memory is exhausted

    return nullptr;
}
//----------------------------------------------------------------------------
void BuddyChunk::Deallocate(void *ptr) {
    if (!ptr)
        return;

    Assert(size_t(ptr) > HeaderFootprint && IS_ALIGNED(16, ptr));

    BuddyBlock *block = reinterpret_cast<BuddyBlock *>(size_t(ptr) - HeaderFootprint);

    CORE_BUDDYCHUNK_CANARY_CHECK(block);

    Assert(block->InUse);
    Assert(_consumedInBytes >= block->SizeInBytes);

    block->InUse = false;

    _available = true; // new memory available, disable quick skip if necessary
    _consumedInBytes -= block->SizeInBytes;
    _blockCount--;

    BuddyBlock *const prevBlock = block->Prev();
    BuddyBlock *const nextBlock = block->Next();

    if (prevBlock && !prevBlock->InUse) {
        Assert(!prevBlock->Prev() || prevBlock->Prev()->InUse);

        CORE_BUDDYCHUNK_CANARY_CHECK(prevBlock);

        prevBlock->SizeInBytes += block->SizeInBytes + OverheadPerBlock;
        prevBlock->SetNext(nextBlock);
        if (nextBlock)
            nextBlock->SetPrev(prevBlock);

        block = prevBlock;

        CORE_BUDDYCHUNK_CANARY_SET(block);
    }

    if (nextBlock && !nextBlock->InUse) {
        Assert(!nextBlock->Next() || nextBlock->Next()->InUse);

        CORE_BUDDYCHUNK_CANARY_CHECK(nextBlock);

        BuddyBlock *const nextNextBlock = nextBlock->Next();

        block->SizeInBytes += nextBlock->SizeInBytes + OverheadPerBlock;
        block->SetNext(nextNextBlock);
        if (nextNextBlock)
            nextNextBlock->SetPrev(block);

        CORE_BUDDYCHUNK_CANARY_SET(block);
    }
}
//----------------------------------------------------------------------------
bool BuddyChunk::Contains(void *ptr) const {
    return size_t(ptr) >= size_t(_blocks) &&
           size_t(ptr) <= size_t(_blocks) + _capacityInBytes + ReservedPerChunk - OverheadPerBlock;
}
//----------------------------------------------------------------------------
BuddyChunk *BuddyChunk::Create(size_t capacityInBytes, BuddyChunk *sibling /* = nullptr */) {
    Assert(capacityInBytes);

    void *const storage = aligned_malloc(capacityInBytes + OverheadPerChunk, BuddyBoundary);
    AssertRelease(storage);

    return new (storage) BuddyChunk(capacityInBytes, sibling);
}
//----------------------------------------------------------------------------
void BuddyChunk::Destroy(BuddyChunk *chunk) {
    Assert(chunk);
    Assert(0 == chunk->BlockCount());
    Assert(0 == chunk->ConsumedInBytes());

    chunk->~BuddyChunk();

    aligned_free(chunk);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
BuddyHeap::BuddyHeap(size_t capacityInBytes, bool growable)
:   _chunkSizeInBytes(checked_cast<u32>(capacityInBytes))
,   _growable(growable)
,   _capacityInBytes(0)
,   _consumedInBytes(0)
,   _blockCount(0)
,   _chunks(nullptr) {
    Assert(capacityInBytes);
}
//----------------------------------------------------------------------------
BuddyHeap::~BuddyHeap() {
    Assert(!_chunks);
}
//----------------------------------------------------------------------------
void *BuddyHeap::Allocate(size_t sizeInBytes) {
    sizeInBytes = ROUND_TO_NEXT_16(sizeInBytes);
    STATIC_ASSERT(16 == BuddyBoundary);
    AssertRelease(sizeInBytes <= _chunkSizeInBytes);

    Assert(_chunks);
    Assert(_capacityInBytes);

    void *ptr = nullptr;
    for (BuddyChunk *chunk = _chunks; chunk; chunk = chunk->Sibling()) {
        if ( nullptr == (ptr = chunk->Allocate(sizeInBytes)) )
            continue;

        _consumedInBytes += sizeInBytes;
        _blockCount++;

        return ptr;
    }

    if (!_growable)
        return nullptr;

    _chunks = BuddyChunk::Create(_chunkSizeInBytes, _chunks);
    _capacityInBytes += _chunkSizeInBytes;

    ptr = _chunks->Allocate(sizeInBytes);
    Assert(ptr);

    _consumedInBytes += sizeInBytes;
    _blockCount++;

    return ptr;
}
//----------------------------------------------------------------------------
void BuddyHeap::Deallocate(void *ptr) {
    if (!ptr)
        return;

    Assert(IS_ALIGNED(BuddyBoundary, ptr));

    BuddyChunk *prevChunk = nullptr;
    for (BuddyChunk *chunk = _chunks; chunk; prevChunk = chunk, chunk = chunk->Sibling()) {
        if (!chunk->Contains(ptr))
            continue;

        const size_t chunkConsumedInBytes = chunk->ConsumedInBytes();

        chunk->Deallocate(ptr);

        Assert(chunkConsumedInBytes > chunk->ConsumedInBytes());
        _consumedInBytes -= chunkConsumedInBytes - chunk->ConsumedInBytes();
        _blockCount--;

        if (prevChunk && 0 == chunk->BlockCount() &&
            prevChunk->ConsumedInBytes() * 3 < 2 * prevChunk->CapacityInBytes()) {
            prevChunk->SetSibling(chunk->Sibling());
            BuddyChunk::Destroy(chunk);
        }

        return;
    }

    AssertNotReached();
}
//----------------------------------------------------------------------------
void BuddyHeap::Start() {
    Assert(!_chunks);
    Assert(0 == _capacityInBytes);
    Assert(0 == _blockCount);
    Assert(0 == _consumedInBytes);

    _chunks = BuddyChunk::Create(_chunkSizeInBytes + OverheadPerChunk);
    _capacityInBytes += _chunkSizeInBytes;
}
//----------------------------------------------------------------------------
void BuddyHeap::Shutdown() {
    Assert(_chunks);
    Assert(_capacityInBytes);
    Assert(0 == _blockCount);
    Assert(0 == _consumedInBytes);

    for (BuddyChunk *sibling = nullptr; _chunks; _chunks = sibling) {
        sibling = _chunks->Sibling();
        BuddyChunk::Destroy(_chunks);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void *ThreadSafeBuddyHeap::Allocate(size_t sizeInBytes) {
    std::lock_guard<std::mutex> scopeLock(_barrier);

    return BuddyHeap::Allocate(sizeInBytes);
}
//----------------------------------------------------------------------------
void ThreadSafeBuddyHeap::Deallocate(void *ptr) {
    std::lock_guard<std::mutex> scopeLock(_barrier);

    return BuddyHeap::Deallocate(ptr);
}
//----------------------------------------------------------------------------
void ThreadSafeBuddyHeap::Start() {
    std::lock_guard<std::mutex> scopeLock(_barrier);

    BuddyHeap::Start();
}
//----------------------------------------------------------------------------
void ThreadSafeBuddyHeap::Shutdown() {
    std::lock_guard<std::mutex> scopeLock(_barrier);

    BuddyHeap::Shutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
