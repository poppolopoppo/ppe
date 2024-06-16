#pragma once

#include "Core.h"

#include "IO/Basename.h"
#include "IO/Dirpath.h"
#include "IO/FileSystemProperties.h"
#include "IO/MountingPoint.h"

#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FFilename {
public:
    FFilename() = default;

    FFilename(FFilename&& rvalue) = default;
    FFilename& operator =(FFilename&& rvalue) = default;

    FFilename(const FFilename& other) = default;
    FFilename& operator =(const FFilename& other) = default;

    FFilename(FDirpath&& dirpath, FBasename&& basename) NOEXCEPT;
    FFilename(const FDirpath& dirpath, const FBasename& basename) NOEXCEPT;

    FFilename(FDirpath&& dirpath, FBasenameNoExt&& basenameNoExt, FExtname&& extname) NOEXCEPT;
    FFilename(const FDirpath& dirpath, const FBasenameNoExt& basenameNoExt, const FExtname& extname) NOEXCEPT;

    FFilename(const FDirpath& dirpath, const FileSystem::FStringView& relfilename);

    FFilename(const FileSystem::FString& content);
    FFilename& operator =(const FileSystem::FString& content);

    FFilename(const FileSystem::FStringView& content);
    FFilename& operator =(const FileSystem::FStringView& content);

    FFilename& operator =(Meta::FDefaultValue) {
        Clear();
        return (*this);
    }

    template <size_t _Dim>
    FFilename(const FileSystem::char_type (&content)[_Dim]) : FFilename(MakeStringView(content)) {}
    template <size_t _Dim>
    FFilename& operator =(const FileSystem::char_type (&content)[_Dim]) { return operator =(MakeStringView(content)); }

    void Clear() NOEXCEPT;
    void Swap(FFilename& other) NOEXCEPT;

    NODISCARD const FDirpath& Dirpath() const { return _dirpath; }
    NODISCARD const FDirectory& Directory() const { return _dirpath; }

    NODISCARD FBasename& Basename() { return _basename; }
    NODISCARD const FBasename& Basename() const { return _basename; }

    NODISCARD FMountingPoint MountingPoint() const { return _dirpath.MountingPoint(); }
    void SetMountingPoint(const FMountingPoint& mountingPoint);
    NODISCARD FFilename WithReplacedMountingPoint(const FMountingPoint& mountingPoint) const;

    NODISCARD size_t ExpandPath(FMountingPoint& mountingPoint, const TMemoryView<FDirname>& dirnames) const { return _dirpath.ExpandPath(mountingPoint, dirnames); }
    NODISCARD const FBasenameNoExt& BasenameNoExt() const { return _basename.BasenameNoExt(); }
    NODISCARD const FExtname& Extname() const { return _basename.Extname(); }

    NODISCARD bool empty() const { return _dirpath.empty() && _basename.empty(); }

    NODISCARD bool HasExtname() const { return _basename.HasExtname(); }
    NODISCARD bool HasMountingPoint() const { return _dirpath.HasMountingPoint(); }

    NODISCARD bool IsAbsolute() const { return _dirpath.IsAbsolute(); }
    NODISCARD bool IsRelative() const { return _dirpath.IsRelative(); }

    void AppendBasename(const FileSystem::FStringView& basenameNoExt);
    void AppendBasename(const FBasenameNoExt& basenameNoExt) { AppendBasename(basenameNoExt.MakeView()); }
    NODISCARD FFilename WithAppendBasename(const FileSystem::FStringView& basenameNoExt) const;
    NODISCARD FFilename WithAppendBasename(const FBasenameNoExt& basenameNoExt) const { return WithAppendBasename(basenameNoExt.MakeView()); }

    void ReplaceExtension(const FExtname& ext);
    NODISCARD FFilename WithReplacedExtension(const FExtname& ext) const;

    NODISCARD bool Absolute(FFilename* absolute, const FDirpath& origin) const;
    NODISCARD bool Normalize(FFilename* normalized) const;
    NODISCARD bool Relative(FFilename* relative, const FDirpath& origin) const;

    NODISCARD FFilename Absolute(const FDirpath& origin) const;
    NODISCARD FFilename Normalized() const;
    NODISCARD FFilename Relative(const FDirpath& origin) const;
    NODISCARD FFilename RemoveExtname() const;

    NODISCARD bool IsRelativeTo(const FDirpath& origin) const;

    NODISCARD bool Equals(const FFilename& other) const;
    NODISCARD bool Less(const FFilename& other) const;

    NODISCARD size_t HashValue() const;

    NODISCARD FString ToString() const;
    NODISCARD FWString ToWString() const;

    FStringView ToCStr(const TMemoryView<char>& dst) const;
    FWStringView ToWCStr(const TMemoryView<wchar_t>& dst) const;

private:
    FDirpath _dirpath;
    FBasename _basename;
};
//----------------------------------------------------------------------------
PPE_ASSUME_TYPE_AS_POD(FFilename)
//----------------------------------------------------------------------------
inline void FFilename::ReplaceExtension(const FExtname& ext) {
    _basename.SetExtname(ext);
}
//----------------------------------------------------------------------------
inline bool FFilename::Equals(const FFilename& other) const {
    return  _basename == other._basename &&
            _dirpath == other._dirpath;
}
//----------------------------------------------------------------------------
inline bool FFilename::Less(const FFilename& other) const {
    return (_dirpath == other._dirpath)
        ? _basename < other._basename
        : _dirpath < other._dirpath;
}
//----------------------------------------------------------------------------
NODISCARD inline bool operator ==(const FFilename& lhs, const FFilename& rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
NODISCARD inline bool operator !=(const FFilename& lhs, const FFilename& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
NODISCARD inline bool operator <(const FFilename& lhs, const FFilename& rhs) {
    return lhs.Less(rhs);
}
//----------------------------------------------------------------------------
NODISCARD inline bool operator >=(const FFilename& lhs, const FFilename& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
inline void swap(FFilename& lhs, FFilename& rhs) NOEXCEPT {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FFilename& filename) NOEXCEPT {
    return filename.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FFilename& filename);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FFilename& filename);
//----------------------------------------------------------------------------
PPE_CORE_API bool operator >>(const FStringConversion& iss, FFilename* filename);
PPE_CORE_API bool operator >>(const FWStringConversion& iss, FFilename* filename);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
