#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/Basename.h"
#include "Core/IO/FS/Dirpath.h"
#include "Core/IO/FS/FileSystemProperties.h"
#include "Core/IO/FS/MountingPoint.h"

#include "Core/IO/String.h"
#include "Core/IO/StringSlice.h"

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
    Filename(const Core::Dirpath& dirpath, const FileSystem::char_type *relfilename);
    Filename(const Core::Dirpath& dirpath, const FileSystem::char_type *relfilename, size_t length);
    Filename(const Core::Dirpath& dirpath, const MemoryView<const FileSystem::char_type>& relfilename);

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

    Core::Basename& Basename() { return _basename; }
    const Core::Basename& Basename() const { return _basename; }

    Core::MountingPoint MountingPoint() const { return _dirpath.MountingPoint(); }
    void SetMountingPoint(const Core::MountingPoint& mountingPoint);

    size_t ExpandPath(Core::MountingPoint& mountingPoint, MemoryView<Dirname>& dirnames) const { return _dirpath.ExpandPath(mountingPoint, dirnames); }
    const Core::BasenameNoExt& BasenameNoExt() const { return _basename.BasenameNoExt(); }
    const Core::Extname& Extname() const { return _basename.Extname(); }

    bool empty() const { return _dirpath.empty() && _basename.empty(); }

    bool HasExtname() const { return _basename.HasExtname(); }
    bool HasMountingPoint() const { return _dirpath.HasMountingPoint(); }

    void ReplaceExtension(const Core::Extname& ext);
    Filename WithReplacedExtension(const Core::Extname& ext) const;

    bool Equals(const Filename& other) const;
    bool Less(const Filename& other) const;

    size_t HashValue() const;

    String ToString() const;
    WString ToWString() const;

    size_t ToCStr(char *dst, size_t capacity) const;
    size_t ToWCStr(wchar_t *dst, size_t capacity) const;

    inline size_t ToWCStr(const MemoryView<wchar_t>& dst) const { return ToWCStr(dst.Pointer(), dst.SizeInBytes()); }

    template <size_t _Dim>
    size_t ToCStr(char (&dst)[_Dim]) const { return ToCStr(dst, _Dim); }
    template <size_t _Dim>
    size_t ToWCStr(wchar_t (&dst)[_Dim]) const { return ToWCStr(dst, _Dim); }

private:
    Core::Dirpath _dirpath;
    Core::Basename _basename;
};
//----------------------------------------------------------------------------
inline void Filename::ReplaceExtension(const Core::Extname& ext) {
    _basename.SetExtname(ext);
}
//----------------------------------------------------------------------------
inline bool Filename::Equals(const Filename& other) const {
    return  _basename == other._basename &&
            _dirpath == other._dirpath;
}
//----------------------------------------------------------------------------
inline bool Filename::Less(const Filename& other) const {
    return (_dirpath == other._dirpath)
        ? _basename < other._basename
        : _dirpath < other._dirpath;
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
inline hash_t hash_value(const Filename& filename) {
    return filename.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Core::Filename& filename) {
    if (filename.empty())
        return oss;
    if (!filename.Dirpath().empty())
        oss << filename.Dirpath();
    return oss << filename.Basename();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
