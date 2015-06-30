#include "stdafx.h"

#include "VirtualFileSystemNativeComponent.h"

#include "VirtualFileSystemNativeStream.h"

#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/FS/FileSystemToken.h"
#include "IO/FS/FileSystemTrie.h"
#include "IO/FileSystem.h"
#include "IO/Stream.h"

// _waccess()
#include <io.h>
#include <wchar.h>

// EnumerateFiles()
#ifdef OS_WINDOWS
#   include <windows.h>
#   include <tchar.h>
#   include <stdio.h>
#endif

#define NATIVE_ENTITYNAME_MAXSIZE 1024

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void SanitizeTarget_(WString& target) {
    static_assert(FileSystem::Separator == L'/', "invalid FileSystem::Separator");
    Assert(target.size());

    const wchar_t *p = target.c_str();
    while (nullptr != (p = StrChr(p, L'\\')) )
        *const_cast<wchar_t *>(p) = L'/';

    if (target.back() != L'/')
        target.insert(target.end(), L'/');
}
//----------------------------------------------------------------------------
static void Unalias_(
    WOCStrStream& oss,
    const Dirpath& aliased,
    const Dirpath& alias, const WString& target) {
    Assert(alias.PathNode());
    Assert(aliased.PathNode());
    Assert(L'/' == target.back());

    oss << target;
    if (aliased.PathNode() == alias.PathNode())
        return;

    const auto subpath = MALLOCA_VIEW(FileSystemToken, Dirpath::MaxDepth);
    const size_t k = FileSystemPath::Instance().Expand(subpath.Pointer(), subpath.size(), alias.PathNode(), aliased.PathNode());

    for (size_t i = 0; i < k; ++i)
        oss << subpath[i] << wchar_t(FileSystem::Separator);
}
//----------------------------------------------------------------------------
static void Unalias_(
    wchar_t *storage, size_t capacity,
    const Dirpath& aliased,
    const Dirpath& alias, const WString& target,
    const wchar_t *suffix = nullptr) {
    WOCStrStream oss(storage, capacity);
    Unalias_(oss, aliased, alias, target);
    if (suffix)
        oss << suffix;
}
//----------------------------------------------------------------------------
static void Unalias_(
    wchar_t *storage, size_t capacity,
    const Filename& aliased,
    const Dirpath& alias, const WString& target) {
    WOCStrStream oss(storage, capacity);
    Unalias_(oss, aliased.Dirpath(), alias, target);
    oss << aliased.Basename();
}
//----------------------------------------------------------------------------
static size_t EnumerateFiles_Windows_(
    const Dirpath& aliased,
    const Dirpath& alias, const WString& target,
    const std::function<void(const Filename&)>& foreach,
    VECTOR_THREAD_LOCAL(FileSystem, Dirpath)& subDirectories,
    bool recursive
    ) {
    WIN32_FIND_DATAW ffd;

    wchar_t nativeGlob[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeGlob, lengthof(nativeGlob), aliased, alias, target, L"*");

    HANDLE hFind = ::FindFirstFileExW(  nativeGlob,
                                        FindExInfoBasic,
                                        &ffd,
                                        FindExSearchNameMatch,
                                        NULL,
                                        FIND_FIRST_EX_LARGE_FETCH);

    if (INVALID_HANDLE_VALUE == hFind) {
        AssertNotReached();
        return 0;
    }

    size_t total = 0;

    do {
        if (FILE_ATTRIBUTE_DIRECTORY & ffd.dwFileAttributes) {
            if (!recursive)
                continue;

            if ( L'.' == ffd.cFileName[0] && (L'\0' == ffd.cFileName[1] ||
                (L'.' == ffd.cFileName[1] && L'\0' == ffd.cFileName[2])) )
                continue;

            subDirectories.emplace_back(aliased, ffd.cFileName);
        }
        else {
            foreach(Filename(aliased, ffd.cFileName));
            ++total;
        }
    } while (::FindNextFileW(hFind, &ffd));

#ifdef WITH_CORE_ASSERT
    DWORD dwError = GetLastError();
    Assert(ERROR_NO_MORE_FILES == dwError);
#endif

    ::FindClose(hFind);
    return total;
}
//----------------------------------------------------------------------------
static size_t EnumerateFiles_(
    const Dirpath& aliased,
    const Dirpath& alias, const WString& destination,
    const std::function<void(const Filename&)>& foreach,
    bool recursive
    ) {
    VECTOR_THREAD_LOCAL(FileSystem, Dirpath) subDirectories;
    if (recursive)
        subDirectories.reserve(32);

    subDirectories.push_back(aliased);

    size_t total = 0;

    do {
        Dirpath dirpath(std::move(subDirectories.back()) );
        subDirectories.pop_back();

#ifdef OS_WINDOWS
        total += EnumerateFiles_Windows_(
            dirpath,
            alias, destination,
            foreach,
            subDirectories, recursive );
#else
        // TODO (12/13) : no uniform support
#   error "TODO"
#endif
    } while (subDirectories.size());

    return total;
}
//----------------------------------------------------------------------------
static bool EntityExists_(const wchar_t *nativeName, ExistPolicy::Mode policy) {
    Assert(nativeName);

    static_assert(0 == ExistPolicy::Exists, "http://msdn.microsoft.com/en-us/library/1w06ktdy.aspx");
    static_assert(2 == ExistPolicy::WriteOnly, "http://msdn.microsoft.com/en-us/library/1w06ktdy.aspx");
    static_assert(4 == ExistPolicy::ReadOnly, "http://msdn.microsoft.com/en-us/library/1w06ktdy.aspx");
    static_assert(6 == ExistPolicy::ReadWrite, "http://msdn.microsoft.com/en-us/library/1w06ktdy.aspx");

    return 0 == ::_waccess(nativeName, policy);
}
//----------------------------------------------------------------------------
static bool TryCreateDirectory_(const wchar_t *nativeDirpath) {
    if (::CreateDirectoryW(nativeDirpath, NULL))
        return true;

    const DWORD dwError = GetLastError();
    Assert(ERROR_ALREADY_EXISTS == dwError);

    return (ERROR_ALREADY_EXISTS == dwError);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
VirtualFileSystemNativeComponent::VirtualFileSystemNativeComponent(const Dirpath& alias, WString&& target, OpenMode mode /* = Mode::ReadWritable */)
:   VirtualFileSystemComponent(alias)
,   _mode(mode), _target(std::move(target)) {
    Assert(!_target.empty());
    SanitizeTarget_(_target);
}
//----------------------------------------------------------------------------
VirtualFileSystemNativeComponent::VirtualFileSystemNativeComponent(const Dirpath& alias, const WString& target, OpenMode mode /* = Mode::ReadWritable */)
:   VirtualFileSystemComponent(alias)
,   _mode(mode), _target(target) {
    Assert(!_target.empty());
    SanitizeTarget_(_target);
}
//----------------------------------------------------------------------------
VirtualFileSystemNativeComponent::~VirtualFileSystemNativeComponent() {}
//----------------------------------------------------------------------------
IVirtualFileSystemComponentReadable* VirtualFileSystemNativeComponent::Readable() {
    return (ModeReadable & _mode) ? this : nullptr;
}
//----------------------------------------------------------------------------
IVirtualFileSystemComponentWritable* VirtualFileSystemNativeComponent::Writable() {
    return (ModeWritable & _mode) ? this : nullptr;
}
//----------------------------------------------------------------------------
IVirtualFileSystemComponentReadWritable* VirtualFileSystemNativeComponent::ReadWritable() {
    return (ModeReadWritable & _mode) ? this : nullptr;
}
//----------------------------------------------------------------------------
WString VirtualFileSystemNativeComponent::Unalias(const Filename& aliased) const {
    wchar_t nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, lengthof(nativeDirpath), aliased, _alias, _target);
    return nativeDirpath;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeComponent::DirectoryExists(const Dirpath& dirpath, ExistPolicy::Mode policy) {
    Assert(ModeReadable & _mode);

    wchar_t nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeDirpath, lengthof(nativeDirpath), dirpath, _alias, _target);

    return EntityExists_(nativeDirpath, policy);
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeComponent::FileExists(const Filename& filename, ExistPolicy::Mode policy) {
    Assert(ModeReadable & _mode);

    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, lengthof(nativeFilename), filename, _alias, _target);

    return EntityExists_(nativeFilename, policy);
}
//----------------------------------------------------------------------------
size_t VirtualFileSystemNativeComponent::EnumerateFiles(const Dirpath& dirpath, bool recursive, const std::function<void(const Filename&)>& foreach) {
    Assert(ModeReadable & _mode);
    return EnumerateFiles_(dirpath, _alias, _target, foreach, recursive);
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIStream> VirtualFileSystemNativeComponent::OpenReadable(const Filename& filename, AccessPolicy::Mode policy) {
    Assert(ModeReadable & _mode);

    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, lengthof(nativeFilename), filename, _alias, _target);
    LOG(Information, L"[VFS] OpenNativeReadable('{0}')", nativeFilename);

    UniquePtr<IVirtualFileSystemIStream> result(new VirtualFileSystemNativeFileIStream(filename, nativeFilename, policy));
    return result;
}
//----------------------------------------------------------------------------
bool VirtualFileSystemNativeComponent::TryCreateDirectory(const Dirpath& dirpath) {
    Assert(ModeWritable & _mode);

    wchar_t nativeDirpath[NATIVE_ENTITYNAME_MAXSIZE];

    bool result = false;

    WOCStrStream oss(nativeDirpath);
    oss << _target;
    nativeDirpath[oss.size()] = L'\0';
    result |= TryCreateDirectory_(nativeDirpath);

    Assert(dirpath.PathNode());

    const auto subpath = MALLOCA_VIEW(FileSystemToken, Dirpath::MaxDepth);
    const size_t k = FileSystemPath::Instance().Expand(subpath.Pointer(), subpath.size(), _alias.PathNode(), dirpath.PathNode());

    for (size_t i = 0; i < k; ++i) {
        oss << subpath[i] << wchar_t(FileSystem::Separator);
        nativeDirpath[oss.size()] = L'\0';
        result |= TryCreateDirectory_(nativeDirpath);
    }

    return result;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemOStream> VirtualFileSystemNativeComponent::OpenWritable(const Filename& filename, AccessPolicy::Mode policy) {
    Assert(ModeWritable & _mode);

    wchar_t nativeFilename[NATIVE_ENTITYNAME_MAXSIZE];
    Unalias_(nativeFilename, lengthof(nativeFilename), filename, _alias, _target);
    LOG(Information, L"[VFS] OpenNativeWritable('{0}')", nativeFilename);

    UniquePtr<IVirtualFileSystemOStream> result(new VirtualFileSystemNativeFileOStream(filename, nativeFilename, policy));
    return result;
}
//----------------------------------------------------------------------------
UniquePtr<IVirtualFileSystemIOStream> VirtualFileSystemNativeComponent::OpenReadWritable(const Filename&/* filename */, AccessPolicy::Mode/* policy */) {
    Assert(ModeReadWritable & _mode);

    // TODO (12/13) : not supported
    AssertNotImplemented();

    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
