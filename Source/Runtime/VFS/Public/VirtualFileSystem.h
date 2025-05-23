#pragma once

#include "VirtualFileSystem_fwd.h"
#include "VirtualFileSystemTrie.h"

#include "Memory/SharedBuffer_fwd.h"
#include "Meta/Singleton.h"

namespace PPE {
template <typename T, typename _Allocator>
class TRawStorage;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_VFS_API FVirtualFileSystem : Meta::TSingleton<FVirtualFileSystemTrie, FVirtualFileSystem> {
    friend class Meta::TSingleton<FVirtualFileSystemTrie, FVirtualFileSystem>;
    using singleton_type = Meta::TSingleton<FVirtualFileSystemTrie, FVirtualFileSystem>;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT;

public:
    using singleton_type::Get;
    using singleton_type::Create;
    using singleton_type::Destroy;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif

    static FBasename TemporaryBasename(const FWStringView& prefix, const FWStringView& ext);
    static FFilename TemporaryFilename(const FWStringView& prefix, const FWStringView& ext);

    static FUniqueBuffer ReadAll(const FFilename& filename, EAccessPolicy policy = EAccessPolicy::None);
    static bool WriteAll(const FFilename& filename, const TMemoryView<const u8>& storage, EAccessPolicy policy = EAccessPolicy::None);

    static bool Copy(const FFilename& dst, const FFilename& src, EAccessPolicy policy = EAccessPolicy::None);
    static bool Compress(const FFilename& dst, const FFilename& src, EAccessPolicy policy = EAccessPolicy::None);
    static bool Decompress(const FFilename& dst, const FFilename& src, EAccessPolicy policy = EAccessPolicy::None);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
