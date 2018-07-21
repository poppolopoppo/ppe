#pragma once

#include "Core/HAL/Generic/GenericPlatformFile.h"

#ifdef PLATFORM_WINDOWS

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct CORE_API FWindowsPlatformFile : FGenericPlatformFile {
public:
    STATIC_CONST_INTEGRAL(bool, IsCaseSensitive, false);
    STATIC_CONST_INTEGRAL(size_t, MaxPathLength, 260);
    STATIC_CONST_INTEGRAL(wchar_t, PathSeparator, L'\\');
    STATIC_CONST_INTEGRAL(wchar_t, PathSeparatorAlt, L'/');

    using char_type = wchar_t;
    using FStat = FWindowsPlatformFile::FStat;

    static FWString TemporaryDirectory();
    static FWString UserDirectory();
    static FWString WorkingDirectory();

    static bool IsAllowedChar(char_type ch);
    static bool NormalizePath(FWString& path);

    static bool TotalSizeAndUsage(u64* pTotalSize, u64* pUsedSize, const char_type* path);

    static bool Stat(FStat* pstat, const char_type* filename);

    static bool DirectoryExists(const char_type* dirpath, EExistPolicy policy);
    static bool FileExists(const char_type* filename, EExistPolicy policy);

    static void EnumerateDir(const char_type* dirpath, const TFunction<void(const FWStringView&)>& onFile, const TFunction<void(const FWStringView&)>& onSubDir);
    static void EnumerateFiles(const char_type* dirpath, bool recursive, const TFunction<void(const FWStringView&)>& onFile);
    static void GlobFiles(const char_type* dirpath, const char_type* pattern, bool recursive, const TFunction<void(const FWStringView&)>& onMatch);

    static bool CreateDirectory(const char_type* dirpath, bool* existed);
    static bool MoveFile(const char_type* src, const char_type* dst);

    static bool RemoveDirectory(const char_type* dirpath, bool force);
    static bool RemoveFile(const char_type* filename);

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif //!PLATFORM_WINDOWS
