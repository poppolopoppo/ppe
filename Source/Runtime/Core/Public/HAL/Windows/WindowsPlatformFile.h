#pragma once

#include "HAL/Generic/GenericPlatformFile.h"

#ifdef PLATFORM_WINDOWS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_CORE_API FWindowsPlatformFile : FGenericPlatformFile {
public:
    STATIC_CONST_INTEGRAL(bool, IsCaseSensitive, false);

    STATIC_CONST_INTEGRAL(size_t, MaxPathDepth, 32);
    STATIC_CONST_INTEGRAL(size_t, MaxPathLength, 260);

    STATIC_CONST_INTEGRAL(wchar_t, PathSeparator, L'\\');
    STATIC_CONST_INTEGRAL(wchar_t, PathSeparatorAlt, L'/');

    using char_type = wchar_t;
    using FStat = FGenericPlatformFile::FStat;

    static FWString SystemDirectory();
    static FWString TemporaryDirectory();
    static FWString UserDirectory();
    static FWString WorkingDirectory();

    static bool IsAllowedChar(char_type ch);
    static bool NormalizePath(FWString& path);
    static FWString JoinPath(const std::initializer_list<FWStringView>& parts);

    static bool TotalSizeAndUsage(u64* pTotalSize, u64* pUsedSize, const char_type* path);

    static bool Stat(FStat* pstat, const char_type* filename);

    static bool DirectoryExists(const char_type* dirpath, EExistPolicy policy);
    static bool FileExists(const char_type* filename, EExistPolicy policy);

    static void EnumerateDir(const char_type* dirpath, const TFunctionRef<void(const FWStringView&)>& onFile, const TFunctionRef<void(const FWStringView&)>& onSubDir);
    static void EnumerateFiles(const char_type* dirpath, bool recursive, const TFunctionRef<void(const FWStringView&)>& onFile);
    static void GlobFiles(const char_type* dirpath, const char_type* pattern, bool recursive, const TFunctionRef<void(const FWStringView&)>& onMatch);

    static bool CreateDirectory(const char_type* dirpath, bool* existed);
    static bool CreateDirectoryRecursively(const char_type* dirpath, bool* existed);
    static bool MoveFile(const char_type* src, const char_type* dst);

    static bool RemoveDirectory(const char_type* dirpath, bool force);
    static bool RemoveFile(const char_type* filename);

    static bool SetFileTime(
        const char_type* filename,
        const FTimestamp* pCreatedAtIFN,
        const FTimestamp* pLastAccessIFN,
        const FTimestamp* pLastModifiedIFN);

    static bool RollFile(const char_type* filename);
    static FWString MakeTemporaryFile(const char_type* prefix, const char_type* extname);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
