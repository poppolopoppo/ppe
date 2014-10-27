#include "stdafx.h"

#include "Basename.h"

#include "IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ParseBasename_(
    const wchar_t *cstr,
    size_t length,
    BasenameNoExt& basenameNoExt,
    Extname& extname
    ) {
    basenameNoExt = BasenameNoExt();
    extname = Extname();

    if (!cstr || L'\0' == cstr[0])
        return false;

    static const FileSystem::char_type *sep = L"\\/";

    const wchar_t *b = cstr + length - 1;
    for (; b != cstr && !StrChr(sep, *b); --b);

    const wchar_t *p = cstr + length - 1;
    for (; p != b && L'.' != *p; --p);

    if (p != b) {
        Assert(L'.' == *p);
        basenameNoExt = BasenameNoExt(b, p - b);
        extname = Extname(p, cstr + length - p);
    }
    else if (L'.' == *b && (L'\0' == b[1] ||
            (L'.' == b[1] && L'\0' == b[2])) ) {
        // '.' and '..' are forbiden
        return false;
    }
    else {
        basenameNoExt = BasenameNoExt(b, cstr + length - b);
    }

    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Basename::Basename(const Core::BasenameNoExt& basenameNoExt, const Core::Extname& extname)
:   _basenameNoExt(basenameNoExt), _extname(extname) {}
//----------------------------------------------------------------------------
Basename::Basename(const Basename& other)
:   _basenameNoExt(other._basenameNoExt), _extname(other._extname) {}
//----------------------------------------------------------------------------
Basename& Basename::operator =(const Basename& other) {
    _basenameNoExt = other._basenameNoExt;
    _extname = other._extname;
    return *this;
}
//----------------------------------------------------------------------------
Basename::Basename(const FileSystem::char_type* content) {
    Assert(content);
    ParseBasename_(content, Length(content), _basenameNoExt, _extname);
}
//----------------------------------------------------------------------------
Basename& Basename::operator =(const FileSystem::char_type* content) {
    Assert(content);
    ParseBasename_(content, Length(content), _basenameNoExt, _extname);
    return *this;
}
//----------------------------------------------------------------------------
Basename::Basename(const FileSystem::char_type* content, size_t length) {
    Assert(content);
    ParseBasename_(content, length, _basenameNoExt, _extname);
}
//----------------------------------------------------------------------------
Basename::Basename(const BasicStringSlice<FileSystem::char_type>& content) {
    Assert(content.begin());
    ParseBasename_(content.begin(), content.size(), _basenameNoExt, _extname);
}
//----------------------------------------------------------------------------
void Basename::Swap(Basename& other) {
    swap(other._basenameNoExt, _basenameNoExt);
    swap(other._extname, _extname);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
