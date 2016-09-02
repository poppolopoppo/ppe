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
Basename VirtualFileSystem::TemporaryBasename(const wchar_t *prefix, const wchar_t *ext) {
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
    return Basename(FileSystem::StringView(buffer, length - 1));
}
//----------------------------------------------------------------------------
Filename VirtualFileSystem::TemporaryFilename(const wchar_t *prefix, const wchar_t *ext) {
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
    return Filename(FileSystem::StringView(buffer, length - 1));
}
//----------------------------------------------------------------------------
bool VirtualFileSystem::WriteAll(const Filename& filename, const MemoryView<const u8>& storage, AccessPolicy::Mode policy /* = AccessPolicy::None */) {
    const UniquePtr<IVirtualFileSystemOStream> ostream = Instance().OpenWritable(filename, policy);
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
void VirtualFileSystemStartup::Start() {
    POOL_TAG(VirtualFileSystem)::Start();
    VirtualFileSystem::Create();
    // current process directory
    {
        VirtualFileSystem::Instance().MountNativePath(MakeStringView(L"Process:/"), CurrentProcess::Instance().Directory());
    }
    // data directory
    {
        WString path;
        Format(path, L"{0}/../../Data", CurrentProcess::Instance().Directory());
        VirtualFileSystem::Instance().MountNativePath(MakeStringView(L"Data:/"), path);
    }
    // system temporary path
    {
        FileSystem::char_type tmpPath[1024];
        if (!FileSystem::SystemTemporaryDirectory(tmpPath, lengthof(tmpPath)) )
            AssertNotReached();

        VirtualFileSystem::Instance().MountNativePath(MakeStringView(L"Tmp:/"), tmpPath);
    }
    // user profile path
    {
        const wchar_t* userPath = nullptr;
#if defined(OS_WINDOWS)
        userPath = _wgetenv(L"USERPROFILE");
#elif defined(OS_LINUX)
        const char* userPathA = std::getenv("HOME");
        WString userPathW = ToWString(userPath);
        userPath = userPathW.c_str();
#else
#   error "unsupported platform"
#endif
        AssertRelease(userPath);
        VirtualFileSystem::Instance().MountNativePath(MakeStringView(L"User:/"), userPath);
    }
}
//----------------------------------------------------------------------------
void VirtualFileSystemStartup::Shutdown() {
    VirtualFileSystem::Destroy();
    POOL_TAG(VirtualFileSystem)::Shutdown();
}
//----------------------------------------------------------------------------
void VirtualFileSystemStartup::Clear() {
    VirtualFileSystem::Instance().Clear();
    POOL_TAG(VirtualFileSystem)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
