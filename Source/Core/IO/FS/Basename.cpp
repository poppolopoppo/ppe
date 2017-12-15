#include "stdafx.h"

#include "Basename.h"

#include "Container/Hash.h"
#include "IO/String.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ParseBasename_(const FileSystem::FStringView& str, FBasenameNoExt& basenameNoExt, FExtname& extname) {
    basenameNoExt = FBasenameNoExt();
    extname = FExtname();

    if (str.empty())
        return false;

#ifdef WITH_CORE_ASSERT
    const auto sep = str.FindIf([](FileSystem::char_type ch) {
        return (ch == FileSystem::Separator || ch == FileSystem::AltSeparator );
    });
    Assert(str.end() == sep);
#endif

    const auto dot = str.FindIfR([](FileSystem::char_type ch) {
        return (ch == L'.');
    });

    if (str.rend() == dot) {
        basenameNoExt = str;
    }
    else {
        basenameNoExt = str.CutBefore(dot);
        extname = str.CutStartingAt(dot);
    }

    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBasename::FBasename(const Core::FBasenameNoExt& basenameNoExt, const Core::FExtname& extname)
:   _basenameNoExt(basenameNoExt), _extname(extname) {}
//----------------------------------------------------------------------------
FBasename::FBasename(const FBasename& other)
:   _basenameNoExt(other._basenameNoExt), _extname(other._extname) {}
//----------------------------------------------------------------------------
FBasename& FBasename::operator =(const FBasename& other) {
    _basenameNoExt = other._basenameNoExt;
    _extname = other._extname;
    return *this;
}
//----------------------------------------------------------------------------
FBasename::FBasename(const FileSystem::FStringView& content) {
    Assert(not content.empty());
    ParseBasename_(content, _basenameNoExt, _extname);
}
//----------------------------------------------------------------------------
FBasename& FBasename::operator =(const FileSystem::FStringView& content) {
    Assert(not content.empty());
    ParseBasename_(content, _basenameNoExt, _extname);
    return *this;
}
//----------------------------------------------------------------------------
FString FBasename::ToString() const {
    STACKLOCAL_OCSTRSTREAM(oss, 1024);
    oss << *this;
    return Core::ToString(oss.MakeView());
}
//----------------------------------------------------------------------------
FWString FBasename::ToWString() const {
    STACKLOCAL_WOCSTRSTREAM(oss, 1024);
    oss << *this;
    return Core::ToWString(oss.MakeView());
}
//----------------------------------------------------------------------------
FStringView FBasename::ToCStr(char *dst, size_t capacity) const {
    FOCStrStream oss(dst, capacity);
    oss << *this;
    return oss.MakeView();
}
//----------------------------------------------------------------------------
FWStringView FBasename::ToWCStr(wchar_t *dst, size_t capacity) const {
    FWOCStrStream oss(dst, capacity);
    oss << *this;
    return oss.MakeView();
}
//----------------------------------------------------------------------------
void FBasename::Swap(FBasename& other) {
    swap(other._basenameNoExt, _basenameNoExt);
    swap(other._extname, _extname);
}
//----------------------------------------------------------------------------
size_t FBasename::HashValue() const {
    return hash_tuple(_basenameNoExt, _extname);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
