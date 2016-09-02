#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemProperties.h"

#include "Core/IO/String.h"
#include "Core/IO/StringView.h"
#include "Core/Memory/UniqueView.h"

#include <initializer_list>
#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Dirname;
class Filename;
FWD_REFPTR(FileSystemNode);
class MountingPoint;
//----------------------------------------------------------------------------
class Dirpath {
public:
    enum : size_t { MaxDepth = 16 };

    Dirpath();
    ~Dirpath();

    Dirpath(Dirpath&& rvalue);
    Dirpath& operator =(Dirpath&& rvalue);

    Dirpath(const Dirpath& other);
    Dirpath& operator =(const Dirpath& other);

    Dirpath(const Core::MountingPoint& mountingPoint, const MemoryView<const Dirname>& path);

    Dirpath(const Dirpath& other, const Dirname& append);
    Dirpath(const Dirpath& other, const MemoryView<const Dirname>& append);

    template <typename _It>
    Dirpath(const Core::MountingPoint& mountingPoint, _It&& ibegin, _It&& iend)
    :   Dirpath(mountingPoint, MakeView(ibegin, iend)) {}

    Dirpath(std::initializer_list<const FileSystem::char_type *> path);

    Dirpath(const FileSystem::StringView& content);
    Dirpath& operator =(const FileSystem::StringView& content);

    template <size_t _Dim>
    Dirpath(const FileSystem::char_type (&content)[_Dim]) : Dirpath(MakeStringView(content)) {}
    template <size_t _Dim>
    Dirpath& operator =(const FileSystem::char_type (&content)[_Dim]) { return operator =(MakeStringView(content)); }

    template <typename _CharTraits, typename _Allocator>
    Dirpath(const std::basic_string<FileSystem::char_type, _CharTraits, _Allocator>& str)
        : Dirpath(MakeStringView(str)) {}

    size_t Depth() const;
    Core::MountingPoint MountingPoint() const;
    Core::Dirname LastDirname() const;
    size_t ExpandPath(Core::MountingPoint& mountingPoint, const MemoryView<Dirname>& dirnames) const; // returns dirnames size

    bool empty() const { return nullptr == _path; }

    bool HasMountingPoint() const;

    void Concat(const Dirname& append);
    void Concat(const MemoryView<const Dirname>& path);
    void Concat(const FileSystem::char_type *cstr);
    void Concat(const MemoryView<const FileSystem::char_type>& strview);

    String ToString() const;
    WString ToWString() const;

    size_t ToCStr(char *dst, size_t capacity) const;
    size_t ToWCStr(wchar_t *dst, size_t capacity) const;

    void Swap(Dirpath& other);

    bool Equals(const Dirpath& other) const { return _path == other._path; }
    bool Less(const Dirpath& other) const;

    size_t HashValue() const;

    const FileSystemNode *PathNode() const { return _path.get(); }

    static bool Absolute(Dirpath* absolute, const Dirpath& origin, const Dirpath& relative);
    static bool Normalize(Dirpath* normalized, const Dirpath& path);
    static bool Relative(Dirpath* relative, const Dirpath& origin, const Dirpath& other);

private:
    SCFileSystemNode _path;
};
//----------------------------------------------------------------------------
inline bool operator ==(const Dirpath& lhs, const Dirpath& rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
inline bool operator !=(const Dirpath& lhs, const Dirpath& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline bool operator <(const Dirpath& lhs, const Dirpath& rhs) {
    return lhs.Less(rhs);
}
//----------------------------------------------------------------------------
inline bool operator >=(const Dirpath& lhs, const Dirpath& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
inline void swap(Dirpath& lhs, Dirpath& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const Dirpath& dirpath) {
    return dirpath.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Core::Dirpath& dirpath) {

    Core::MountingPoint mountingPoint;
    STACKLOCAL_POD_ARRAY(Dirname, dirnames, dirpath.Depth());
    const size_t k = dirpath.ExpandPath(mountingPoint, dirnames);

    if (false == mountingPoint.empty())
        oss << mountingPoint << wchar_t(FileSystem::Separator);
    for (size_t i = 0; i < k; ++i)
        oss << dirnames[i] << wchar_t(FileSystem::Separator);

    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
