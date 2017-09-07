#include "stdafx.h"

#include "Filename.h"

#include "Dirname.h"

#include "Container/Hash.h"
#include "IO/Stream.h"
#include "IO/String.h"
#include "Memory/UniqueView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ParseFilename_(const FileSystem::FStringView& str, FDirpath& dirpath, FBasename& basename) {
    dirpath = FDirpath();
    basename = FBasename();

    if (str.empty())
        return false;

    const auto it = str.FindIfR([](FileSystem::char_type ch) {
        return (ch == FileSystem::Separator || ch == FileSystem::AltSeparator );
    });

    if (str.rend() == it) {
        basename = str;
    }
    else {
        dirpath = FDirpath(str.CutBefore(it));
        basename = str.CutStartingAt(it - 1);
    }

    return true;
}
//----------------------------------------------------------------------------
static bool AppendRelname_(FDirpath& dirpath, FBasename& basename, const FileSystem::FStringView& relname) {
    Assert(basename.empty());
    if (relname.empty())
        return false;

    const auto it = relname.FindIfR([](FileSystem::char_type ch) {
        return (ch == FileSystem::Separator || ch == FileSystem::AltSeparator );
    });

    if (relname.rend() == it) {
        basename = relname;
    }
    else {
        dirpath.Concat(relname.CutBefore(it));
        basename = relname.CutStartingAt(it - 1);
    }

    return false;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFilename::FFilename(FDirpath&& dirpath, FBasename&& basename)
:   _dirpath(std::move(dirpath)), _basename(std::move(basename)) {}
//----------------------------------------------------------------------------
FFilename::FFilename(const FDirpath& dirpath, const FBasename& basename)
:   _dirpath(dirpath), _basename(basename) {}
//----------------------------------------------------------------------------
FFilename::FFilename(const FDirpath& dirpath, const FileSystem::FStringView& relfilename)
:   _dirpath(dirpath) {
    if (!AppendRelname_(_dirpath, _basename, relfilename))
        AssertNotReached();
}
//----------------------------------------------------------------------------
FFilename::FFilename(FFilename&& rvalue)
:   _dirpath(std::move(rvalue._dirpath)),
    _basename(std::move(rvalue._basename)) {}
//----------------------------------------------------------------------------
FFilename& FFilename::operator =(FFilename&& rvalue) {
    _dirpath = std::move(rvalue._dirpath);
    _basename = std::move(rvalue._basename);
    return *this;
}
//----------------------------------------------------------------------------
FFilename::FFilename(const FFilename& other)
:   _dirpath(other._dirpath),
    _basename(other._basename) {}
//----------------------------------------------------------------------------
FFilename& FFilename::operator =(const FFilename& other) {
    _dirpath = other._dirpath;
    _basename = other._basename;
    return *this;
}
//----------------------------------------------------------------------------
FFilename::FFilename(const FileSystem::FStringView& content) {
    if (!ParseFilename_(content, _dirpath, _basename))
        AssertNotReached();
}
//----------------------------------------------------------------------------
FFilename& FFilename::operator =(const FileSystem::FStringView& content) {
    if (!ParseFilename_(content, _dirpath, _basename))
        AssertNotReached();
    return *this;
}
//----------------------------------------------------------------------------
void FFilename::SetMountingPoint(const FMountingPoint& mountingPoint) {
    FMountingPoint oldMountingPoint;
    STACKLOCAL_POD_ARRAY(FDirname, dirnames, _dirpath.Depth());
    const size_t k = _dirpath.ExpandPath(oldMountingPoint, dirnames);
    Assert(_dirpath.Depth() >= k);
    UNUSED(k);
    _dirpath = FDirpath(mountingPoint, dirnames.CutBeforeConst(k));
}
//----------------------------------------------------------------------------
FFilename FFilename::WithReplacedMountingPoint(const FMountingPoint& mountingPoint) const {
    FFilename cpy(*this);
    cpy.SetMountingPoint(mountingPoint);
    return cpy;
}
//----------------------------------------------------------------------------
void FFilename::AppendBasename(const FileSystem::FStringView& basenameNoExt) {
    FWString newBasenameNoExt(Core::ToWString(_basename.BasenameNoExt().MakeView()));
    newBasenameNoExt.append(basenameNoExt.begin(), basenameNoExt.end());
    _basename.SetBasenameNoExt(newBasenameNoExt);
}
//----------------------------------------------------------------------------
FFilename FFilename::WithAppendBasename(const FileSystem::FStringView& basenameNoExt) const {
    FFilename cpy(*this);
    cpy.AppendBasename(basenameNoExt);
    return cpy;
}
//----------------------------------------------------------------------------
FFilename FFilename::WithReplacedExtension(const FExtname& ext) const {
    FFilename cpy(*this);
    cpy.ReplaceExtension(ext);
    return cpy;
}
//----------------------------------------------------------------------------
bool FFilename::Absolute(FFilename* absolute, const FDirpath& origin) const {
    Assert(absolute);
    FDirpath dirpath;
    if (false == FDirpath::Absolute(&dirpath, origin, _dirpath))
        return false;
    *absolute = FFilename(dirpath, _basename);
    return true;
}
//----------------------------------------------------------------------------
FFilename FFilename::Absolute(const FDirpath& origin) const {
    FFilename result;
    if (not Absolute(&result, origin))
        AssertNotReached();
    return result;
}
//----------------------------------------------------------------------------
bool FFilename::Normalize(FFilename* normalized) const {
    Assert(normalized);
    FDirpath dirpath;
    if (false == FDirpath::Normalize(&dirpath, _dirpath))
        return false;
    *normalized = FFilename(dirpath, _basename);
    return true;
}
//----------------------------------------------------------------------------
FFilename FFilename::Normalized() const {
    FFilename result;
    if (not Normalize(&result))
        AssertNotReached();
    return result;
}
//----------------------------------------------------------------------------
bool FFilename::Relative(FFilename* relative, const FDirpath& origin) const {
    Assert(relative);
    FDirpath dirpath;
    if (false == FDirpath::Relative(&dirpath, origin, _dirpath))
        return false;
    *relative = FFilename(dirpath, _basename);
    return true;
}
//----------------------------------------------------------------------------
FFilename FFilename::Relative(const FDirpath& origin) const {
    FFilename result;
    if (not Relative(&result, origin))
        AssertNotReached();
    return result;
}
//----------------------------------------------------------------------------
size_t FFilename::HashValue() const {
    return hash_tuple(_dirpath.HashValue(), _basename.HashValue());
}
//----------------------------------------------------------------------------
FString FFilename::ToString() const {
    STACKLOCAL_OCSTRSTREAM(oss, 1024);
    oss << *this;
    return Core::ToString(oss.MakeView());
}
//----------------------------------------------------------------------------
FWString FFilename::ToWString() const {
    STACKLOCAL_WOCSTRSTREAM(oss, 1024);
    oss << *this;
    return Core::ToWString(oss.MakeView());
}
//----------------------------------------------------------------------------
size_t FFilename::ToCStr(char *dst, size_t capacity) const {
    FOCStrStream oss(dst, capacity);
    oss << *this;
    return static_cast<size_t>(oss.tellp());
}
//----------------------------------------------------------------------------
size_t FFilename::ToWCStr(wchar_t *dst, size_t capacity) const {
    FWOCStrStream oss(dst, capacity);
    oss << *this;
    return static_cast<size_t>(oss.tellp());
}
//----------------------------------------------------------------------------
void FFilename::Swap(FFilename& other) {
    swap(other._dirpath, _dirpath);
    swap(other._basename, _basename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
