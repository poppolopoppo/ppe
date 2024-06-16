// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IO/Dirpath.h"

#include "IO/Dirname.h"
#include "IO/Filename.h"
#include "IO/FileSystemTrie.h"
#include "IO/MountingPoint.h"

#include "Allocator/Alloca.h"
#include "Container/Token.h"
#include "Container/Stack.h"
#include "Diagnostic/DebugFunction.h"
#include "IO/ConstNames.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryProvider.h"
#include "Memory/UniqueView.h"

#include <algorithm>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::is_pod_v<FDirpath>);
//----------------------------------------------------------------------------
NODISCARD static bool NormalizePath_(size_t* plength, const TMemoryView<FDirname>& dirnames) {
    Assert(plength);

    const FDirname dotdot = FFSConstNames::DotDot();
    if (dirnames.front() == dotdot)
        return false;

    size_t insert = 0;
    forrange(i, 0, dirnames.size()) {
        if (dirnames[i] != dotdot)
            dirnames[insert++] = dirnames[i];
        else if (0 < insert)
            --insert;
        else
            return false; // invalid path, too many /../../../..
    }

    *plength = insert;
    return true;
}
//----------------------------------------------------------------------------
NODISCARD static const FFileSystemNode *ParseDirpath_(const FileSystem::FStringView& str) {
    Assert(not str.empty());

    STACKLOCAL_ASSUMEPOD_STACK(FFileSystemToken, path, FDirpath::MaxDepth);

    const FileSystem::FStringView separators = FileSystem::Separators();

    FileSystem::FStringView src = str;
    FileSystem::FStringView slice;
    while (Split(src, separators, slice)) {
        if (not slice.empty()) // ex: "toto//file.ext" -> 1 empty slice at "//"
            path.Push(slice);
    }

    return FFileSystemTrie::Get().GetOrCreate(path.MakeView());
}
//----------------------------------------------------------------------------
NODISCARD static const FFileSystemNode *DirpathNode_(const FMountingPoint& mountingPoint, const TMemoryView<const FDirname>& path) {
    size_t count = path.size();
    if (!mountingPoint.empty())
        ++count;

    if (0 == count)
        return nullptr;

    STACKLOCAL_ASSUMEPOD_STACK(FFileSystemToken, tokens, count);

    if (!mountingPoint.empty())
        tokens.Push(mountingPoint);

    for (const FDirname& dirname : path)
        tokens.Push(dirname);

    return FFileSystemTrie::Get().GetOrCreate(tokens.MakeView());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDirpath FDirpath::FromTokens(const TMemoryView<const FFileSystemToken>& tokens) {
    FDirpath result;
    result._path = FFileSystemTrie::Get().GetOrCreate(tokens);
    return result;
}
//----------------------------------------------------------------------------
FDirpath::FDirpath(const FMountingPoint& mountingPoint, const TMemoryView<const FDirname>& path) {
    _path = DirpathNode_(mountingPoint, path);
}
//----------------------------------------------------------------------------
FDirpath::FDirpath(const FDirpath& other, const FDirname& append) {
    _path = FFileSystemTrie::Get().Concat(other._path, append);
}
//----------------------------------------------------------------------------
FDirpath::FDirpath(const FDirpath& other, const TMemoryView<const FDirname>& append) {
    _path = FFileSystemTrie::Get().Concat(other._path, append.Cast<const FFileSystemToken>());
}
//----------------------------------------------------------------------------
FDirpath::FDirpath(const FileSystem::FString& str)
:   FDirpath(str.MakeView())
{}
//----------------------------------------------------------------------------
FDirpath& FDirpath::operator =(const FileSystem::FString& str) {
    return operator =(str.MakeView());
}
//----------------------------------------------------------------------------
FDirpath::FDirpath(const FileSystem::FStringView& content) {
    if (Likely(not content.empty()))
        _path = ParseDirpath_(content);
}
//----------------------------------------------------------------------------
FDirpath& FDirpath::operator =(const FileSystem::FStringView& content) {
    if (Likely(not content.empty()))
        _path = ParseDirpath_(content);

    return (*this);
}
//----------------------------------------------------------------------------
FDirpath::FDirpath(std::initializer_list<const FileSystem::char_type *> path)  {
    STACKLOCAL_ASSUMEPOD_STACK(FFileSystemToken, tokens, path.size());

    for (const FileSystem::char_type *wcstr : path) {
        tokens.Push(MakeCStringView(wcstr));
        Assert_NoAssume(not tokens.Peek()->empty());
    }

    _path = FFileSystemTrie::Get().GetOrCreate(tokens.MakeView());
    Assert(_path);
}
//----------------------------------------------------------------------------
size_t FDirpath::Depth() const {
    return (nullptr == _path) ? 0 : _path->Depth();
}
//----------------------------------------------------------------------------
FMountingPoint FDirpath::MountingPoint() const {
    if (nullptr == _path)
        return Default;

    if (not EndsWith(_path->MakeView().front(), L":"_view))
        return Default;

    return { _path->MakeView().front() };
}
//----------------------------------------------------------------------------
FDirname FDirpath::LastDirname() const {
    if (nullptr == _path)
        return Default;

    if (EndsWith(_path->MakeView().back(), L":"_view))
        return Default;

    return { _path->MakeView().back() };
}
//----------------------------------------------------------------------------
size_t FDirpath::ExpandPath(FMountingPoint& mountingPoint, const TMemoryView<FDirname>& dirnames) const {
    if (nullptr == _path)
        return 0;

    FFileSystemTrie::FPath path = _path->MakeView();
    if (_path->HasMountingPoint()) {
        mountingPoint = path.front();
        path = path.ShiftFront();
    }

    path.CopyTo(dirnames.CutBefore(path.size()));
    return path.size();
}
//----------------------------------------------------------------------------
void FDirpath::AssignTokens(const TMemoryView<const FFileSystemToken>& tokens) {
    _path = FFileSystemTrie::Get().GetOrCreate(tokens);
}
//----------------------------------------------------------------------------
TMemoryView<const FFileSystemToken> FDirpath::ExpandTokens() const {
    if (Likely(!!_path))
        return _path->MakeView();

    return Default;
}
//----------------------------------------------------------------------------
bool FDirpath::HasMountingPoint() const {
    return (_path && _path->HasMountingPoint());
}
//----------------------------------------------------------------------------
bool FDirpath::IsSubdirectory(const FDirpath& other) const {
    Assert_NoAssume(not empty());
    Assert_NoAssume(not other.empty());

    return (_path->IsChildOf(*other._path));
}
//----------------------------------------------------------------------------
void FDirpath::Concat(const FDirname& append) {
    Assert(!append.empty());
    const FFileSystemToken *ptoken = &append;
    _path = FFileSystemTrie::Get().Concat(_path, MakeView(ptoken, ptoken + 1));
}
//----------------------------------------------------------------------------
void FDirpath::Concat(const TMemoryView<const FDirname>& path) {
    Assert(!path.empty());
    _path = FFileSystemTrie::Get().Concat(_path, path.Cast<const FFileSystemToken>());
}
//----------------------------------------------------------------------------
void FDirpath::Concat(const FileSystem::char_type *cstr) {
    Assert(cstr);
    return Concat(TMemoryView<const FileSystem::char_type>(cstr, Length(cstr)) );
}
//----------------------------------------------------------------------------
void FDirpath::Concat(const TBasicStringView<FileSystem::char_type>& strview) {
    Assert(strview.Pointer());

    FFileSystemTrie& trie = FFileSystemTrie::Get();

    const TBasicStringView<FileSystem::char_type> separators = FileSystem::Separators();

    TBasicStringView<FileSystem::char_type> src(strview);
    TBasicStringView<FileSystem::char_type> slice;
    while (Split(src, separators, slice))
        _path = trie.Concat(_path, FFileSystemToken(slice) );
}
//----------------------------------------------------------------------------
FString FDirpath::ToString() const {
    FStringBuilder oss;
    oss << *this;
    return oss.ToString();
}
//----------------------------------------------------------------------------
FWString FDirpath::ToWString() const {
    FWStringBuilder oss;
    oss << *this;
    return oss.ToString();
}
//----------------------------------------------------------------------------
FStringView FDirpath::ToCStr(const TMemoryView<char>& dst) const {
    FFixedSizeTextWriter oss(dst);
    oss << *this << Eos;
    return oss.Written().ShiftBack();
}
//----------------------------------------------------------------------------
FWStringView FDirpath::ToWCStr(const TMemoryView<wchar_t>& dst) const {
    FWFixedSizeTextWriter oss(dst);
    oss << *this << Eos;
    return oss.Written().ShiftBack();
}
//----------------------------------------------------------------------------
void FDirpath::Clear() NOEXCEPT {
    _path.reset();
}
//----------------------------------------------------------------------------
void FDirpath::Swap(FDirpath& other) NOEXCEPT {
    std::swap(other._path, _path);
}
//----------------------------------------------------------------------------
bool FDirpath::Less(const FDirpath& other) const {
    if (_path == other._path || nullptr == other._path)
        return false;
    if (nullptr == _path)
        return true;

    return _path->Less(*other._path);
}
//----------------------------------------------------------------------------
size_t FDirpath::HashValue() const {
    return (_path ? size_t(_path->HashValue()) : 0);
}
//----------------------------------------------------------------------------
bool FDirpath::Absolute(FDirpath* absolute, const FDirpath& origin, const FDirpath& relative) {
    Assert(absolute);
    Assert_NoAssume(origin.HasMountingPoint());
    Assert_NoAssume(not relative.HasMountingPoint());

    STACKLOCAL_ASSUMEPOD_ARRAY(FDirname, dirnames, origin.Depth()+relative.Depth());

    FMountingPoint origin_mp, relative_mp;
    const size_t origin_s = origin.ExpandPath(origin_mp, dirnames.SubRange(0, origin.Depth()));
    const size_t relative_s = relative.ExpandPath(relative_mp, dirnames.SubRange(origin_s, relative.Depth()));

    Assert(origin.Depth() >= origin_s);
    Assert(relative.Depth() >= relative_s);

    size_t length = 0;
    if (not NormalizePath_(&length, dirnames.SubRange(0, origin_s+relative_s)))
        return false;

    Assert((origin_s+relative_s) >= length);
    *absolute = FDirpath(origin_mp, dirnames.SubRange(0, length));
    return true;
}
//----------------------------------------------------------------------------
bool FDirpath::Normalize(FDirpath* normalized, const FDirpath& path) {
    Assert(normalized);

    if (path.empty()) {
        *normalized = path;
        return true;
    }

    FMountingPoint mountingPoint;
    STACKLOCAL_ASSUMEPOD_ARRAY(FDirname, dirnames, path.Depth());
    const size_t n = path.ExpandPath(mountingPoint, dirnames);
    Assert(path.Depth() >= n);

    size_t length = 0;
    if (not NormalizePath_(&length, dirnames.SubRange(0, n)))
        return false;

    Assert(length <= n);
    *normalized = FDirpath(mountingPoint, dirnames.SubRangeConst(0, length));
    return true;
}
//----------------------------------------------------------------------------
bool FDirpath::Relative(FDirpath* relative, const FDirpath& origin, const FDirpath& other) {
    Assert(relative);
    Assert_NoAssume(origin.HasMountingPoint());
    Assert_NoAssume(other.HasMountingPoint());

    if (origin.MountingPoint() != other.MountingPoint())
        return false;

    if (0 == origin.Depth() || 0 == other.Depth()) {
        *relative = (0 == origin.Depth() ? other : origin);
        return true;
    }

    STACKLOCAL_ASSUMEPOD_ARRAY(FDirname, dirnames, origin.Depth()+other.Depth());
    TMemoryView<FDirname> origin_dirs = dirnames.SubRange(0, origin.Depth());
    TMemoryView<FDirname> other_dirs = dirnames.SubRange(origin.Depth(), other.Depth());

    FMountingPoint origin_mp, other_mp;
    size_t origin_s = origin.ExpandPath(origin_mp, origin_dirs);
    size_t other_s = other.ExpandPath(other_mp, other_dirs);

    if (false == NormalizePath_(&origin_s, origin_dirs.SubRange(0, origin_s)) ||
        false == NormalizePath_(&other_s, other_dirs.SubRange(0, other_s)) )
        return false;

    Assert(origin.Depth() >= origin_s);
    Assert(other.Depth() >= other_s);

    origin_dirs = origin_dirs.SubRange(0, origin_s);
    other_dirs = other_dirs.SubRange(0, other_s);

    size_t begin = 0;
    while ( begin < origin_s &&
            begin < other_s &&
            origin_dirs[begin] == other_dirs[begin] )
        begin++;

    const FDirname dotdot = FFSConstNames::DotDot();
    STACKLOCAL_ASSUMEPOD_STACK(FDirname, relative_dirs, (origin_s-begin)+(other_s-begin));

    forrange(i, begin, origin_s)
        relative_dirs.Push(dotdot);

    forrange(i, begin, other_s)
        relative_dirs.Push(other_dirs[i]);

    Assert(relative_dirs.size() == relative_dirs.capacity());

    *relative = FDirpath(FMountingPoint(), relative_dirs.MakeView());
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NODISCARD FDirpath operator /(const FDirpath& lhs, const FDirname& rhs) {
    return { lhs, rhs };
}
//----------------------------------------------------------------------------
NODISCARD FFilename operator /(const FDirpath& lhs, const FBasename& basename) {
    return { lhs, basename };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const PPE::FDirpath& dirpath) {
    STATIC_ASSERT(static_cast<char>(FileSystem::NormalizedSeparator) == '/');
    for (const FFileSystemToken& token : dirpath.ExpandTokens()) {
        oss << token << static_cast<char>(FileSystem::NormalizedSeparator);
    }
    return oss;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const PPE::FDirpath& dirpath) {
    STATIC_ASSERT(static_cast<wchar_t>(FileSystem::NormalizedSeparator) == L'/');
    for (const FFileSystemToken& token : dirpath.ExpandTokens()) {
        oss << token << static_cast<wchar_t>(FileSystem::NormalizedSeparator);
    }
    return oss;
}
//----------------------------------------------------------------------------
bool operator >>(const FStringConversion& iss, FDirpath* dirpath) {
    return (FWStringConversion(UTF_8_TO_WCHAR(iss.Input)) >> dirpath);
}
//----------------------------------------------------------------------------
PPE_CORE_API bool operator >>(const FWStringConversion& iss, FDirpath* dirpath) {
    if (Likely(not iss.Input.empty())) {
        const auto* const fsNode = ParseDirpath_(iss.Input);
        *dirpath = FDirpath{ fsNode };
        return (!!fsNode || iss.empty());
    }
    *dirpath = FDirpath();
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Used in natvis/debuggers for printing FDirpath content
DEBUG_FUNCTION(PPE_CORE_API, DebugPrintDirpath, FWStringView, (const FDirpath& dirpath), {
    wchar_t tmp[FileSystem::MaxPathLength];
    return dirpath.ToWCStr(tmp);
})
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
