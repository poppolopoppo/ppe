#pragma once

#include "VirtualFileSystem_fwd.h"
#include "VirtualFileSystemTrie.h"

#include "Allocator/PoolAllocatorTag.h"
#include "Memory/UniquePtr.h"
#include "Meta/Singleton.h"
#include "Thread/ReadWriteLock.h"

#include <mutex>

namespace PPE {
template <typename T, typename _Allocator>
class TRawStorage;
POOL_TAG_DECL(VirtualFileSystem);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_VFS_API FVirtualFileSystem : Meta::TSingleton<FVirtualFileSystemTrie, FVirtualFileSystem> {
public:
    friend class FVirtualFileSystemStartup;
    typedef Meta::TSingleton<FVirtualFileSystemTrie, FVirtualFileSystem> parent_type;

    using parent_type::Get;
#ifdef WITH_PPE_ASSERT
    using parent_type::HasInstance;
#endif

    static FBasename TemporaryBasename(const FWStringView& prefix, const FWStringView& ext);
    static FFilename TemporaryFilename(const FWStringView& prefix, const FWStringView& ext);

    template <typename T, typename _Allocator>
    static bool ReadAll(const FFilename& filename, TRawStorage<T, _Allocator>& storage, EAccessPolicy policy = EAccessPolicy::None);
    template <typename T, typename _Allocator>
    static bool WriteAll(const FFilename& filename, const TRawStorage<T, _Allocator>& storage, EAccessPolicy policy = EAccessPolicy::None);
    static bool WriteAll(const FFilename& filename, const TMemoryView<const u8>& storage, EAccessPolicy policy = EAccessPolicy::None);

    static bool Copy(const FFilename& dst, const FFilename& src, EAccessPolicy policy = EAccessPolicy::None);
    static bool Compress(const FFilename& dst, const FFilename& src, EAccessPolicy policy = EAccessPolicy::None);
    static bool Decompress(const FFilename& dst, const FFilename& src, EAccessPolicy policy = EAccessPolicy::None);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_VFS_API FVirtualFileSystemStartup {
public:
    static void Start();
    static void Shutdown();

    static void Clear();

    FVirtualFileSystemStartup() { Start(); }
    ~FVirtualFileSystemStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "VirtualFileSystem-inl.h"