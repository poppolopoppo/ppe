// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Linux/LinuxPlatformFile.h"

#ifdef PLATFORM_LINUX

#include "Container/RingBuffer.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StaticString.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "HAL/PlatformLowLevelIO.h"
#include "HAL/PlatformTime.h"
#include "Misc/Function.h"
#include "Time/DateTime.h"

#include "HAL/TargetPlatform.h"
#include "HAL/Linux/Errno.h"
#include "HAL/Linux/LinuxPlatformIncludes.h"

#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/types.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
CONSTEXPR const int GDirectoryMode = 0755;
//----------------------------------------------------------------------------
using char_type = FLinuxPlatformFile::char_type;
STATIC_ASSERT(FLinuxPlatformFile::MaxPathLength == PATH_MAX);
//----------------------------------------------------------------------------
template <typename _OnSubFile, typename _OnSubDir>
static bool EnumerateDirNonRecursive_(
    const char_type* path,
    const _OnSubFile& onFile,
    const _OnSubDir& onSubDir) {
    Assert(path);

    if (::DIR* d = ::opendir(WCHAR_TO_UTF_8<FLinuxPlatformFile::MaxPathLength>((path))) ) {
        ONLY_IF_ASSERT(errno = 0);

        struct ::dirent* nt;
        while (!!(nt = ::readdir(d))) {

            if (nt->d_type == DT_DIR) {
                if (0 == strcmp(nt->d_name, ".") ||
                    0 == strcmp(nt->d_name, "..") )
                continue;

                onSubDir(UTF_8_TO_WCHAR<>(nt->d_name));
            }
            else {
                IF_CONSTEXPR(std::is_same_v<void, decltype(onFile(FWStringView{}))>) {
                    onFile(UTF_8_TO_WCHAR<>(nt->d_name));
                }
                else {
                    if (not onFile(UTF_8_TO_WCHAR<>(nt->d_name)))
                        return false;
                }
            }
        }
        const int thisErrno = errno;
        if (thisErrno)
            PPE_LOG(HAL, Error, "readdir() failed with errno: {0}", FErrno{});

        Verify(0  == ::closedir(d));

        if (thisErrno)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
template <typename _OnMatch>
static void GlobFilesNonRecursive_(
    const char_type* path,
    const char_type* pattern,
    const _OnMatch& onMatch ) {
    Assert(path);
    Assert(pattern);
    Assert(onMatch);

    EnumerateDirNonRecursive_(path,
        [pattern{ MakeCStringView(pattern) }, &onMatch](const FWStringView& fname) {
            if (WildMatch(pattern, fname))
                onMatch(fname);
        },
        [](const FWStringView&) {}/* ignores directories */);
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
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWString FLinuxPlatformFile::SystemDirectory() {
    return L"/";
}
//----------------------------------------------------------------------------
FWString FLinuxPlatformFile::TemporaryDirectory() {
    if (const char* tmpDir = ::getenv("TMPDIR"))
        return ToWString(MakeCStringView(tmpDir));
    else
        return L"/tmp";
}
//----------------------------------------------------------------------------
FWString FLinuxPlatformFile::UserDirectory() {
    if (const char* homeDir = ::getenv("HOME"))
        return ToWString(MakeCStringView(homeDir));
    else
        return L"~";
}
//----------------------------------------------------------------------------
FWString FLinuxPlatformFile::WorkingDirectory() {
    char buf[MaxPathLength + 1];
    char* cwd = ::getcwd(buf, MaxPathLength);
    if (cwd) {
        return ToWString(MakeCStringView(cwd));
    }
    else {
        PPE_LOG(HAL, Error, "getcwd() failed with errno: {0}", FErrno{});
        return FWString{};
    }
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::IsAllowedChar(char_type ch) {
    return (std::isalnum(ch) ||
        ch == L':' || // for virtual mounting-points
        ch == L'_' ||
        ch == L'-' ||
        ch == L'.' );
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::NormalizePath(FWString& path) {
    Assert(path.size());

    while (path.gsub(L"//", L"/"));

    for (;;) {
        const size_t dotdot = path.find(L"/../");
        if (dotdot == FWString::npos)
            break;

        Assert(path[dotdot] == L'/');

        const size_t folder = path.rfind(L'/', dotdot - 1);
        AssertRelease(folder != FWString::npos);
        Assert(path[folder] == L'/');

        path.erase(path.begin() + folder, path.begin() + (dotdot + 3));
    }

    while (path.size() && path.back() == PathSeparator)
        path.pop_back();

    path.shrink_to_fit();

    return true;
}
//----------------------------------------------------------------------------
FWString FLinuxPlatformFile::JoinPath(const std::initializer_list<FWStringView>& parts) {
    FWStringBuilder sb;

    auto sep = Fmt::NotFirstTime(PathSeparator);
    for (const auto& it : parts)
        sb << sep << it;

    return sb.ToString();
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::TotalSizeAndUsage(u64* pTotalSize, u64* pUsedSize, const char_type* path) {
    Assert(pTotalSize);
    Assert(pUsedSize);
    Assert(path);

    struct ::statvfs st;
    if (::statvfs(WCHAR_TO_UTF_8<MaxPathLength>(path), &st) == 0) {
        const u64 available = st.f_bavail * st.f_bsize;
        const u64 total = st.f_blocks * st.f_bsize;
        Assert(available <= total);
        *pTotalSize = total;
        *pUsedSize = total - available;
        return true;
    }
    else {
        PPE_LOG(HAL, Error, "statvfs({0}) failed with errno: {1}",
            MakeCStringView(path), FErrno{} );
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::Stat(FStat* pstat, const char_type* filename) {
    Assert(pstat);
    Assert(filename);

    struct ::stat fs;
    if (::stat(WCHAR_TO_UTF_8<MaxPathLength>(filename), &fs))
        return false;

    pstat->UID = checked_cast<u16>(fs.st_uid);
    pstat->GID = checked_cast<u16>(fs.st_gid);
    pstat->SizeInBytes = checked_cast<u64>(fs.st_size);
    pstat->CreatedAt.SetValue(fs.st_ctime);
    pstat->LastAccess.SetValue(fs.st_atime);
    pstat->LastModified.SetValue(fs.st_mtime);

    pstat->Mode = Zero;
    if (fs.st_mode & S_IFCHR   ) pstat->Mode += FStat::Device;
    if (fs.st_mode & S_IFDIR   ) pstat->Mode += FStat::Directory;
    if (fs.st_mode & S_IFREG   ) pstat->Mode += FStat::RegularFile;
    if (fs.st_mode & S_IREAD   ) pstat->Mode += FStat::Read;
    if (fs.st_mode & S_IWRITE  ) pstat->Mode += FStat::Write;
    if (fs.st_mode & S_IEXEC   ) pstat->Mode += FStat::Execute;

    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::DirectoryExists(const char_type* dirpath, EExistPolicy policy) {
    Assert(dirpath);

    if (not FPlatformLowLevelIO::Access(dirpath, policy))
        return false;

    struct ::stat fs;
    return (::stat(WCHAR_TO_UTF_8<MaxPathLength>(dirpath), &fs) == 0 && S_ISDIR(fs.st_mode));
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::FileExists(const char_type* filename, EExistPolicy policy) {
    Assert(filename);

    if (not FPlatformLowLevelIO::Access(filename, policy))
        return false;

    struct ::stat fs;
    return (::stat(WCHAR_TO_UTF_8<MaxPathLength>(filename), &fs) == 0 && S_ISREG(fs.st_mode));
}
//----------------------------------------------------------------------------
void FLinuxPlatformFile::EnumerateDir(const char_type* dirpath, const TFunctionRef<void(const FWStringView&)>& onFile, const TFunctionRef<void(const FWStringView&)>& onSubDir) {
    Assert(dirpath);
    Assert(onSubDir.Valid() || onFile.Valid());

    EnumerateDirNonRecursive_(dirpath,
        [&onFile](const FWStringView& file) { if (onFile) onFile(file); },
        [&onSubDir](const FWStringView& subdir) { if (onSubDir) onSubDir(subdir); });
}
//----------------------------------------------------------------------------
void FLinuxPlatformFile::EnumerateFiles(const char_type* dirpath, bool recursive, const TFunctionRef<void(const FWStringView&)>& onFile) {
    Assert(dirpath);
    Assert(onFile);

    if (recursive) {
        char_type buffer[MaxPathLength];

        VECTORINSITU(FileSystem, FDirectoryIterator_, MaxPathDepth) stack;
        stack.emplace_back(MakeCStringView(dirpath));

        FDirectoryIterator_ currDir;
        do {
            currDir = std::move(stack.back());
            stack.pop_back();

            const size_t offset = stack.size();

            EnumerateDirNonRecursive_(currDir.FullName.c_str(),
                [&currDir, &buffer, &onFile](const FWStringView & file) {
                    const size_t len = Format(buffer, L"{0}\\{1}", currDir.Relative, file);
                    onFile(FWStringView(buffer, len));
                },
                [&currDir, &stack](const FWStringView & subDir) {
                    stack.emplace_back(
                        StringFormat(L"{0}\\{1}", currDir.FullName, subDir),
                        StringFormat(L"{0}\\{1}", currDir.Relative, subDir));
                });

            std::reverse(stack.begin() + offset, stack.end());

        } while (not stack.empty());
    }
    else {
        EnumerateDirNonRecursive_(dirpath, onFile, [](const FWStringView&){});
    }
}
//----------------------------------------------------------------------------
void FLinuxPlatformFile::GlobFiles(const char_type* dirpath, const char_type* pattern, bool recursive, const TFunctionRef<void(const FWStringView&)>& onMatch) {
    Assert(dirpath);
    Assert(pattern);
    Assert(onMatch);

    if (recursive) {
        struct FGlobContext_ {
            FWStringView Pattern;
            FDirectoryIterator_ Directory;
            char_type Buffer[MaxPathLength + 1];

        }   context;
        context.Pattern = MakeCStringView(pattern);

        VECTORINSITU(FileSystem, FDirectoryIterator_, MaxPathDepth) stack;
        stack.emplace_back(MakeCStringView(dirpath));

        do {
            context.Directory = std::move(stack.back());
            stack.pop_back();

            const size_t offset = stack.size();

            EnumerateDirNonRecursive_(context.Directory.FullName.c_str(),
                [&context, &onMatch](const FWStringView& file) {
                    if (WildMatchI(context.Pattern, file)) {
                        const size_t len = Format(context.Buffer, L"{0}\\{1}", context.Directory.Relative, file);
                        onMatch(FWStringView(context.Buffer, len));
                    }
                },
                [&context, &stack](const FWStringView& subDir) {
                    stack.emplace_back(
                        StringFormat(L"{0}\\{1}", context.Directory.FullName, subDir),
                        StringFormat(L"{0}\\{1}", context.Directory.Relative, subDir));
                });

            std::reverse(stack.begin() + offset, stack.end());

        } while (not stack.empty());
    }
    else {
        GlobFilesNonRecursive_(dirpath, pattern, onMatch);
    }
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::CreateDirectory(const char_type* dirpath, bool* existed) {
    Assert(dirpath);

    auto dirpathUTF_8 = WCHAR_TO_UTF_8<MaxPathLength>(dirpath);

    struct ::stat fs;
    if (::stat(dirpathUTF_8, &fs) == 0) {
        if (S_ISDIR(fs.st_mode)) {
            if (existed)
                *existed = true;

            return true;
        }
        else {
            PPE_LOG(HAL, Error, "mkdir({0} failed: can't create a folder over a file",
                MakeCStringView(dirpath) );

            return false;
        }
    }

    if (existed)
        *existed = false;

    if (::mkdir(dirpathUTF_8, GDirectoryMode)) {
        PPE_LOG(HAL, Error, "mkdir({0}) failed with errno: {1}",
            MakeCStringView(dirpath), FErrno{});

        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::CreateDirectoryRecursively(const char_type* dirpath, bool* existed) {
    Assert(dirpath);

    size_t sz = 0;
    wchar_t folder[PATH_MAX + 1];

    FWStringView slice;
    FWStringView s = MakeCStringView(dirpath);
    while (Split(s, L'/', slice)) {
        if (sz)
            folder[sz++] = PathSeparator;

        slice.CopyTo(folder, sz);
        sz += slice.size();

        Assert(sz < lengthof(folder));
        folder[sz] = L'\0';

        if (not FLinuxPlatformFile::CreateDirectory(folder, existed))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::MoveFile(const char_type* src, const char_type* dst) {
    Assert(src);
    Assert(dst);

    if (::rename(
        WCHAR_TO_UTF_8<MaxPathLength>(src),
        WCHAR_TO_UTF_8<MaxPathLength>(dst) ) == 0)
        return true;

    PPE_LOG(HAL, Error, "rename({0}, {1}) failed with errno: {2}",
        MakeCStringView(src), MakeCStringView(dst), FErrno{} );

    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::RemoveDirectory(const char_type* dirpath, bool force) {
    Assert(dirpath);

    // first assume empty directory
    if (::rmdir(WCHAR_TO_UTF_8<MaxPathLength>(dirpath)) == 0)
        return true;

    if (force) {
        struct FDirectoryPass_ {
            FWString Path;
            bool Empty = false;
            FDirectoryPass_() = default;
            FDirectoryPass_(const FWStringView& path)
                : Path(path)
            {}
        };
        VECTORINSITU(FileSystem, FDirectoryPass_, MaxPathDepth) stack;
        stack.emplace_back(MakeCStringView(dirpath));

        FDirectoryPass_ currDir;

        do {
            currDir = std::move(stack.back());
            stack.pop_back();

            if (not currDir.Empty) {
                const size_t offset = stack.size();

                if (not EnumerateDirNonRecursive_(
                    currDir.Path.c_str(),
                    [&currDir](const FWStringView& file) {
                        char_type buffer[MaxPathLength];
                        Format(buffer, L"{0}/{1}\0", currDir.Path, file);
                        return RemoveFile(buffer);
                    },
                    [&currDir, &stack](const FWStringView& subDir) {
                        stack.emplace_back(StringFormat(L"{0}/{1}", currDir.Path, subDir));
                    }))
                    return false;

                if (offset != stack.size()) {
                    currDir.Empty = true;
                    stack.push_back(std::move(currDir));
                    std::reverse(stack.begin() + offset, stack.end());
                    continue;
                }
            }

            if (::rmdir(WCHAR_TO_UTF_8<MaxPathLength>(currDir.Path)) != 0) {
                PPE_LOG(HAL, Error, "rmdir({0}) failed with errno: {1}",
                    currDir.Path, FErrno{} );
                return false;
            }

        } while (not stack.empty());

        return true;
    }
    else {
        PPE_LOG(HAL, Error, "rmdir({0}) failed with errno: {1}",
            MakeCStringView(dirpath), FErrno{} );
        return false;
    }
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::RemoveFile(const char_type* filename) {
    Assert(filename);

    if (::remove(WCHAR_TO_UTF_8<MaxPathLength>(filename)) == 0)
        return true;

    PPE_LOG(HAL, Error, "remove({0}) failed with errno: {1}",
        MakeCStringView(filename), FErrno{} );

    return false;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::SetFileTime(
    const char_type* filename,
    const FTimestamp* pCreatedAtIFN,
    const FTimestamp* pLastAccessIFN,
    const FTimestamp* pLastModifiedIFN ) {
    Assert(filename);
    Assert(pCreatedAtIFN || pLastAccessIFN || pLastModifiedIFN); // should have a side effect

    if (pCreatedAtIFN)
        AssertNotReached(); // no supported on linux :|

    struct ::timeval timevals[2];
    struct ::timeval& atime = timevals[0];
    struct ::timeval& mtime = timevals[1];

    auto filenameUTF_8 = WCHAR_TO_UTF_8<MaxPathLength>(filename);

    struct ::stat fs;
    if (::stat(filenameUTF_8, &fs) == 0) {
        atime.tv_sec = (pLastAccessIFN ? pLastAccessIFN->Value() : fs.st_atime);
        atime.tv_usec = 0;

        mtime.tv_sec = (pLastModifiedIFN ? pLastModifiedIFN->Value() : fs.st_mtime);
        mtime.tv_usec = 0;

        if (::utimes(filenameUTF_8, timevals) != 0) {
            PPE_LOG(HAL, Error, "utimes({0}) failed with errno: {1}",
                MakeCStringView(filename), FErrno{} );

            return false;
        }
    }
    else {
        PPE_LOG(HAL, Error, "stat({0}) failed with errno: {1}",
            MakeCStringView(filename), FErrno{} );

        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformFile::RollFile(const char_type* filename) {
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

            PPE_LOG(HAL, Warning, "file roll failed because '{0}' already exists, trying {1}",
                logroll, logroll_bak );

            logroll = std::move(logroll_bak);
        }

        PPE_LOG(HAL, Info, "roll file '{0}' -> '{1}'", MakeCStringView(filename), logroll);

        if (not MoveFile(filename, logroll.c_str()))
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
FWString FLinuxPlatformFile::MakeTemporaryFile(const char_type* prefix, const char_type* extname) {
    Assert(prefix);
    Assert(extname);
    Assert(L'.' == *extname);

    u32 year, mon, mday, day, hour, min, sec, msec;
    FPlatformTime::UtcTime(FPlatformTime::Timestamp(), year, mon, mday, day, hour, min, sec, msec);

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

#endif //!PLATFORM_LINUX
