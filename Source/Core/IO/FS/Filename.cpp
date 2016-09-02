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
static bool ParseFilename_(const FileSystem::StringView& str, Dirpath& dirpath, Basename& basename) {
    dirpath = Dirpath();
    basename = Basename();

    if (str.empty())
        return false;

    const auto it = str.FindIfR([](FileSystem::char_type ch) {
        return (ch == FileSystem::Separator || ch == FileSystem::AltSeparator );
    });

    if (str.rend() == it) {
        basename = str;
    }
    else {
        dirpath = Dirpath(str.CutBefore(it));
        basename = str.CutStartingAt(it - 1);
    }

    return true;
}
//----------------------------------------------------------------------------
static bool AppendRelname_(Dirpath& dirpath, Basename& basename, const FileSystem::StringView& relname) {
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
Filename::Filename(Core::Dirpath&& dirpath, Core::Basename&& basename)
:   _dirpath(std::move(dirpath)), _basename(std::move(basename)) {}
//----------------------------------------------------------------------------
Filename::Filename(const Core::Dirpath& dirpath, const Core::Basename& basename)
:   _dirpath(dirpath), _basename(basename) {}
//----------------------------------------------------------------------------
Filename::Filename(const Core::Dirpath& dirpath, const FileSystem::StringView& relfilename)
:   _dirpath(dirpath) {
    if (!AppendRelname_(_dirpath, _basename, relfilename))
        AssertNotReached();
}
//----------------------------------------------------------------------------
Filename::Filename(Filename&& rvalue)
:   _dirpath(std::move(rvalue._dirpath)),
    _basename(std::move(rvalue._basename)) {}
//----------------------------------------------------------------------------
Filename& Filename::operator =(Filename&& rvalue) {
    _dirpath = std::move(rvalue._dirpath);
    _basename = std::move(rvalue._basename);
    return *this;
}
//----------------------------------------------------------------------------
Filename::Filename(const Filename& other)
:   _dirpath(other._dirpath),
    _basename(other._basename) {}
//----------------------------------------------------------------------------
Filename& Filename::operator =(const Filename& other) {
    _dirpath = other._dirpath;
    _basename = other._basename;
    return *this;
}
//----------------------------------------------------------------------------
Filename::Filename(const FileSystem::StringView& content) {
    if (!ParseFilename_(content, _dirpath, _basename))
        AssertNotReached();
}
//----------------------------------------------------------------------------
Filename& Filename::operator =(const FileSystem::StringView& content) {
    if (!ParseFilename_(content, _dirpath, _basename))
        AssertNotReached();
    return *this;
}
//----------------------------------------------------------------------------
void Filename::SetMountingPoint(const Core::MountingPoint& mountingPoint) {
    Core::MountingPoint oldMountingPoint;
    STACKLOCAL_POD_ARRAY(Dirname, dirnames, _dirpath.Depth());
    const size_t k = _dirpath.ExpandPath(oldMountingPoint, dirnames);
    Assert(_dirpath.Depth() == k);
    UNUSED(k);
    _dirpath = Core::Dirpath(mountingPoint, dirnames);
}
//----------------------------------------------------------------------------
Filename Filename::WithReplacedExtension(const Core::Extname& ext) const {
    Filename cpy(*this);
    cpy.ReplaceExtension(ext);
    return cpy;
}
//----------------------------------------------------------------------------
bool Filename::Absolute(Filename* absolute, const Core::Dirpath& origin) const {
    Assert(absolute);
    Core::Dirpath dirpath;
    if (false == Core::Dirpath::Absolute(&dirpath, origin, _dirpath))
        return false;
    *absolute = Filename(dirpath, _basename);
    return true;
}
//----------------------------------------------------------------------------
Filename Filename::Absolute(const Core::Dirpath& origin) const {
    Filename result;
    if (not Absolute(&result, origin))
        AssertNotReached();
    return result;
}
//----------------------------------------------------------------------------
bool Filename::Normalize(Filename* normalized) const {
    Assert(normalized);
    Core::Dirpath dirpath;
    if (false == Core::Dirpath::Normalize(&dirpath, _dirpath))
        return false;
    *normalized = Filename(dirpath, _basename);
    return true;
}
//----------------------------------------------------------------------------
Filename Filename::Normalized() const {
    Filename result;
    if (not Normalize(&result))
        AssertNotReached();
    return result;
}
//----------------------------------------------------------------------------
bool Filename::Relative(Filename* relative, const Core::Dirpath& origin) const {
    Assert(relative);
    Core::Dirpath dirpath;
    if (false == Core::Dirpath::Relative(&dirpath, origin, _dirpath))
        return false;
    *relative = Filename(dirpath, _basename);
    return true;
}
//----------------------------------------------------------------------------
Filename Filename::Relative(const Core::Dirpath& origin) const {
    Filename result;
    if (not Relative(&result, origin))
        AssertNotReached();
    return result;
}
//----------------------------------------------------------------------------
size_t Filename::HashValue() const {
    return hash_tuple(_dirpath.HashValue(), _basename.HashValue());
}
//----------------------------------------------------------------------------
String Filename::ToString() const {
    STACKLOCAL_OCSTRSTREAM(oss, 1024);
    oss << *this;
    return Core::ToString(oss.MakeView());
}
//----------------------------------------------------------------------------
WString Filename::ToWString() const {
    STACKLOCAL_WOCSTRSTREAM(oss, 1024);
    oss << *this;
    return Core::ToWString(oss.MakeView());
}
//----------------------------------------------------------------------------
size_t Filename::ToCStr(char *dst, size_t capacity) const {
    OCStrStream oss(dst, capacity);
    oss << *this;
    return static_cast<size_t>(oss.tellp());
}
//----------------------------------------------------------------------------
size_t Filename::ToWCStr(wchar_t *dst, size_t capacity) const {
    WOCStrStream oss(dst, capacity);
    oss << *this;
    return static_cast<size_t>(oss.tellp());
}
//----------------------------------------------------------------------------
void Filename::Swap(Filename& other) {
    swap(other._dirpath, _dirpath);
    swap(other._basename, _basename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
