#include "stdafx.h"

#include "IO/Filename.h"

#include "IO/Dirname.h"

#include "Container/Hash.h"
#include "Diagnostic/DebugFunction.h"
#include "IO/StreamProvider.h"
#include "IO/StringBuilder.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryProvider.h"
#include "Memory/UniqueView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void ParseFilename_(const FileSystem::FStringView& str, FDirpath& dirpath, FBasename& basename) {
    dirpath = FDirpath();
    basename = FBasename();

    if (str.empty())
        return;

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

    return true;
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
FFilename::FFilename(FDirpath&& dirpath, FBasenameNoExt&& basenameNoExt, FExtname&& extname)
:   _dirpath(std::move(dirpath)), _basename(std::move(basenameNoExt), std::move(extname)) {}
//----------------------------------------------------------------------------
FFilename::FFilename(const FDirpath& dirpath, const FBasenameNoExt& basenameNoExt, const FExtname& extname)
:   _dirpath(dirpath), _basename(basenameNoExt, extname) {}
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
//----------------------------------------------------------------------------}
FFilename::FFilename(const FileSystem::FString& content)
:   FFilename(content.MakeView())
{}
//----------------------------------------------------------------------------
FFilename& FFilename::operator =(const FileSystem::FString& content) {
    return operator =(content.MakeView());
}
//----------------------------------------------------------------------------
FFilename::FFilename(const FileSystem::FStringView& content) {
    ParseFilename_(content, _dirpath, _basename);
}
//----------------------------------------------------------------------------
FFilename& FFilename::operator =(const FileSystem::FStringView& content) {
    ParseFilename_(content, _dirpath, _basename);
    return (*this);
}
//----------------------------------------------------------------------------
void FFilename::SetMountingPoint(const FMountingPoint& mountingPoint) {
    FMountingPoint oldMountingPoint;
    STACKLOCAL_ASSUMEPOD_ARRAY(FDirname, dirnames, _dirpath.Depth());
    const size_t k = _dirpath.ExpandPath(oldMountingPoint, dirnames);
    Assert(_dirpath.Depth() >= k);
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
    FWStringBuilder oss;
    oss.reserve(_basename.BasenameNoExt().size() + basenameNoExt.size());
    oss << _basename.BasenameNoExt() << basenameNoExt;
    _basename.SetBasenameNoExt(oss.ToString().MakeView());
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
FFilename FFilename::RemoveExtname() const {
    return FFilename(_dirpath, _basename.RemoveExtname());
}
//----------------------------------------------------------------------------
size_t FFilename::HashValue() const {
    return hash_tuple(_dirpath, _basename);
}
//----------------------------------------------------------------------------
FString FFilename::ToString() const {
    FStringBuilder oss;
    oss << *this;
    return oss.ToString();
}
//----------------------------------------------------------------------------
FWString FFilename::ToWString() const {
    FWStringBuilder oss;
    oss << *this;
    return oss.ToString();
}
//----------------------------------------------------------------------------
FStringView FFilename::ToCStr(const TMemoryView<char>& dst) const {
    FFixedSizeTextWriter oss(dst);
    oss << *this << Eos;
    return oss.Written().ShiftBack();
}
//----------------------------------------------------------------------------
FWStringView FFilename::ToWCStr(const TMemoryView<wchar_t>& dst) const {
    FWFixedSizeTextWriter oss(dst);
    oss << *this << Eos;
    return oss.Written().ShiftBack();
}
//----------------------------------------------------------------------------
void FFilename::Swap(FFilename& other) {
    swap(other._dirpath, _dirpath);
    swap(other._basename, _basename);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FFilename& filename) {
    if (filename.empty())
        return oss;
    if (!filename.Dirpath().empty())
        oss << filename.Dirpath();
    return oss << filename.Basename();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FFilename& filename) {
    if (filename.empty())
        return oss;
    if (!filename.Dirpath().empty())
        oss << filename.Dirpath();
    return oss << filename.Basename();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Used in natvis/debuggers for printing FFilename content
DEBUG_FUNCTION(PPE_CORE_API, DebugPrintFilename, FWStringView, (const FFilename& filename), {
    static wchar_t GDebugBuffer[FileSystem::MaxPathLength];
    return filename.ToWCStr(GDebugBuffer);
})
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE