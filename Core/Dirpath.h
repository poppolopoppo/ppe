#pragma once

#include "Core.h"
#include "FileSystemProperties.h"
#include "Dirname.h"
#include "MountingPoint.h"
#include "RawStorage.h"
#include "StringSlice.h"

#include <initializer_list>
#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Dirpath {
public:
    typedef RAWSTORAGE(FileSystem, Dirname) Dirnames;

    Dirpath() {}
    ~Dirpath() {}

    Dirpath(Dirpath&& rvalue);
    Dirpath& operator =(Dirpath&& rvalue);

    Dirpath(const Dirpath& other);
    Dirpath& operator =(const Dirpath& other);

    Dirpath(const Core::MountingPoint& mountingPoint, Dirnames&& path);
    Dirpath(const Core::MountingPoint& mountingPoint, const MemoryView<const Dirname>& path);

    Dirpath(const Dirpath& other, const Dirname& append);
    Dirpath(const Dirpath& other, const MemoryView<const Dirname>& append);

    template <typename _It>
    Dirpath(const Core::MountingPoint& mountingPoint, _It&& begin, _It&& end)
        : _mountingPoint(mountingPoint), _path(begin, end) {}

    template <typename T>
    Dirpath(std::initializer_list<T> path)
        : _mountingPoint(*path.begin()), _path(path.begin() + 1, path.end()) {
        Assert(path.begin() != path.end());
    }

    Dirpath(const FileSystem::char_type* content);
    Dirpath& operator =(const FileSystem::char_type* content);

    Dirpath(const FileSystem::char_type* content, size_t length);
    Dirpath(const BasicStringSlice<FileSystem::char_type>& content);

    template <typename _CharTraits, typename _Allocator>
    Dirpath(const std::basic_string<FileSystem::char_type, _CharTraits, _Allocator>& content)
        : Dirpath(content.c_str(), content.size()) {}

    const Core::MountingPoint& MountingPoint() const { return _mountingPoint; }
    const Dirnames& Path() const { return _path; }

    bool empty() const { return _mountingPoint.empty() && _path.empty(); }

    void ConcatPath(const Dirname& append);
    void ConcatPath(const MemoryView<const Dirname>& path);

    template <typename _It>
    void ConcatPath(_It&& begin, _It&& end) {
        _path.insert(_path.end(), begin, end);
    }

    void Swap(Dirpath& other);

    bool Equals(const Dirpath& other) const;
    bool Less(const Dirpath& other) const;

    size_t HashValue() const;

private:
    Core::MountingPoint _mountingPoint;
    Dirnames _path;
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
    if (!dirpath.MountingPoint().empty())
        oss << dirpath.MountingPoint() << wchar_t(FileSystem::Separator);
    for (const Dirname& dirname : dirpath.Path())
        oss << dirname << wchar_t(FileSystem::Separator);
    return oss;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
