#include "stdafx.h"

#include "Dirpath.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ParseDirpath_(
    const wchar_t *cstr,
    size_t length,
    MountingPoint& mountingPoint,
    Dirpath::Dirnames& path
    ) {
    mountingPoint = MountingPoint();

    if (!cstr || 0 == length) {
        path.Clear_ReleaseMemory();
        return false;
    }

    static const FileSystem::char_type *sep = L"\\/";

    WStringSlice slices[32];

    size_t count = 0;
    while (Split(&cstr, &length, sep, slices[count])) {
        Assert(count < lengthof(slices));
        if (slices[count].size())
            ++count;
    }

    if (0 == count) {
        path.Clear_ReleaseMemory();
        return false;
    }

    size_t first = 0;
    if (slices[0].back() == L':') {
        if (1 == slices[0].size()) {
            path.Clear_ReleaseMemory();
            return false;
        }

        mountingPoint = MountingPoint(slices[0].begin(), slices[0].size() );
        ++first;
    }

    path.Resize_DiscardData(count - first);
    for (size_t i = first; i < count; ++i)
        path[i - first] = Dirname(slices[i].begin(), slices[i].size() );

    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Dirpath::Dirpath(Dirpath&& rvalue)
:   _mountingPoint(std::move(rvalue._mountingPoint))
,   _path(std::move(rvalue._path)) {}
//----------------------------------------------------------------------------
Dirpath& Dirpath::operator =(Dirpath&& rvalue) {
    _mountingPoint = std::move(rvalue._mountingPoint);
    _path = std::move(rvalue._path);
    return *this;
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const Dirpath& other)
:   _mountingPoint(other._mountingPoint)
,   _path(other._path) {}
//----------------------------------------------------------------------------
Dirpath& Dirpath::operator =(const Dirpath& other) {
    _mountingPoint = other._mountingPoint;
    _path = other._path;
    return *this;
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const Core::MountingPoint& mountingPoint, Dirnames&& path)
:   _mountingPoint(mountingPoint), _path(std::move(path)) {}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const Core::MountingPoint& mountingPoint, const MemoryView<const Dirname>& path)
:   _mountingPoint(mountingPoint), _path(path.begin(), path.end()) {}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const Dirpath& other, const Dirname& append)
:   _mountingPoint(other.MountingPoint()) {
    Assert(!append.empty());
    const size_t k = other._path.size();
    _path.Resize_DiscardData(k + 1);
    for (size_t i = 0; i < k; ++i)
        _path[i] = other._path[i];
    _path[k] = append;
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const Dirpath& other, const MemoryView<const Dirname>& append)
:   _mountingPoint(other.MountingPoint()) {
    Assert(!append.empty());
    const size_t k = other._path.size();
    _path.Resize_DiscardData(k + append.size());
    for (size_t i = 0; i < k; ++i)
        _path[i] = other._path[i];
    for (size_t i = 0; i < append.size(); ++i) {
        Assert(!append[i].empty());
        _path[k + i] = append[i];
    }
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const FileSystem::char_type *content) {
    Assert(content);
    ParseDirpath_(content, Length(content), _mountingPoint, _path);
}
//----------------------------------------------------------------------------
Dirpath& Dirpath::operator =(const FileSystem::char_type *content) {
    Assert(content);
    ParseDirpath_(content, Length(content), _mountingPoint, _path);
    return *this;
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const FileSystem::char_type* content, size_t length) {
    Assert(content);
    ParseDirpath_(content, length, _mountingPoint, _path);
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const BasicStringSlice<FileSystem::char_type>& content) {
    Assert(content.begin());
    ParseDirpath_(content.begin(), content.size(), _mountingPoint, _path);
}
//----------------------------------------------------------------------------
void Dirpath::ConcatPath(const Dirname& append) {
    const size_t last = _path.size();
    _path.Resize_KeepData(last + 1);
    _path[last] = append;
}
//----------------------------------------------------------------------------
void Dirpath::ConcatPath(const MemoryView<const Dirname>& path) {
    _path.insert(_path.end(), path.begin(), path.end());
}
//----------------------------------------------------------------------------
void Dirpath::Swap(Dirpath& other) {
    std::swap(other._mountingPoint, _mountingPoint);
    std::swap(other._path, _path);
}
//----------------------------------------------------------------------------
bool Dirpath::Equals(const Dirpath& other) const {
    if (_mountingPoint != other._mountingPoint)
        return false;

    if (_path.size() != other._path.size())
        return false;

    const size_t count = _path.size();
    for (size_t i = 0; i < count; ++i)
    if (_path[i] != other._path[i])
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool Dirpath::Less(const Dirpath& other) const {
    if (_mountingPoint == other._mountingPoint) {
        const size_t count = _path.size() < other._path.size() ? _path.size() : other._path.size();
        for (size_t i = 0; i < count; ++i) {
            if (_path[i] < other._path[i])
                return true;
            else if (_path[i] != other._path[i])
                return false;
        }

        return _path.size() < other._path.size();
    }
    else {
        return _mountingPoint < other._mountingPoint;
    }
}
//----------------------------------------------------------------------------
size_t Dirpath::HashValue() const {
    if (_path.empty())
        return _mountingPoint.HashValue();

    size_t h = _mountingPoint.HashValue();
    for (const Dirname& dirname : _path)
        h = hash_value(h, dirname);

    return h;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
