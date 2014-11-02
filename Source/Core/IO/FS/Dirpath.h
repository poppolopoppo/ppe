#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/FileSystemProperties.h"

#include "Core/Memory/UniqueView.h"
#include "Core/IO/StringSlice.h"

#include <initializer_list>
#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Dirname;
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

    Dirpath(const FileSystem::char_type* content);
    Dirpath& operator =(const FileSystem::char_type* content);

    Dirpath(const FileSystem::char_type* content, size_t length);

    Dirpath(const BasicStringSlice<FileSystem::char_type>& slice);
    Dirpath& operator =(const BasicStringSlice<FileSystem::char_type>& slice);

    template <typename _CharTraits, typename _Allocator>
    Dirpath(const std::basic_string<FileSystem::char_type, _CharTraits, _Allocator>& str)
        : Dirpath(str.c_str(), str.size()) {}

    Core::MountingPoint MountingPoint() const;
    Core::Dirname LastDirname() const;
    size_t ExpandPath(Core::MountingPoint& mountingPoint, const MemoryView<Dirname>& dirnames) const; // returns dirnames size

    bool empty() const { return nullptr == _path; }

    void Concat(const Dirname& append);
    void Concat(const MemoryView<const Dirname>& path);

    void Swap(Dirpath& other);

    bool Equals(const Dirpath& other) const { return _path == other._path; }
    bool Less(const Dirpath& other) const;

    size_t HashValue() const;

    const FileSystemNode *PathNode() const { return _path.get(); }

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
inline size_t hash_value(const Dirpath& dirpath) {
    return dirpath.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Core::Dirpath& dirpath) {
    Core::MountingPoint mp;
    const auto dirs = MALLOCA_VIEW(Dirname, Dirpath::MaxDepth);
    const size_t k = dirpath.ExpandPath(mp, dirs);
    if (!mp.empty())
        oss << mp << wchar_t(FileSystem::Separator);
    for (size_t i = 0; i < k; ++i)
        oss << dirs[i] << wchar_t(FileSystem::Separator);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
