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
static bool ParseBasename_(
    const wchar_t *cstr,
    size_t length,
    Basename& basename
    ) {
    if (!cstr || L'\0' == cstr[0])
        return false;

    const wchar_t *p = cstr + length - 1;
    for (; p != cstr && L'.' != *p; --p);

    if (p != cstr) {
        Assert(L'.' == *p);
        const BasenameNoExt basenameNoExt(cstr, p - cstr);
        const Extname extname(p, cstr + length - p);
        basename = Basename(basenameNoExt, extname);
    }
    else if (L'.' == *cstr && (1 == length || (2 == length && L'.' == cstr[1]))) {
        // '.' and '..' are forbiden
        return false;
    }
    else {
        basename = Basename(BasenameNoExt(cstr, length), Extname());
    }

    return true;
}
//----------------------------------------------------------------------------
static bool ParseFilename_(
    const wchar_t *cstr,
    size_t length,
    Dirpath& dirpath,
    Basename& basename
    ) {
    dirpath = Dirpath();
    basename = Basename();

    if (!cstr || 0 == length)
        return false;

    const wchar_t *basenamePtr = std::max(
        StrRChr(cstr, FileSystem::Separator),
        StrRChr(cstr, FileSystem::AltSeparator)
        );

    if (!basenamePtr)
        return ParseBasename_(cstr, length, basename);

    dirpath = Dirpath(cstr, basenamePtr - cstr);
    return ParseBasename_(basenamePtr + 1, length - (basenamePtr - cstr) - 1, basename);
}
//----------------------------------------------------------------------------
static bool AppendRelname_(Dirpath& dirpath, Basename& basename, const MemoryView<const FileSystem::char_type>& relname) {
    if (relname.empty())
        return false;

    const wchar_t *basenamePtr = std::max(
        StrRChr(relname.Pointer(), FileSystem::Separator),
        StrRChr(relname.Pointer(), FileSystem::AltSeparator)
        );

    if (!basenamePtr)
        return ParseBasename_(relname.Pointer(), relname.size(), basename);

    dirpath.Concat(relname.SubRange(0, basenamePtr - relname.Pointer()));
    return ParseBasename_(basenamePtr + 1, relname.size() - (basenamePtr - relname.Pointer()) - 1, basename);
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
Filename::Filename(const Core::Dirpath& dirpath, const FileSystem::char_type *relfilename) 
:   Filename(dirpath, MemoryView<const FileSystem::char_type>(relfilename, Length(relfilename)) ) {}
//----------------------------------------------------------------------------
Filename::Filename(const Core::Dirpath& dirpath, const FileSystem::char_type *relfilename, size_t length) 
:   Filename(dirpath, MemoryView<const FileSystem::char_type>(relfilename, length) ) {}
//----------------------------------------------------------------------------
Filename::Filename(const Core::Dirpath& dirpath, const MemoryView<const FileSystem::char_type>& relfilename) 
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
Filename::Filename(const FileSystem::char_type* content) {
    Assert(content);
    if (!ParseFilename_(content, Length(content), _dirpath, _basename))
        AssertNotReached();
}
//----------------------------------------------------------------------------
Filename& Filename::operator =(const FileSystem::char_type* content) {
    Assert(content);
    if (!ParseFilename_(content, Length(content), _dirpath, _basename))
        AssertNotReached();
    return *this;
}
//----------------------------------------------------------------------------
Filename::Filename(const FileSystem::char_type* content, size_t length) {
    Assert(content);
    if (!ParseFilename_(content, length, _dirpath, _basename))
        AssertNotReached();
}
//----------------------------------------------------------------------------
Filename::Filename(const BasicStringSlice<FileSystem::char_type>& content) {
    Assert(content.begin());
    if (!ParseFilename_(content.begin(), content.size(), _dirpath, _basename))
        AssertNotReached();
}
//----------------------------------------------------------------------------
size_t Filename::HashValue() const {
    return hash_value(_dirpath, _basename);
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
