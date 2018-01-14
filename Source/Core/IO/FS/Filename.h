#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/Basename.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/FileSystemProperties.h"
#include "Core/IO/FS/MountingPoint.h"

#include "Core/IO/String_fwd.h"
#include "Core/IO/TextWriter_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CORE_API FFilename {
public:
    FFilename() {}
    ~FFilename() {}

    FFilename(FDirpath&& dirpath, FBasename&& basename);
    FFilename(const FDirpath& dirpath, const FBasename& basename);
    FFilename(const FDirpath& dirpath, const FileSystem::FStringView& relfilename);

    FFilename(FFilename&& rvalue);
    FFilename& operator =(FFilename&& rvalue);

    FFilename(const FFilename& other);
    FFilename& operator =(const FFilename& other);

    FFilename(const FileSystem::FString& content);
    FFilename& operator =(const FileSystem::FString& content);

    FFilename(const FileSystem::FStringView& content);
    FFilename& operator =(const FileSystem::FStringView& content);

    template <size_t _Dim>
    FFilename(const FileSystem::char_type (&content)[_Dim]) : FFilename(MakeStringView(content)) {}
    template <size_t _Dim>
    FFilename& operator =(const FileSystem::char_type (&content)[_Dim]) { return operator =(MakeStringView(content)); }

    void Swap(FFilename& other);

    const FDirpath& Dirpath() const { return _dirpath; }

    FBasename& Basename() { return _basename; }
    const FBasename& Basename() const { return _basename; }

    FMountingPoint MountingPoint() const { return _dirpath.MountingPoint(); }
    void SetMountingPoint(const FMountingPoint& mountingPoint);
    FFilename WithReplacedMountingPoint(const FMountingPoint& mountingPoint) const;

    size_t ExpandPath(FMountingPoint& mountingPoint, TMemoryView<FDirname>& dirnames) const { return _dirpath.ExpandPath(mountingPoint, dirnames); }
    const FBasenameNoExt& BasenameNoExt() const { return _basename.BasenameNoExt(); }
    const FExtname& Extname() const { return _basename.Extname(); }

    bool empty() const { return _dirpath.empty() && _basename.empty(); }

    bool HasExtname() const { return _basename.HasExtname(); }
    bool HasMountingPoint() const { return _dirpath.HasMountingPoint(); }

    void AppendBasename(const FileSystem::FStringView& basenameNoExt);
    void AppendBasename(const FBasenameNoExt& basenameNoExt) { AppendBasename(basenameNoExt.MakeView()); }
    FFilename WithAppendBasename(const FileSystem::FStringView& basenameNoExt) const;
    FFilename WithAppendBasename(const FBasenameNoExt& basenameNoExt) const { return WithAppendBasename(basenameNoExt.MakeView()); }

    void ReplaceExtension(const FExtname& ext);
    FFilename WithReplacedExtension(const FExtname& ext) const;

    bool Absolute(FFilename* absolute, const FDirpath& origin) const;
    bool Normalize(FFilename* normalized) const;
    bool Relative(FFilename* relative, const FDirpath& origin) const;

    FFilename Absolute(const FDirpath& origin) const;
    FFilename Normalized() const;
    FFilename Relative(const FDirpath& origin) const;

    bool Equals(const FFilename& other) const;
    bool Less(const FFilename& other) const;

    size_t HashValue() const;

    FString ToString() const;
    FWString ToWString() const;

    FStringView ToCStr(const TMemoryView<char>& dst) const;
    FWStringView ToWCStr(const TMemoryView<wchar_t>& dst) const;

    template <size_t _Dim>
    size_t ToCStr(char (&dst)[_Dim]) const { return ToCStr(dst, _Dim); }
    template <size_t _Dim>
    size_t ToWCStr(wchar_t (&dst)[_Dim]) const { return ToWCStr(dst, _Dim); }

private:
    FDirpath _dirpath;
    FBasename _basename;
};
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
inline bool operator ==(const FFilename& lhs, const FFilename& rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
inline bool operator !=(const FFilename& lhs, const FFilename& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline bool operator <(const FFilename& lhs, const FFilename& rhs) {
    return lhs.Less(rhs);
}
//----------------------------------------------------------------------------
inline bool operator >=(const FFilename& lhs, const FFilename& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
inline void swap(FFilename& lhs, FFilename& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FFilename& filename) {
    return filename.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, const FFilename& filename);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FFilename& filename);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
