#include "stdafx.h"

#include "Dirpath.h"

#include "Dirname.h"
#include "FileSystemTrie.h"
#include "MountingPoint.h"

#include "Allocator/Alloca.h"
#include "Container/Token.h"
#include "Memory/UniqueView.h"

#include <algorithm>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static const FileSystemNode *ParseDirpath_(const FileSystem::char_type *cstr, size_t length) {
    Assert(0 == length || cstr);
    if (nullptr == cstr || 0 == length)
        return nullptr;

    static const FileSystem::char_type *sep = L"\\/";
    STACKLOCAL_POD_STACK(FileSystemToken, path, Dirpath::MaxDepth);

    BasicStringSlice<FileSystem::char_type> slice;
    while (Split(&cstr, &length, sep, slice)) {
        if (slice.empty()) // ex: "toto//file.ext" -> 1 empty slice at "//"
            continue;

        path.Push(slice);
    }

    return FileSystemPath::Instance().GetOrCreate(path.MakeView());
}
//----------------------------------------------------------------------------
static const FileSystemNode *DirpathNode_(const MountingPoint& mountingPoint, const MemoryView<const Dirname>& path) {
    size_t count = path.size();
    if (!mountingPoint.empty())
        ++count;

    if (0 == count)
        return nullptr;

    STACKLOCAL_POD_STACK(FileSystemToken, tokens, count);

    if (!mountingPoint.empty())
        tokens.Push(mountingPoint);

    for (const Dirname& dirname : path)
        tokens.Push(dirname);

    return FileSystemPath::Instance().GetOrCreate(tokens.MakeView());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Dirpath::Dirpath() {}
//----------------------------------------------------------------------------
Dirpath::~Dirpath() {}
//----------------------------------------------------------------------------
Dirpath::Dirpath(Dirpath&& rvalue)
:   _path(std::move(rvalue._path)) {}
//----------------------------------------------------------------------------
Dirpath& Dirpath::operator =(Dirpath&& rvalue) {
    _path = std::move(rvalue._path);
    return *this;
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const Dirpath& other)
:   _path(other._path) {}
//----------------------------------------------------------------------------
Dirpath& Dirpath::operator =(const Dirpath& other) {
    _path = other._path;
    return *this;
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const Core::MountingPoint& mountingPoint, const MemoryView<const Dirname>& path) {
    _path = DirpathNode_(mountingPoint, path);
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const Dirpath& other, const Dirname& append) {
    _path = FileSystemPath::Instance().Concat(other._path, append);
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const Dirpath& other, const MemoryView<const Dirname>& append) {
    _path = FileSystemPath::Instance().Concat(other._path, append.Cast<const FileSystemToken>());
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const FileSystem::char_type *content) {
    _path = ParseDirpath_(content, Length(content));
}
//----------------------------------------------------------------------------
Dirpath& Dirpath::operator =(const FileSystem::char_type *content) {
    _path = ParseDirpath_(content, Length(content));
    return *this;
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const FileSystem::char_type* content, size_t length) {
    _path = ParseDirpath_(content, length);
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(const BasicStringSlice<FileSystem::char_type>& slice) {
    _path = ParseDirpath_(slice.begin(), slice.size());
}
//----------------------------------------------------------------------------
Dirpath& Dirpath::operator =(const BasicStringSlice<FileSystem::char_type>& slice) {
    _path = ParseDirpath_(slice.begin(), slice.size());
    return *this;
}
//----------------------------------------------------------------------------
Dirpath::Dirpath(std::initializer_list<const FileSystem::char_type *> path)  {

    STACKLOCAL_POD_STACK(FileSystemToken, tokens, path.size());

    for (const FileSystem::char_type *wcstr : path) {
        tokens.Push(wcstr);
        Assert(!tokens.Peek()->empty());
    }

    _path = FileSystemPath::Instance().GetOrCreate(tokens.MakeView());
    Assert(_path);
}
//----------------------------------------------------------------------------
size_t Dirpath::Depth() const {
    return (nullptr == _path) ? 0 : _path->Depth();
}
//----------------------------------------------------------------------------
Core::MountingPoint Dirpath::MountingPoint() const {
    if (nullptr == _path)
        return Core::MountingPoint();

    const FileSystemNode* pparent = FileSystemPath::Instance().RootNode(_path);
    Assert(pparent);

    if (L':' != pparent->Token().MakeView().back())
        return Core::MountingPoint();

    return Core::MountingPoint(pparent->Token());
}
//----------------------------------------------------------------------------
Core::Dirname Dirpath::LastDirname() const {
    if (nullptr == _path)
        return Core::Dirname();

    const FileSystemToken& token = _path->Token();
    if (L':' == token.MakeView().back())
        return Core::Dirname();

    return Core::Dirname(token);
}
//----------------------------------------------------------------------------
size_t Dirpath::ExpandPath(Core::MountingPoint& mountingPoint, const MemoryView<Dirname>& dirnames) const {
    if (nullptr == _path)
        return 0;

    STACKLOCAL_POD_ARRAY(FileSystemToken, tokens, _path->Depth());
    const size_t k = FileSystemPath::Instance().Expand(tokens.Pointer(), tokens.size(), _path);
    if (0 == k)
        return 0;
    Assert(_path->Depth() == k);

    size_t beg = 0;
    if (L':' == tokens.front().MakeView().back())
        mountingPoint = tokens[beg++];

    for (size_t i = 0; beg + i < k; ++i)
        dirnames[i] = tokens[beg + i];

    return (k -  beg);
}
//----------------------------------------------------------------------------
bool Dirpath::HasMountingPoint() const {
    return !MountingPoint().empty();
}
//----------------------------------------------------------------------------
void Dirpath::Concat(const Dirname& append) {
    Assert(!append.empty());
    const FileSystemToken *ptoken = &append;
    _path = FileSystemPath::Instance().Concat(_path, MakeView(ptoken, ptoken + 1));
}
//----------------------------------------------------------------------------
void Dirpath::Concat(const MemoryView<const Dirname>& path) {
    Assert(!path.empty());
    _path = FileSystemPath::Instance().Concat(_path, path.Cast<const FileSystemToken>());
}
//----------------------------------------------------------------------------
void Dirpath::Concat(const FileSystem::char_type *cstr) {
    Assert(cstr);
    return Concat(MemoryView<const FileSystem::char_type>(cstr, Length(cstr)) );
}
//----------------------------------------------------------------------------
void Dirpath::Concat(const MemoryView<const FileSystem::char_type>& strview) {
    Assert(strview.Pointer());

    const FileSystem::char_type *separators = FileSystem::Separators();

    size_t length = strview.size();
    const FileSystem::char_type *cstr = strview.Pointer();

    FileSystemTrie& trie = FileSystemPath::Instance();
    MemoryView<const FileSystem::char_type> slice;
    while (Split(&cstr, &length, separators, slice))
        _path = trie.Concat(_path, FileSystemToken(slice) );
}
//----------------------------------------------------------------------------
String Dirpath::ToString() const {
    STACKLOCAL_OCSTRSTREAM(oss, 2048);
    oss << *this;
    return Core::ToString(oss.MakeView());
}
//----------------------------------------------------------------------------
WString Dirpath::ToWString() const {
    STACKLOCAL_WOCSTRSTREAM(oss, 2048);
    oss << *this;
    return Core::ToWString(oss.MakeView());
}
//----------------------------------------------------------------------------
size_t Dirpath::ToCStr(char *dst, size_t capacity) const {
    OCStrStream oss(dst, capacity);
    oss << *this;
    return static_cast<size_t>(oss.tellp());
}
//----------------------------------------------------------------------------
size_t Dirpath::ToWCStr(wchar_t *dst, size_t capacity) const {
    WOCStrStream oss(dst, capacity);
    oss << *this;
    return static_cast<size_t>(oss.tellp());
}
//----------------------------------------------------------------------------
void Dirpath::Swap(Dirpath& other) {
    std::swap(other._path, _path);
}
//----------------------------------------------------------------------------
bool Dirpath::Less(const Dirpath& other) const {
    if (_path == other._path || nullptr == other._path)
        return false;
    else if (nullptr == _path)
        return true;

    const auto& fsp = FileSystemPath::Instance();

    STACKLOCAL_POD_ARRAY(FileSystemToken, p0, _path->Depth() );
    const size_t k0 = fsp.Expand(p0.Pointer(), p0.size(), _path);
    Assert(_path->Depth() == k0 );

    STACKLOCAL_POD_ARRAY(FileSystemToken, p1, other._path->Depth() );
    const size_t k1 = fsp.Expand(p1.Pointer(), p1.size(), other._path);
    Assert(other._path->Depth() == k1 );

    const size_t k = (k0 < k1) ? k0 : k1;
    for (size_t i = 0; i < k; ++i) {
        if (p0[i] < p1[i])
            return true;
        else if (p0[i] != p1[i])
            return false;
    }

    return (k0 < k1); // both equals, shortest wins
}
//----------------------------------------------------------------------------
size_t Dirpath::HashValue() const {
    return (_path) ? _path->HashValue() : 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
