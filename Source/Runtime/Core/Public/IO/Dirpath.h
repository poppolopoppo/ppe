#pragma once

#include "Core.h"

#include "IO/FileSystem_fwd.h"

#include "IO/Dirname.h"
#include "IO/FileSystemProperties.h"

#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/UniqueView.h"

#include <initializer_list>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FDirpath {
public:
    enum : size_t { MaxDepth = 16 };

    CONSTEXPR FDirpath() = default;
    CONSTEXPR explicit FDirpath(PFileSystemNode path) NOEXCEPT : _path(path) {}

    FDirpath(const FDirpath& other) = default;
    FDirpath& operator =(const FDirpath& other) = default;

    FDirpath(const PPE::FMountingPoint& mountingPoint, const TMemoryView<const FDirname>& path);

    FDirpath(const FDirpath& other, const FDirname& append);
    FDirpath(const FDirpath& other, const TMemoryView<const FDirname>& append);

    template <typename _It>
    FDirpath(const PPE::FMountingPoint& mountingPoint, _It&& ibegin, _It&& iend)
    :   FDirpath(mountingPoint, MakeView(ibegin, iend)) {}

    FDirpath(std::initializer_list<const FileSystem::char_type *> path);

    FDirpath(const FileSystem::FString& str);
    FDirpath& operator =(const FileSystem::FString& str);

    FDirpath(const FileSystem::FStringView& content);
    FDirpath& operator =(const FileSystem::FStringView& content);

    template <size_t _Dim>
    FDirpath(const FileSystem::char_type (&content)[_Dim]) : FDirpath(MakeStringView(content)) {}
    template <size_t _Dim>
    FDirpath& operator =(const FileSystem::char_type (&content)[_Dim]) { return operator =(MakeStringView(content)); }

    NODISCARD size_t Depth() const;
    NODISCARD FMountingPoint MountingPoint() const;
    NODISCARD FDirname LastDirname() const;

    NODISCARD size_t ExpandPath(FMountingPoint& mountingPoint, const TMemoryView<FDirname>& dirnames) const; // returns dirnames size

    void AssignTokens(const TMemoryView<const FFileSystemToken>& tokens);
    NODISCARD TMemoryView<const FFileSystemToken> ExpandTokens() const;

    NODISCARD bool empty() const { return nullptr == _path; }

    NODISCARD bool HasMountingPoint() const;

    NODISCARD bool IsAbsolute() const { return HasMountingPoint(); }
    NODISCARD bool IsRelative() const { return (not HasMountingPoint()); }

    NODISCARD bool IsSubdirectory(const FDirpath& other) const;

    void Concat(const FDirname& append);
    void Concat(const TMemoryView<const FDirname>& path);
    void Concat(const FileSystem::char_type *cstr);
    void Concat(const TBasicStringView<FileSystem::char_type>& strview);

    NODISCARD FString ToString() const;
    NODISCARD FWString ToWString() const;

    NODISCARD FStringView ToCStr(const TMemoryView<char>& dst) const;
    NODISCARD FWStringView ToWCStr(const TMemoryView<wchar_t>& dst) const;

    void Clear() NOEXCEPT;
    void Swap(FDirpath& other) NOEXCEPT;

    NODISCARD bool Equals(const FDirpath& other) const { return _path == other._path; }
    NODISCARD bool Less(const FDirpath& other) const;

    NODISCARD size_t HashValue() const;

    NODISCARD PFileSystemNode PathNode() const { return _path; }

    NODISCARD static bool Absolute(FDirpath* absolute, const FDirpath& origin, const FDirpath& relative);
    NODISCARD static bool Normalize(FDirpath* normalized, const FDirpath& path);
    NODISCARD static bool Relative(FDirpath* relative, const FDirpath& origin, const FDirpath& other);

    NODISCARD static FDirpath FromTokens(const TMemoryView<const FFileSystemToken>& tokens);

    PPE_CORE_API friend FDirpath operator /(const FDirpath& lhs, const FDirname& rhs);
    PPE_CORE_API friend FFilename operator /(const FDirpath& lhs, const class FBasename& basename);

private:
    PFileSystemNode _path;
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FDirpath)
//----------------------------------------------------------------------------
NODISCARD inline bool operator ==(const FDirpath& lhs, const FDirpath& rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
NODISCARD inline bool operator !=(const FDirpath& lhs, const FDirpath& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
NODISCARD inline bool operator <(const FDirpath& lhs, const FDirpath& rhs) {
    return lhs.Less(rhs);
}
//----------------------------------------------------------------------------
NODISCARD inline bool operator >=(const FDirpath& lhs, const FDirpath& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
inline void swap(FDirpath& lhs, FDirpath& rhs) NOEXCEPT {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
NODISCARD inline hash_t hash_value(const FDirpath& dirpath) NOEXCEPT {
    return dirpath.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FDirpath& dirpath);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FDirpath& dirpath);
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool operator >>(const FStringConversion& iss, FDirpath* dirpath);
NODISCARD PPE_CORE_API bool operator >>(const FWStringConversion& iss, FDirpath* dirpath);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
