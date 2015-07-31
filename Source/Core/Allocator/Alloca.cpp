#include "stdafx.h"

#include "Alloca.h"

#include "Memory/MemoryTracking.h"
#include "Thread/ThreadLocalSingleton.h"
#include "ThreadLocalHeap.h"

#ifdef WITH_CORE_ASSERT
#   define WITH_CORE_ALLOCA_CANARY
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
class AllocaStorage {
public:
    enum : u32 {
    // memory reserved per thread for alloca
#if defined(ARCH_X64)
        Capacity        =128<<10    /* 128 kb   */
#elif defined(ARCH_X86)
        Capacity        = 64<<10    /* 64 kb    */
#else
#   error "no support"
#endif

    // each allocation are guaranteed to be aligned on :
    ,   Boundary        = 16        /* 16 b     */

    // each allocation fallback on TLH when bigger than :
    ,   MaxBlockSize    = 16<<10    /* 16 kb    */

    // internal configuration :

    ,   HeaderSize      = Boundary  // keeps blocks aligned on boundary

#ifdef WITH_CORE_ALLOCA_CANARY
    ,   HeaderCanary    = 0xABADCAFE

    ,   FooterCanary    = 0xDEADBABE
    ,   FooterSize      = Boundary // keeps blocks aligned on boundary
#else
    ,   FooterSize      = 0
#endif
    ,   PayloadSize     = HeaderSize + FooterSize
    };
    STATIC_ASSERT(Boundary == 16);
    STATIC_ASSERT(ROUND_TO_NEXT_16(PayloadSize) == PayloadSize);

    AllocaStorage();
    ~AllocaStorage();

    void *Push(size_t sizeInBytes);
    void Pop(void *ptr);

private:
    u32 _offset;
    void *_storage;
};
//----------------------------------------------------------------------------
AllocaStorage::AllocaStorage()
:   _offset(0)
,   _storage(nullptr)
{}
//----------------------------------------------------------------------------
AllocaStorage::~AllocaStorage() {
    Assert(0 == _offset); // Check that all used memory has been released

    if (_storage)
        GetThreadLocalHeap().aligned_free(_storage,
            MEMORY_DOMAIN_TRACKING_DATA(Alloca));
}
//----------------------------------------------------------------------------
void *AllocaStorage::Push(size_t sizeInBytes) {
    Assert(sizeInBytes);

    if (sizeInBytes <= MaxBlockSize &&
        sizeInBytes + _offset <= Capacity) {
        if (!_storage) {
            Assert(0  == _offset);
            _storage = GetThreadLocalHeap().aligned_malloc(
                Capacity,
                Boundary,
                MEMORY_DOMAIN_TRACKING_DATA(Alloca));
            Assert(_storage);
        }

        Assert(ROUND_TO_NEXT_16(_offset) == _offset);

        void *const block = reinterpret_cast<u8 *>(_storage) + _offset;

        const u32 blockSize = checked_cast<u32>(ROUND_TO_NEXT_16(sizeInBytes) + PayloadSize);

        u32 *const header = reinterpret_cast<u32 *>(block);
        void *const userBlock = reinterpret_cast<u8 *>(block) + HeaderSize;

        _offset += (*header = blockSize);

#ifdef WITH_CORE_ALLOCA_CANARY
        header[1] = checked_cast<u32>(sizeInBytes);
        header[2] = HeaderCanary;
        header[3] = HeaderCanary;

        u32 *const footer = reinterpret_cast<u32 *>((u8 *)&header[4] + sizeInBytes);
        footer[0] = FooterCanary;
        footer[1] = FooterCanary;
        footer[2] = FooterCanary;
        footer[3] = header[1];
#endif

        Assert(IS_ALIGNED(Boundary, userBlock));
        return userBlock;
    }
    else {
        // Fallback on thread local heap allocations if :
        // - asked block size is too large (> MaxBlockSize)
        // - no more space available in _storage (should not happen ...)
        return GetThreadLocalHeap().aligned_malloc(
            sizeInBytes,
            Boundary,
            MEMORY_DOMAIN_TRACKING_DATA(Alloca));
    }
}
//----------------------------------------------------------------------------
void AllocaStorage::Pop(void *ptr) {
    Assert(ptr);

    void *const block = reinterpret_cast<u8 *>(ptr) - HeaderSize;

    const u32 *header = reinterpret_cast<const u32 *>(block);
    const u32 blockSize = *header;

    if (size_t(block) + blockSize == size_t(_storage) + _offset) {
        Assert(_storage);

        Assert(blockSize <= _offset);
        Assert(blockSize <= checked_cast<u32>(ROUND_TO_NEXT_16(MaxBlockSize) + PayloadSize));

#ifdef WITH_CORE_ALLOCA_CANARY
        const u32 userSizeInBytes = header[1];
        Assert(header[2] == HeaderCanary);
        Assert(header[3] == HeaderCanary);

        const u32 *footer = reinterpret_cast<const u32 *>((const u8 *)&header[4] + userSizeInBytes);
        Assert(footer[0] == FooterCanary);
        Assert(footer[1] == FooterCanary);
        Assert(footer[2] == FooterCanary);
        Assert(footer[3] == userSizeInBytes);
#endif

#ifdef WITH_CORE_ASSERT
        memset(block, 0xDD, blockSize);
#endif

        _offset -= blockSize;
    }
    else {
        Assert( size_t(block) < size_t(_storage) || // dont delete any other ptr than the last allocated
                size_t(block) > size_t(_storage) + Capacity);

        GetThreadLocalHeap().aligned_free(ptr, MEMORY_DOMAIN_TRACKING_DATA(Alloca));
    }
}
//----------------------------------------------------------------------------
class ThreadLocalAllocaStorage : Meta::ThreadLocalSingleton<AllocaStorage, ThreadLocalAllocaStorage> {
    typedef Meta::ThreadLocalSingleton<AllocaStorage, ThreadLocalAllocaStorage> parent_type;
public:
    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create();
};
//----------------------------------------------------------------------------
void ThreadLocalAllocaStorage::Create() {
    static const char* key = "ThreadLocalAlloca";
    parent_type::Create(key);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void *Alloca(size_t sizeInBytes) {
    if (0 == sizeInBytes)
        return nullptr;

    return ThreadLocalAllocaStorage::Instance().Push(sizeInBytes);
}
//----------------------------------------------------------------------------
void FreeAlloca(void *ptr) {
    if (!ptr)
        return;

    ThreadLocalAllocaStorage::Instance().Pop(ptr);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void AllocaStartup::Start(bool/* mainThread */) {
    ThreadLocalAllocaStorage::Create();
}
//----------------------------------------------------------------------------
void AllocaStartup::Shutdown() {
    ThreadLocalAllocaStorage::Destroy();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
