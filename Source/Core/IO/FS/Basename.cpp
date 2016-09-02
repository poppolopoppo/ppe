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
static bool ParseBasename_(const FileSystem::StringView& str, BasenameNoExt& basenameNoExt, Extname& extname) {
    basenameNoExt = BasenameNoExt();
    extname = Extname();

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
Basename::Basename(const FileSystem::StringView& content) {
    Assert(not content.empty());
    ParseBasename_(content, _basenameNoExt, _extname);
}
//----------------------------------------------------------------------------
Basename& Basename::operator =(const FileSystem::StringView& content) {
    Assert(not content.empty());
    ParseBasename_(content, _basenameNoExt, _extname);
    return *this;
}
//----------------------------------------------------------------------------
String Basename::ToString() const {
    STACKLOCAL_OCSTRSTREAM(oss, 1024);
    oss << *this;
    return Core::ToString(oss.MakeView());
}
//----------------------------------------------------------------------------
WString Basename::ToWString() const {
    STACKLOCAL_WOCSTRSTREAM(oss, 1024);
    oss << *this;
    return Core::ToWString(oss.MakeView());
}
//----------------------------------------------------------------------------
size_t Basename::ToCStr(char *dst, size_t capacity) const {
    OCStrStream oss(dst, capacity);
    oss << *this;
    return static_cast<size_t>(oss.tellp());
}
//----------------------------------------------------------------------------
size_t Basename::ToWCStr(wchar_t *dst, size_t capacity) const {
    WOCStrStream oss(dst, capacity);
    oss << *this;
    return static_cast<size_t>(oss.tellp());
}
//----------------------------------------------------------------------------
void Basename::Swap(Basename& other) {
    swap(other._basenameNoExt, _basenameNoExt);
    swap(other._extname, _extname);
}
//----------------------------------------------------------------------------
size_t Basename::HashValue() const {
    return hash_tuple(  size_t(_basenameNoExt.c_str()),
                        size_t(_extname.c_str()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
