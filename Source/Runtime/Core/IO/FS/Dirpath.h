#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/Dirname.h"
#include "Core/IO/FS/FileSystemProperties.h"

#include "Core/IO/String_fwd.h"
#include "Core/IO/TextWriter_fwd.h"
#include "Core/Memory/UniqueView.h"

#include <initializer_list>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFilename;
FWD_REFPTR(FileSystemNode);
class FMountingPoint;
//----------------------------------------------------------------------------
class CORE_API FDirpath {
public:
    enum : size_t { MaxDepth = 16 };

    FDirpath();
    ~FDirpath();

    FDirpath(FDirpath&& rvalue);
    FDirpath& operator =(FDirpath&& rvalue);

    FDirpath(const FDirpath& other);
    FDirpath& operator =(const FDirpath& other);

    FDirpath(const Core::FMountingPoint& mountingPoint, const TMemoryView<const FDirname>& path);

    FDirpath(const FDirpath& other, const FDirname& append);
    FDirpath(const FDirpath& other, const TMemoryView<const FDirname>& append);

    template <typename _It>
    FDirpath(const Core::FMountingPoint& mountingPoint, _It&& ibegin, _It&& iend)
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

    size_t Depth() const;
    Core::FMountingPoint MountingPoint() const;
    Core::FDirname LastDirname() const;

    size_t ExpandPath(Core::FMountingPoint& mountingPoint, const TMemoryView<FDirname>& dirnames) const; // returns dirnames size

    void AssignTokens(const TMemoryView<const FFileSystemToken>& tokens);
    void ExpandTokens(const TMemoryView<FFileSystemToken>& tokens) const;

    bool empty() const { return nullptr == _path; }

    bool HasMountingPoint() const;

    void Concat(const FDirname& append);
    void Concat(const TMemoryView<const FDirname>& path);
    void Concat(const FileSystem::char_type *cstr);
    void Concat(const TMemoryView<const FileSystem::char_type>& strview);

    FString ToString() const;
    FWString ToWString() const;

    FStringView ToCStr(const TMemoryView<char>& dst) const;
    FWStringView ToWCStr(const TMemoryView<wchar_t>& dst) const;

    void Swap(FDirpath& other);

    bool Equals(const FDirpath& other) const { return _path == other._path; }
    bool Less(const FDirpath& other) const;

    size_t HashValue() const;

    const FFileSystemNode *PathNode() const { return _path.get(); }

    static bool Absolute(FDirpath* absolute, const FDirpath& origin, const FDirpath& relative);
    static bool Normalize(FDirpath* normalized, const FDirpath& path);
    static bool Relative(FDirpath* relative, const FDirpath& origin, const FDirpath& other);

    friend FDirpath operator /(const FDirpath& lhs, const FDirname& rhs);
    friend FFilename operator /(const FDirpath& lhs, const class FBasename& basename);

private:
    SCFileSystemNode _path;
};
//----------------------------------------------------------------------------
inline bool operator ==(const FDirpath& lhs, const FDirpath& rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
inline bool operator !=(const FDirpath& lhs, const FDirpath& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline bool operator <(const FDirpath& lhs, const FDirpath& rhs) {
    return lhs.Less(rhs);
}
//----------------------------------------------------------------------------
inline bool operator >=(const FDirpath& lhs, const FDirpath& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
inline void swap(FDirpath& lhs, FDirpath& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FDirpath& dirpath) {
    return dirpath.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CORE_API FTextWriter& operator <<(FTextWriter& oss, const FDirpath& dirpath);
CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FDirpath& dirpath);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
