#include "stdafx.h"

#include "VirtualFileSystem.h"

#include "FileSystem.h"
#include "Format.h"
#include "StringView.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "Diagnostic/Logger.h"
#include "Diagnostic/CurrentProcess.h"

#include "VFS/VirtualFileSystemComponent.h"
#include "VFS/VirtualFileSystemNativeComponent.h"
#include "VFS/VirtualFileSystemTrie.h"

#include <chrono>

#if defined(OS_WINDOWS)
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
bool FVirtualFileSystem::WriteAll(const FFilename& filename, const TMemoryView<const u8>& storage, AccessPolicy::EMode policy /* = AccessPolicy::None */) {
    const TUniquePtr<IVirtualFileSystemOStream> ostream = Instance().OpenWritable(filename, policy);
    if (ostream) {
        ostream->Write(storage.Pointer(), storage.SizeInBytes());
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FVirtualFileSystemStartup::Start() {
    POOL_TAG(VirtualFileSystem)::Start();
    FVirtualFileSystem::Create();
    auto& VFS = FVirtualFileSystem::Instance();
    // current process directory
    {
        VFS.MountNativePath(L"Process:/", FCurrentProcess::Instance().Directory());
    }
    // data directory
    {
        FWString path;
        Format(path, L"{0}/../../Data", FCurrentProcess::Instance().Directory());
        VFS.MountNativePath(MakeStringView(L"Data:/"), path);
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
#if defined(OS_WINDOWS)
        wchar_t* userPath = nullptr;
        size_t userPathLen = 0;
        if (0 != _wdupenv_s(&userPath, &userPathLen, L"USERPROFILE"))
            AssertNotReached();
#elif defined(OS_LINUX)
        const wchar_t* userPath = nullptr;
        const char* userPathA = std::getenv("HOME");
        FWString userPathW = ToWString(userPath);
        userPath = userPathW.c_str();
#else
#   error "unsupported platform"
#endif
        AssertRelease(userPath);
        VFS.MountNativePath(L"User:/", userPath);
#if defined(OS_WINDOWS)
        free(userPath); // must free() the string allocated by _wdupenv_s()
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
