#pragma once

#include "Core.h"
#include "FileSystemProperties.h"
#include "Basename.h"
#include "Dirpath.h"
#include "MountingPoint.h"
#include "StringSlice.h"
#include "String.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Filename {
public:
    Filename() {}
    ~Filename() {}

    Filename(Core::Dirpath&& dirpath, Core::Basename&& basename);
    Filename(const Core::Dirpath& dirpath, const Core::Basename& basename);

    Filename(Filename&& rvalue);
    Filename& operator =(Filename&& rvalue);

    Filename(const Filename& other);
    Filename& operator =(const Filename& other);

    Filename(const FileSystem::char_type* content);
    Filename& operator =(const FileSystem::char_type* content);

    Filename(const FileSystem::char_type* content, size_t length);
    Filename(const BasicStringSlice<FileSystem::char_type>& content);

    template <typename _CharTraits, typename _Allocator>
    Filename(const std::basic_string<FileSystem::char_type, _CharTraits, _Allocator>& content)
        : Filename(content.c_str(), content.size()) {}

    void Swap(Filename& other);

    const Core::Dirpath& Dirpath() const { return _dirpath; }
    const Core::Basename& Basename() const { return _basename; }

    const Core::MountingPoint& MountingPoint() const { return _dirpath.MountingPoint(); }
    const Core::Dirpath::Dirnames& Path() const { return _dirpath.Path(); }
    const Core::BasenameNoExt& BasenameNoExt() const { return _basename.BasenameNoExt(); }
    const Core::Extname& Extname() const { return _basename.Extname(); }

    bool empty() const { return _dirpath.empty() &&
                                _basename.empty(); }

    bool Equals(const Filename& other) const;
    bool Less(const Filename& other) const;

    size_t HashValue() const;
    String ToString() const;

private:
    Core::Dirpath _dirpath;
    Core::Basename _basename;
};
//----------------------------------------------------------------------------
inline bool Filename::Equals(const Filename& other) const {
    return  _basename == other._basename &&
            _dirpath == other._dirpath;
}
//----------------------------------------------------------------------------
inline bool Filename::Less(const Filename& other) const {
    if (_dirpath == other._dirpath)
        return _basename < other._basename;
    else
        return _dirpath < other._dirpath;
}
//----------------------------------------------------------------------------
inline bool operator ==(const Filename& lhs, const Filename& rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
inline bool operator !=(const Filename& lhs, const Filename& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline bool operator <(const Filename& lhs, const Filename& rhs) {
    return lhs.Less(rhs);
}
//----------------------------------------------------------------------------
inline bool operator >=(const Filename& lhs, const Filename& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
inline void swap(Filename& lhs, Filename& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline size_t hash_value(const Filename& filename) {
    return filename.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Core::Filename& filename) {
    if (!filename.Dirpath().empty())
        oss << filename.Dirpath();
    return oss << filename.Basename();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
