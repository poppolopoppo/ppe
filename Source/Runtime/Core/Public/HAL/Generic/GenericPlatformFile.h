#pragma once

#include "HAL/TargetPlatform.h"

#include "IO/StreamPolicies.h"
#include "IO/String_fwd.h"
#include "Misc/Function.h"
#include "Time/Timestamp.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGenericPlatformFileStat {
    u16 UID;
    u16 GID;

    u16 Link;
    u16 Mode;

    u64 SizeInBytes;

    FTimestamp CreatedAt;
    FTimestamp LastAccess;
    FTimestamp LastModified;
};
//----------------------------------------------------------------------------
struct PPE_CORE_API FGenericPlatformFile {
public: // must be defined for every platform
    STATIC_CONST_INTEGRAL(bool, IsCaseSensitive, false);
    STATIC_CONST_INTEGRAL(size_t, MaxPathLength, 0);

    STATIC_CONST_INTEGRAL(wchar_t, PathSeparator, L'/');
    STATIC_CONST_INTEGRAL(wchar_t, PathSeparatorAlt, L'/');

    using char_type = wchar_t;
    using FStat = FGenericPlatformFileStat;

    static FWString TemporaryDirectory() = delete;
    static FWString UserDirectory() = delete;
    static FWString WorkingDirectory() = delete;

    static bool IsAllowedChar(char_type ch) = delete;
    static bool NormalizePath(FWString& path) = delete;

    static bool TotalSizeAndUsage(u64* pTotalSize, u64* pUsedSize, const char_type* path) = delete;

    static bool Stat(FStat* pstat, const char_type* path) = delete;

    static bool DirectoryExists(const char_type* dirpath, EExistPolicy policy) = delete;
    static bool FileExists(const char_type* filename, EExistPolicy policy) = delete;

    static void EnumerateDir(const char_type* dirpath, const char_type* pattern, const TFunction<void(const FWStringView&)>& onFile, const TFunction<void(const FWStringView&)>& onSubDir) = delete;
    static void EnumerateFiles(const char_type* dirpath, bool recursive, const TFunction<void(const FWStringView&)>& onFile) = delete;
    static void GlobFiles(const char_type* dirpath, const char_type* pattern, bool recursive, const TFunction<void(const FWStringView&)>& onMatch) = delete;

    static bool CreateDirectory(const char_type* dirpath, bool* existed) = delete;
    static bool MoveFile(const char_type* src, const char_type* dst) = delete;

    static bool RemoveDirectory(const char_type* dirpath, bool force) = delete;
    static bool RemoveFile(const char_type* filename) = delete;

    static bool SetFileTime(
        const char_type* filename,
        const FTimestamp* pCreatedAtIFP,
        const FTimestamp* pLastAccessIFP,
        const FTimestamp* pLastModifiedIFP ) = delete;

    static bool RollFile(const char_type* filename) = delete;
    static FWString MakeTemporaryFile(const char_type* prefix, const char_type* extname) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE