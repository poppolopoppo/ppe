#include "stdafx.h"

#include "VirtualFileSystem.h"

#include "FileSystem.h"
#include "Format.h"
#include "StringView.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "Container/RawStorage.h"
#include "Diagnostic/Logger.h"
#include "Diagnostic/CurrentProcess.h"

#include "VFS/VirtualFileSystemComponent.h"
#include "VFS/VirtualFileSystemNativeComponent.h"
#include "VFS/VirtualFileSystemTrie.h"

#include <chrono>

#if defined(PLATFORM_WINDOWS)
#   include <stdlib.h>
#   include <wchar.h>
#endif

namespace Core {
POOL_TAG_DEF(VirtualFileSystem);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBasename FVirtualFileSystem::TemporaryBasename(const wchar_t *prefix, const wchar_t *ext) {
    Assert(prefix);
    Assert(ext);

    const auto now = std::chrono::system_clock::now();
    const ::time_t tt = std::chrono::system_clock::to_time_t(now);

    ::tm local_tm;
    ::localtime_s(&local_tm, &tt);

    wchar_t buffer[2048];
    const size_t length = Format(buffer, L"{0}_{1:#4}{2:#2}{3:#2}{4:#2}{5:#2}{6:#2}{7}{8}",
        prefix,
        local_tm.tm_year,
        local_tm.tm_mon,
        local_tm.tm_mday,
        local_tm.tm_hour,
        local_tm.tm_min,
        local_tm.tm_sec,
        tt,
        ext );

    Assert(length > 0);
    return FBasename(FileSystem::FStringView(buffer, length - 1));
}
//----------------------------------------------------------------------------
FFilename FVirtualFileSystem::TemporaryFilename(const wchar_t *prefix, const wchar_t *ext) {
    Assert(prefix);
    Assert(ext);

    const auto now = std::chrono::system_clock::now();
    const ::time_t tt = std::chrono::system_clock::to_time_t(now);

    ::tm local_tm;
    ::localtime_s(&local_tm, &tt);

    wchar_t buffer[2048];
    const size_t length = Format(buffer, L"Tmp:/{0}_{1:#4}{2:#2}{3:#2}{4:#2}{5:#2}{6:#2}{7}{8}",
        prefix,
        local_tm.tm_year,
        local_tm.tm_mon,
        local_tm.tm_mday,
        local_tm.tm_hour,
        local_tm.tm_min,
        local_tm.tm_sec,
        tt,
        ext );

    Assert(length > 0);
    return FFilename(FileSystem::FStringView(buffer, length - 1));
}
//----------------------------------------------------------------------------
bool FVirtualFileSystem::WriteAll(const FFilename& filename, const TMemoryView<const u8>& storage, EAccessPolicy policy /* = EAccessPolicy::None */) {
    Assert(!filename.empty());

    const TUniquePtr<IVirtualFileSystemOStream> ostream = Instance().OpenWritable(filename, (policy ^ EAccessPolicy::Compress
        ? policy - EAccessPolicy::Compress + EAccessPolicy::Binary
        : policy ) + EAccessPolicy::Create );

    if (ostream) {
        if (policy ^ EAccessPolicy::Compress) {
            RAWSTORAGE_THREAD_LOCAL(Compress, u8) compressed;
            const size_t compressedSizeInBytes = Compression::CompressMemory(compressed, storage, Compression::HighCompression);
            ostream->Write(compressed.Pointer(), compressedSizeInBytes);
        }
        else {
            ostream->Write(storage.Pointer(), storage.SizeInBytes());
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

    const TUniquePtr<IVirtualFileSystemIStream> istream = Instance().OpenReadable(src, policy - EAccessPolicy::Truncate);
    if (not istream)
        return false;

    const TUniquePtr<IVirtualFileSystemOStream> ostream = Instance().OpenWritable(dst, policy + EAccessPolicy::Truncate);
    if (not ostream)
        return false;

    const TUniqueArray<u8> buffer = NewArray<u8>(HUGE_PAGE_SIZE);
    while (size_t read = istream->ReadSome(buffer.data(), 1, buffer.SizeInBytes())) {
        if (not ostream->Write(buffer.data(), read)) {
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
    RAWSTORAGE_THREAD_LOCAL(Compress, u8) data;
    {
        const TUniquePtr<IVirtualFileSystemIStream> istream = Instance().OpenReadable(src, policy - EAccessPolicy::Truncate);
        if (not istream)
            return false;

        istream->ReadAll(data);
    }
    {
        RAWSTORAGE_THREAD_LOCAL(Compress, u8) compressed;
        compressedSizeInBytes = Compression::CompressMemory(compressed, data.MakeConstView().Cast<const u8>());

        swap(data, compressed);
    }
    {
        const TUniquePtr<IVirtualFileSystemOStream> ostream = Instance().OpenWritable(dst, policy + EAccessPolicy::Truncate + EAccessPolicy::Binary);
        if (not ostream)
            return false;

        if (not ostream->Write(data.Pointer(), compressedSizeInBytes))
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

    RAWSTORAGE_THREAD_LOCAL(Compress, u8) data;
    {
        const TUniquePtr<IVirtualFileSystemIStream> istream = Instance().OpenReadable(src, policy - EAccessPolicy::Truncate + EAccessPolicy::Binary);
        if (not istream)
            return false;

        istream->ReadAll(data);
    }
    {
        RAWSTORAGE_THREAD_LOCAL(Compress, u8) uncompressed;
        if (not Compression::DecompressMemory(uncompressed, data.MakeConstView().Cast<const u8>()))
            return false;

        swap(data, uncompressed);
    }
    {
        const TUniquePtr<IVirtualFileSystemOStream> ostream = Instance().OpenWritable(dst, policy + EAccessPolicy::Truncate);
        if (not ostream)
            return false;

        if (not ostream->Write(data.Pointer(), data.SizeInBytes()))
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
    auto& VFS = FVirtualFileSystem::Instance();
    // current process executable directory
    {
        VFS.MountNativePath(L"Process:/", FCurrentProcess::Instance().Directory());
    }
    // current process working directory
    {
        FileSystem::char_type workingPath[1024];
        if (!FileSystem::WorkingDirectory(workingPath, lengthof(workingPath)))
            AssertNotReached();

        VFS.MountNativePath(L"Working:/", workingPath);
    }
    // data directory
    {
        FWString path;
        Format(path, L"{0}/../../Data", FCurrentProcess::Instance().Directory());
        VFS.MountNativePath(MakeStringView(L"Data:/"), path);
    }
    // saved directory
    {
        FWString path;
        Format(path, L"{0}/../../Output/Saved", FCurrentProcess::Instance().Directory());
        VFS.MountNativePath(MakeStringView(L"Saved:/"), path);
    }
    // system temporary path
    {
        FileSystem::char_type tmpPath[1024];
        if (!FileSystem::SystemTemporaryDirectory(tmpPath, lengthof(tmpPath)) )
            AssertNotReached();

        VFS.MountNativePath(L"Tmp:/", tmpPath);
    }
    // user profile path
    {
#if defined(PLATFORM_WINDOWS)
        wchar_t* userPath = nullptr;
        size_t userPathLen = 0;
        if (0 != _wdupenv_s(&userPath, &userPathLen, L"USERPROFILE"))
            AssertNotReached();
#elif defined(PLATFORM_LINUX)
        const wchar_t* userPath = nullptr;
        const char* userPathA = std::getenv("HOME");
        FWString userPathW = ToWString(userPath);
        userPath = userPathW.c_str();
#else
#   error "unsupported platform"
#endif
        AssertRelease(userPath);
        VFS.MountNativePath(L"User:/", userPath);
#if defined(PLATFORM_WINDOWS)
        ::free(userPath); // must free() the string allocated by _wdupenv_s()
#endif
    }
}
//----------------------------------------------------------------------------
void FVirtualFileSystemStartup::Shutdown() {
    FVirtualFileSystem::Destroy();
    POOL_TAG(VirtualFileSystem)::Shutdown();
}
//----------------------------------------------------------------------------
void FVirtualFileSystemStartup::Clear() {
    FVirtualFileSystem::Instance().Clear();
    POOL_TAG(VirtualFileSystem)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
