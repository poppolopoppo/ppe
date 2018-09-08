#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformFile.h"

#ifdef PLATFORM_WINDOWS

#include "Container/RingBuffer.h"
#include "Container/Vector.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "HAL/PlatformLowLevelIO.h"
#include "HAL/PlatformTime.h"
#include "Time/DateTime.h"

#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"

#include <sys/stat.h> // _stat64

#include <initguid.h>
#include <KnownFolders.h> // UserDirectory()
#include <shellapi.h> // SHFileOperation()
#include <Shlwapi.h> // PathIsDirectory()
#include <ShlObj.h> // UserDirectory()
#include <wchar.h> // wcschr()

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
using char_type = FWindowsPlatformFile::char_type;
STATIC_ASSERT(FWindowsPlatformFile::MaxPathLength == MAX_PATH);
//----------------------------------------------------------------------------
static void EnumerateDirNonRecursive_(
    const char_type* path,
    const TFunction<void(const FWStringView&)>& onFile,
    const TFunction<void(const FWStringView&)>& onSubDir) {
    Assert(path);
    Assert(onFile || onSubDir);

    ::WIN32_FIND_DATAW ffd;

    wchar_t tmp[2048];
    Format(tmp, L"{0}\\*", MakeCStringView(path));

    ::HANDLE hFind = ::FindFirstFileExW(
        tmp,
        FindExInfoBasic,
        &ffd,
        FindExSearchNameMatch,
        NULL,
        FIND_FIRST_EX_LARGE_FETCH);

    if (INVALID_HANDLE_VALUE == hFind) {
        LOG_LASTERROR(HAL, L"FindFirstFileExW()");
        return;
    }

    do {
        const auto fname = MakeCStringView(ffd.cFileName);
        if (FILE_ATTRIBUTE_DIRECTORY & ffd.dwFileAttributes) {
            if (fname == L"." || fname == L"..")
                continue;

            if (onSubDir)
                onSubDir(fname);
        }
        else if (onFile) {
            onFile(fname);
        }
    } while (::FindNextFileW(hFind, &ffd));

#ifdef WITH_PPE_ASSERT
    ::DWORD dwError = ::GetLastError();
    Assert(ERROR_NO_MORE_FILES == dwError);
#endif

    Verify(::FindClose(hFind));
}
//----------------------------------------------------------------------------
static void GlobFilesNonRecursive_(
    const char_type* path,
    const char_type* pattern,
    const TFunction<void(const FWStringView)>& onMatch ) {
    Assert(path);
    Assert(pattern);
    Assert(onMatch);

    ::WIN32_FIND_DATAW ffd;

    wchar_t tmp[2048];
    Format(tmp, L"{0}\\{1}\0", MakeCStringView(path), MakeCStringView(pattern));

    ::HANDLE hFind = ::FindFirstFileExW(
        tmp,
        FindExInfoBasic,
        &ffd,
        FindExSearchNameMatch,
        NULL,
        FIND_FIRST_EX_LARGE_FETCH );

    if (INVALID_HANDLE_VALUE == hFind) {
        LOG_LASTERROR(HAL, L"FindFirstFileExW()");
        return;
    }

    size_t numFiles = 0;

    do {
        const auto fname = MakeCStringView(ffd.cFileName);
        if (FILE_ATTRIBUTE_DIRECTORY & ffd.dwFileAttributes) {
            if (2 == fname.size() && L'.' == fname[0] && L'.' == fname[1])
                continue;
        }
        else {
            const size_t len = Format(tmp, L"{0}\\{1}", path, fname);
            onMatch(FWStringView(tmp, len));

            ++numFiles;
        }
    } while (::FindNextFileW(hFind, &ffd));

#ifdef WITH_PPE_ASSERT
    ::DWORD dwError = ::GetLastError();
    Assert(ERROR_NO_MORE_FILES == dwError);
#endif

    Verify(::FindClose(hFind));
}
//----------------------------------------------------------------------------
struct FDirectoryIterator_ {
    FWString FullName;
    FWString Relative;

    FDirectoryIterator_() = default;

    explicit FDirectoryIterator_(const FWStringView& root)
        : FullName(root)
    {}

    FDirectoryIterator_(FWString&& fullname, FWString&& relative)
        : FullName(std::move(fullname))
        , Relative(std::move(relative))
    {}

    FDirectoryIterator_(const FDirectoryIterator_&) = delete;
    FDirectoryIterator_& operator =(const FDirectoryIterator_&) = delete;

    FDirectoryIterator_(FDirectoryIterator_&&) = default;
    FDirectoryIterator_& operator =(FDirectoryIterator_&&) = default;
};
//----------------------------------------------------------------------------
// https://docs.microsoft.com/en-us/windows/desktop/sysinfo/converting-a-time-t-value-to-a-file-time
void TimetToFileTime_(time_t t, ::LPFILETIME pft) {
    ::LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
    pft->dwLowDateTime = (::DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWString FWindowsPlatformFile::SystemDirectory() {
    wchar_t buffer[MAX_PATH + 1];
    const ::DWORD len = ::GetSystemDirectoryW(buffer, lengthof(buffer));
    CLOG_LASTERROR(len == 0, HAL, L"GetSystemDirectoryW()");

    FWString result(buffer, checked_cast<size_t>(len));
    Verify(NormalizePath(result));
    return result;
}
//----------------------------------------------------------------------------
FWString FWindowsPlatformFile::TemporaryDirectory() {
    wchar_t buffer[MAX_PATH + 1];
    const ::DWORD len = ::GetTempPathW(lengthof(buffer), buffer);
    CLOG_LASTERROR(len == 0, HAL, L"GetTempPathW()");

    FWString result(buffer, checked_cast<size_t>(len));
    Verify(NormalizePath(result));
    return result;
}
//----------------------------------------------------------------------------
FWString FWindowsPlatformFile::UserDirectory() {
    ::PWSTR path = NULL;
    auto success = ::SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &path);
    NOOP(success);
    CLOG_LASTERROR(FAILED(success), HAL, L"SHGetKnownFolderPath()");

    if (path) {
        Assert(not FAILED(success));
        FWString result(MakeCStringView(path));
        Verify(NormalizePath(result));
        ::CoTaskMemFree(path);
        return result;
    }
    else {
        Assert(FAILED(success));
        return FWString();
    }
}
//----------------------------------------------------------------------------
FWString FWindowsPlatformFile::WorkingDirectory() {
    wchar_t buffer[MAX_PATH + 1];
    const ::DWORD len = ::GetCurrentDirectoryW(lengthof(buffer), buffer);
    CLOG_LASTERROR(0 == len, HAL, L"GetCurrentDirectoryW()");

    FWString result(buffer, checked_cast<size_t>(len));
    Verify(NormalizePath(result));
    return result;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::IsAllowedChar(char_type ch) {
    return (std::isalnum(ch) ||
        ch == L'_' ||
        ch == L'-' ||
        ch == L':' ||
        ch == L'.' );
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::NormalizePath(FWString& path) {
    Assert(path.size());

    path.gsub(L'/', L'\\');
    while (path.gsub(L"\\\\", L"\\"));

    for (;;) {
        const size_t dotdot = path.find(L"\\..\\");
        if (dotdot == FWString::npos)
            break;

        Assert(path[dotdot] == L'\\');

        const size_t folder = path.rfind(L'\\', dotdot - 1);
        AssertRelease(folder != FWString::npos);
        Assert(path[folder] == L'\\');

        path.erase(path.begin() + folder, path.begin() + (dotdot + 3));
    }

    while (path.size() && path.back() == PathSeparator || path.back() == PathSeparatorAlt)
        path.pop_back();

    path.shrink_to_fit();

    return true;
}
//----------------------------------------------------------------------------
FWString FWindowsPlatformFile::JoinPath(const std::initializer_list<FWStringView>& parts) {
    FWStringBuilder sb;

    auto sep = Fmt::NotFirstTime(PathSeparator);
    for (const auto& it : parts)
        sb << sep << it;

    return sb.ToString();
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::TotalSizeAndUsage(u64* pTotalSize, u64* pUsedSize, const char_type* path) {
    Assert(pTotalSize);
    Assert(pUsedSize);
    Assert(path);

    ::ULARGE_INTEGER freeBytesAvailableToCaller;
    ::ULARGE_INTEGER totalNumberOfBytes;
    ::ULARGE_INTEGER totalNumberOfFreeBytes;
    if (not ::GetDiskFreeSpaceExW(path, &freeBytesAvailableToCaller, &totalNumberOfBytes, &totalNumberOfFreeBytes))
        return false;

    *pTotalSize = checked_cast<u64>(totalNumberOfBytes.QuadPart);
    *pUsedSize = checked_cast<u64>(totalNumberOfFreeBytes.QuadPart);

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::Stat(FStat* pstat, const char_type* filename) {
    Assert(pstat);
    Assert(filename);

    struct ::_stat64 fs;
    if (::_wstat64(filename, &fs))
        return false;

    pstat->UID = checked_cast<u16>(fs.st_uid);
    pstat->GID = checked_cast<u16>(fs.st_gid);
    pstat->Link = checked_cast<u16>(fs.st_nlink);
    pstat->Mode = checked_cast<u16>(fs.st_mode);
    pstat->SizeInBytes = checked_cast<u64>(fs.st_size);
    pstat->CreatedAt.SetValue(fs.st_ctime);
    pstat->LastAccess.SetValue(fs.st_atime);
    pstat->LastModified.SetValue(fs.st_mtime);

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::DirectoryExists(const char_type* dirpath, EExistPolicy policy) {
    Assert(dirpath);

    return (FPlatformLowLevelIO::Access(dirpath, policy) && ::PathIsDirectoryW(dirpath));
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::FileExists(const char_type* filename, EExistPolicy policy) {
    Assert(filename);

    return (FPlatformLowLevelIO::Access(filename, policy) && not ::PathIsDirectoryW(filename));
}
//----------------------------------------------------------------------------
void FWindowsPlatformFile::EnumerateDir(const char_type* dirpath, const TFunction<void(const FWStringView&)>& onFile, const TFunction<void(const FWStringView&)>& onSubDir) {
    Assert(dirpath);
    Assert(onSubDir || onFile);

    EnumerateDirNonRecursive_(dirpath, onFile, onSubDir);
}
//----------------------------------------------------------------------------
void FWindowsPlatformFile::EnumerateFiles(const char_type* dirpath, bool recursive, const TFunction<void(const FWStringView&)>& onFile) {
    Assert(dirpath);
    Assert(onFile);

    if (recursive) {
        char_type buffer[MaxPathLength];

        TFixedSizeRingBuffer<FDirectoryIterator_, 16> stack;
        stack.push_back(MakeCStringView(dirpath));

        FDirectoryIterator_ currDir;
        const TFunction<void(const FWStringView&)> onFile_toFullPath = [&currDir, &buffer, &onFile](const FWStringView& file) {
            const size_t len = Format(buffer, L"{0}\\{1}", currDir.Relative, file);
            onFile(FWStringView(buffer, len));
        };
        const TFunction<void(const FWStringView&)> onSubDir = [&currDir, &stack](const FWStringView& subDir) {
            stack.push_back(
                StringFormat(L"{0}\\{1}", currDir.FullName, subDir),
                StringFormat(L"{0}\\{1}", currDir.Relative, subDir) );
        };

        do {

            Verify(stack.pop_front(&currDir));

            EnumerateDirNonRecursive_(currDir.FullName.c_str(), onFile_toFullPath, onSubDir);

        } while (not stack.empty());
    }
    else {
        EnumerateDirNonRecursive_(dirpath, onFile, TFunction<void(FWStringView)>{});
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformFile::GlobFiles(const char_type* dirpath, const char_type* pattern, bool recursive, const TFunction<void(const FWStringView&)>& onMatch) {
    Assert(dirpath);
    Assert(pattern);
    Assert(onMatch);

    if (recursive) {
        struct FGlobContext_ {
            FWStringView Pattern;
            FDirectoryIterator_ Directory;
            char_type Buffer[MaxPathLength + 1];

        }   context{ MakeCStringView(pattern) };

        TFixedSizeRingBuffer<FDirectoryIterator_, 16> stack;
        stack.push_back(MakeCStringView(dirpath));

        const TFunction<void(const FWStringView&)> onFile = [&context, &onMatch](const FWStringView& file) {
            if (WildMatchI(context.Pattern, file)) {
                const size_t len = Format(context.Buffer, L"{0}\\{1}", context.Directory.Relative, file);
                onMatch(FWStringView(context.Buffer, len));
            }
        };
        const TFunction<void(const FWStringView&)> onSubDir = [&context, &stack](const FWStringView& subDir) {
            stack.push_back(
                StringFormat(L"{0}\\{1}", context.Directory.FullName, subDir),
                StringFormat(L"{0}\\{1}", context.Directory.Relative, subDir));
        };

        do {
            Verify(stack.pop_front(&context.Directory));

            EnumerateDirNonRecursive_(context.Directory.FullName.c_str(), onFile, onSubDir);

        } while (not stack.empty());
    }
    else {
        GlobFilesNonRecursive_(dirpath, pattern, onMatch);
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::CreateDirectory(const char_type* dirpath, bool* existed) {
    Assert(dirpath);

    if (::CreateDirectoryW(dirpath, NULL) == TRUE) {
        if (existed)
            *existed = false;

        LOG(HAL, Info, L"successfully created directory '{0}'", MakeCStringView(dirpath));
        return true;
    }
    else if (::GetLastError() == ERROR_ALREADY_EXISTS) {
        if (existed)
            *existed = true;

        return true;
    }
    else {
        LOG_LASTERROR(HAL, L"CreateDirectory()");
        if (existed)
            *existed = false;

        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::CreateDirectoryRecursively(const char_type* dirpath) {
    Assert(dirpath);

    size_t sz = 0;
    wchar_t folder[MAX_PATH + 1];

    FWStringView slice;
    FWStringView s = MakeCStringView(dirpath);
    while (Split(s, L"\\/", slice)) {
        if (sz)
            folder[sz++] = PathSeparator;

        slice.CopyTo(folder, sz);
        sz += slice.size();

        Assert(sz < lengthof(folder));
        folder[sz] = L'\0';

        if (not FWindowsPlatformFile::CreateDirectory(folder, nullptr))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::MoveFile(const char_type* src, const char_type* dst) {
    Assert(src);
    Assert(dst);

    if (::MoveFileExW(src, dst, MOVEFILE_WRITE_THROUGH | MOVEFILE_COPY_ALLOWED) == TRUE)
        return true;

    LOG_LASTERROR(HAL, L"MoveFileExW()");
    return false;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::RemoveDirectory(const char_type* dirpath, bool force) {
    Assert(dirpath);

    if (force) { // will delete the directory recursively when non empty
        // !! THE STRING MUST BE *DOUBLE-NULL* TERMINATED !!
        // https://docs.microsoft.com/fr-fr/windows/desktop/api/shellapi/ns-shellapi-_shfileopstructa

        const FWStringView org = MakeCStringView(dirpath);
        STACKLOCAL_POD_ARRAY(char_type, doubleNullTerminated, org.size() + 2);
        org.CopyTo(doubleNullTerminated.TrimLastNElements(2));
        doubleNullTerminated[org.size() + 0] = L'\0';
        doubleNullTerminated[org.size() + 1] = L'\0';

        ::SHFILEOPSTRUCTW shOp{
            NULL,
            FO_DELETE,
            doubleNullTerminated.data(),
            NULL,
            FOF_SILENT | FOF_NOERRORUI | FOF_NOCONFIRMATION,
            FALSE,
            NULL,
            NULL
        };

        const int result = ::SHFileOperationW(&shOp);
        if (0 == result)
            return true;

        LOG(HAL, Error, L"SHFileOperationW() failed, last error : {0}", FLastError(result));
        return false;
    }
    else {
        if (::RemoveDirectoryW(dirpath) == TRUE)
            return true;

        LOG_LASTERROR(HAL, L"RemoveDirectoryW()");
        return false;
    }
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::RemoveFile(const char_type* filename) {
    Assert(filename);

    if (::DeleteFileW(filename) == TRUE)
        return true;

    LOG_LASTERROR(HAL, L"DeleteFileW()");
    return false;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::SetFileTime(
    const char_type* filename,
    const FTimestamp* pCreatedAtIFN,
    const FTimestamp* pLastAccessIFN,
    const FTimestamp* pLastModifiedIFN ) {
    Assert(filename);
    Assert(pCreatedAtIFN || pLastAccessIFN || pLastModifiedIFN); // should have a side effect

    const ::HANDLE hFile = ::CreateFileW(filename,
        FILE_WRITE_ATTRIBUTES,    // open only for setting attributes
        0,                        // do not share
        NULL,                     // no security
        OPEN_EXISTING,            // existing file only
        FILE_ATTRIBUTE_NORMAL,    // normal file
        NULL );                   // no attr. template

    if (INVALID_HANDLE_VALUE == hFile) {
        LOG_LASTERROR(HAL, L"CreateFileW()");
        return false;
    }

    ::FILETIME createdAtFT;
    ::FILETIME lastAccessFT;
    ::FILETIME lastModifiedFT;

    if (pCreatedAtIFN)
        TimetToFileTime_(pCreatedAtIFN->Value(), &createdAtFT);
    else
        ::ZeroMemory(&createdAtFT, sizeof(::FILETIME));

    if (pLastAccessIFN)
        TimetToFileTime_(pLastAccessIFN->Value(), &lastAccessFT);
    else
        ::ZeroMemory(&lastAccessFT, sizeof(::FILETIME));

    if (pLastModifiedIFN)
        TimetToFileTime_(pLastModifiedIFN->Value(), &lastModifiedFT);
    else
        ::ZeroMemory(&lastModifiedFT, sizeof(::FILETIME));

    const ::BOOL succeed = ::SetFileTime(hFile, &createdAtFT, &lastAccessFT, &lastModifiedFT);
    CLOG_LASTERROR(!succeed, HAL, L"SetFileTime()");

    Verify(::CloseHandle(hFile));

    return (succeed != 0);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformFile::RollFile(const char_type* filename) {
    Assert(filename);

    FStat fstat;
    if (Stat(&fstat, filename)) {
        const FDateTime date = fstat.LastModified.ToDateTimeUTC();

        char_type buffer[MaxPathLength + 1];
        FWFixedSizeTextWriter oss(buffer);
        Format(oss, L"_{0:#4}-{1:#2}-{2:#2}_{3:#2}-{4:#2}-{5:#2}_UTC",
            date.Year, date.Month, date.Day,
            date.Hours, date.Minutes, date.Seconds );

        FWString logroll{ MakeCStringView(filename) };
        const size_t ext_pos = logroll.find_last_of(L'.');
        logroll.insert(ext_pos, oss.Written());

        while (FileExists(logroll.c_str(), EExistPolicy::Exists)) {
            oss << L"_bak";
            FWString logroll_bak{ MakeCStringView(filename) };
            logroll_bak.insert(ext_pos, oss.Written());

            LOG(HAL, Warning, L"file roll failed because '{0}' already exists, trying {1}",
                logroll, logroll_bak );

            logroll = std::move(logroll_bak);
        }

        LOG(HAL, Info, L"roll file '{0}' -> '{1}'", MakeCStringView(filename), logroll);

        if (not MoveFile(filename, logroll.c_str()))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
FWString FWindowsPlatformFile::MakeTemporaryFile(const char_type* prefix, const char_type* extname) {
    Assert(prefix);
    Assert(extname);
    Assert(L'.' == *extname);

    u32 year, mon, mday, day, hour, min, sec, msec;
    FPlatformTime::UtcTime(year, mon, mday, day, hour, min, sec, msec);

    char_type buffer[MaxPathLength + 1];
    FWFixedSizeTextWriter oss(buffer);
    oss << TemporaryDirectory() << PathSeparator;
    Format(oss, L"{0}_{1:#4}{2:#2}{3:#2}{4:#2}{5:#2}{6:#2}_{7}",
        prefix, year, mon, mday, hour, min, sec, msec );
    oss << extname;

    return FWString(oss.Written());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
