#pragma once

#include "Core/Core.h"

#include "Core/IO/VirtualFileSystem_fwd.h"
#include "Core/IO/VFS/VirtualFileSystemTrie.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/Singleton.h"
#include "Core/Thread/ReadWriteLock.h"

#include <mutex>

namespace Core {
template <typename T, typename _Allocator>
class TRawStorage;
POOL_TAG_DECL(FVirtualFileSystem);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVirtualFileSystem : Meta::TSingleton<FVirtualFileSystemTrie, FVirtualFileSystem> {
public:
    typedef Meta::TSingleton<FVirtualFileSystemTrie, FVirtualFileSystem> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }

    static FBasename TemporaryBasename(const wchar_t *prefix, const wchar_t *ext);
    static FFilename TemporaryFilename(const wchar_t *prefix, const wchar_t *ext);

    template <typename T, typename _Allocator>
    static bool ReadAll(const FFilename& filename, TRawStorage<T, _Allocator>& storage, AccessPolicy::EMode policy = AccessPolicy::None);
    template <typename T, typename _Allocator>
    static bool WriteAll(const FFilename& filename, const TRawStorage<T, _Allocator>& storage, AccessPolicy::EMode policy = AccessPolicy::None);
    static bool WriteAll(const FFilename& filename, const TMemoryView<const u8>& storage, AccessPolicy::EMode policy = AccessPolicy::None);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVirtualFileSystemStartup {
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
} //!namespace Core

#include "Core/IO/VirtualFileSystem-inl.h"
