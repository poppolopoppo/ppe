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
class RawStorage;
POOLTAG_DECL(VirtualFileSystem);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VirtualFileSystem : Meta::Singleton<VirtualFileSystemTrie, VirtualFileSystem> {
public:
    typedef Meta::Singleton<VirtualFileSystemTrie, VirtualFileSystem> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create() { parent_type::Create(); }

    static Filename TemporaryFilename(const wchar_t *prefix, const wchar_t *ext);

    template <typename T, typename _Allocator>
    static bool ReadAll(const Filename& filename, RawStorage<T, _Allocator>& storage, AccessPolicy::Mode policy = AccessPolicy::None);
    template <typename T, typename _Allocator>
    static bool WriteAll(const Filename& filename, const RawStorage<T, _Allocator>& storage, AccessPolicy::Mode policy = AccessPolicy::None);
    static bool WriteAll(const Filename& filename, const MemoryView<const u8>& storage, AccessPolicy::Mode policy = AccessPolicy::None);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VirtualFileSystemStartup {
public:
    static void Start();
    static void Shutdown();

    static void Clear();

    VirtualFileSystemStartup() { Start(); }
    ~VirtualFileSystemStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/VirtualFileSystem-inl.h"
