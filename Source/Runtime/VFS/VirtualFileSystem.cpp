#include "stdafx.h"

#include "VirtualFileSystem.h"

#include "FileSystem.h"
#include "Format.h"
#include "StringView.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "Container/RawStorage.h"
#include "Diagnostic/Logger.h"
#include "Diagnostic/CurrentProcess.h"
#include "IO/StringBuilder.h"
#include "HAL/PlatformTime.h"

#include "VFS/VirtualFileSystemComponent.h"
#include "VFS/VirtualFileSystemNativeComponent.h"
#include "VFS/VirtualFileSystemTrie.h"

namespace Core {
POOL_TAG_DEF(VirtualFileSystem);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBasename FVirtualFileSystem::TemporaryBasename(const FWStringView& prefix, const FWStringView& ext) {
    Assert(not prefix.empty());
    Assert(not ext.empty());

    u32 year, mon, mday, day, hour, min, sec, msec;
    FPlatformTime::UtcTime(year, mon, mday, day, hour, min, sec, msec);

    STACKLOCAL_WTEXTWRITER(buffer, FileSystem::MaxPathLength);
    Format(buffer, L"{0}_{1:#4}{2:#2}{3:#2}{4:#2}{5:#2}{6:#2}_{7}{8}",
        prefix,
        year, mon, mday, hour, min, sec, msec,
        ext );

    return FBasename(buffer.Written());
}
//----------------------------------------------------------------------------
FFilename FVirtualFileSystem::TemporaryFilename(const FWStringView& prefix, const FWStringView& ext) {
    Assert(not prefix.empty());
    Assert(not ext.empty());

    u32 year, mon, mday, day, hour, min, sec, msec;
    FPlatformTime::UtcTime(year, mon, mday, day, hour, min, sec, msec);

    STACKLOCAL_WTEXTWRITER(buffer, FileSystem::MaxPathLength);
    Format(buffer, L"Tmp:/{0}_{1:#4}{2:#2}{3:#2}{4:#2}{5:#2}{6:#2}_{7}{8}",
        prefix,
        year, mon, mday, hour, min, sec, msec,
        ext );

    return FFilename(buffer.Written());
}
//----------------------------------------------------------------------------
bool FVirtualFileSystem::WriteAll(const FFilename& filename, const TMemoryView<const u8>& storage, EAccessPolicy policy /* = EAccessPolicy::None */) {
    Assert(!filename.empty());

    bool needCompress = false;
    if (policy ^ EAccessPolicy::Compress) {
        needCompress = true;
        policy = policy - EAccessPolicy::Compress + EAccessPolicy::Binary;
    }

    policy = policy + EAccessPolicy::Sequential; // we're going to make only 1 write, full sequential

    const UStreamWriter writer = Get().OpenWritable(filename, policy);

    if (writer) {
        if (needCompress) {
            RAWSTORAGE(Compress, u8) compressed;
            const size_t compressedSizeInBytes = Compression::CompressMemory(compressed, storage, Compression::HighCompression);
            writer->Write(compressed.Pointer(), compressedSizeInBytes);
        }
        else {
            writer->Write(storage.Pointer(), storage.SizeInBytes());
        }

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FVirtualFileSystem::Copy(const FFilename& dst, const FFilename& src, EAccessPolicy policy/* = EAccessPolicy::None */) {
    Assert(dst != src);
    Assert(not (policy ^ EAccessPolicy::Compress)); // use Compress() method instead

    if (src == dst)
        return true;

    const UStreamReader reader = Get().OpenReadable(src, policy - EAccessPolicy::Truncate);
    if (not reader)
        return false;

    const UStreamWriter writer = Get().OpenWritable(dst, policy + EAccessPolicy::Truncate);
    if (not writer)
        return false;

    STACKLOCAL_POD_ARRAY(u8, buffer, ALLOCATION_GRANULARITY);
    while (size_t read = reader->ReadSome(buffer.data(), 1, buffer.SizeInBytes())) {
        if (not writer->Write(buffer.data(), read)) {
            AssertNotReached();
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystem::Compress(const FFilename& dst, const FFilename& src, EAccessPolicy policy/* = EAccessPolicy::None */) {
    Assert(dst != src);
    Assert(not (policy ^ EAccessPolicy::Compress)); // don't specify it, implicit within this method

    if (src == dst)
        return true;

    size_t compressedSizeInBytes = 0;
    RAWSTORAGE(Compress, u8) data;
    {
        const UStreamReader reader = Get().OpenReadable(src, policy - EAccessPolicy::Truncate);
        if (not reader)
            return false;

        reader->ReadAll(data);
    }
    {
        RAWSTORAGE(Compress, u8) compressed;
        compressedSizeInBytes = Compression::CompressMemory(compressed, data.MakeConstView().Cast<const u8>());

        swap(data, compressed);
    }
    {
        const UStreamWriter writer = Get().OpenWritable(dst, policy + EAccessPolicy::Truncate + EAccessPolicy::Binary);
        if (not writer)
            return false;

        if (not writer->Write(data.Pointer(), compressedSizeInBytes))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FVirtualFileSystem::Decompress(const FFilename& dst, const FFilename& src, EAccessPolicy policy/* = EAccessPolicy::None */) {
    Assert(dst != src);
    Assert(not (policy ^ EAccessPolicy::Compress)); // don't specify it, implicit within this method

    if (src == dst)
        return true;

    RAWSTORAGE(Compress, u8) data;
    {
        const UStreamReader reader = Get().OpenReadable(src, policy - EAccessPolicy::Truncate + EAccessPolicy::Binary);
        if (not reader)
            return false;

        reader->ReadAll(data);
    }
    {
        RAWSTORAGE(Compress, u8) uncompressed;
        if (not Compression::DecompressMemory(uncompressed, data.MakeConstView().Cast<const u8>()))
            return false;

        swap(data, uncompressed);
    }
    {
        const UStreamWriter writer = Get().OpenWritable(dst, policy + EAccessPolicy::Truncate);
        if (not writer)
            return false;

        if (not writer->Write(data.Pointer(), data.SizeInBytes()))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FVirtualFileSystemStartup::Start() {
    POOL_TAG(VirtualFileSystem)::Start();
    FVirtualFileSystem::Create();
    auto& VFS = FVirtualFileSystem::Get();
    // current process executable directory
    {
        VFS.MountNativePath(L"Process:/", FCurrentProcess::Get().Directory());
    }
    // current process working directory
    {
        VFS.MountNativePath(L"Working:/", FPlatformFile::WorkingDirectory());
    }
    // data directory
    {
        FWString path;
        Format(path, L"{0}/../../Data", FCurrentProcess::Get().Directory());
        VFS.MountNativePath(MakeStringView(L"Data:/"), std::move(path));
    }
    // saved directory
    {
        FWString path;
        Format(path, L"{0}/../../Output/Saved", FCurrentProcess::Get().Directory());
        VFS.MountNativePath(MakeStringView(L"Saved:/"), std::move(path));
    }
    // system temporary path
    {
        VFS.MountNativePath(L"Tmp:/", FPlatformFile::TemporaryDirectory());
    }
    // user profile path
    {
        VFS.MountNativePath(L"User:/", FPlatformFile::UserDirectory());
    }
}
//----------------------------------------------------------------------------
void FVirtualFileSystemStartup::Shutdown() {
    FVirtualFileSystem::Destroy();
    POOL_TAG(VirtualFileSystem)::Shutdown();
}
//----------------------------------------------------------------------------
void FVirtualFileSystemStartup::Clear() {
    FVirtualFileSystem::Get().Clear();
    POOL_TAG(VirtualFileSystem)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
