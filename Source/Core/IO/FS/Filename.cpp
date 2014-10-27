#include "stdafx.h"

#include "Filename.h"

#include "Container/Hash.h"
#include "IO/Stream.h"

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
    ParseFilename_(content, Length(content), _dirpath, _basename);
}
//----------------------------------------------------------------------------
Filename& Filename::operator =(const FileSystem::char_type* content) {
    Assert(content);
    ParseFilename_(content, Length(content), _dirpath, _basename);
    return *this;
}
//----------------------------------------------------------------------------
Filename::Filename(const FileSystem::char_type* content, size_t length) {
    Assert(content);
    ParseFilename_(content, length, _dirpath, _basename);
}
//----------------------------------------------------------------------------
Filename::Filename(const BasicStringSlice<FileSystem::char_type>& content) {
    Assert(content.begin());
    ParseFilename_(content.begin(), content.size(), _dirpath, _basename);
}
//----------------------------------------------------------------------------
size_t Filename::HashValue() const {
    return hash_value(_dirpath, _basename);
}
//----------------------------------------------------------------------------
String Filename::ToString() const {
    char cstr[1024];
    {
        OCStrStream oss(cstr);
        oss << *this;
    }
    return String(cstr);
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
