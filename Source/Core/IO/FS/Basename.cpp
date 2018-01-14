#include "stdafx.h"

#include "Basename.h"

#include "Container/Hash.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryProvider.h"

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
FBasename::FBasename(const FileSystem::FString& content)
    : FBasename(content.MakeView())
{}
//----------------------------------------------------------------------------
FBasename& FBasename::operator =(const FileSystem::FString& content) {
    return operator =(content.MakeView());
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
    FStringBuilder oss;
    oss << *this;
    return oss.ToString();
}
//----------------------------------------------------------------------------
FWString FBasename::ToWString() const {
    FWStringBuilder oss;
    oss << *this;
    return oss.ToString();
}
//----------------------------------------------------------------------------
FStringView FBasename::ToCStr(char *dst, size_t capacity) const {
    FFixedSizeTextWriter oss(dst, capacity);
    oss << *this << Eos;
    return oss.Written();
}
//----------------------------------------------------------------------------
FWStringView FBasename::ToWCStr(wchar_t *dst, size_t capacity) const {
    FWFixedSizeTextWriter oss(dst, capacity);
    oss << *this << Eos;
    return oss.Written();
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
FTextWriter& operator <<(FTextWriter& oss, const FBasename& basename) {
    return oss << basename.BasenameNoExt() << basename.Extname();
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FBasename& basename) {
    return oss << basename.BasenameNoExt() << basename.Extname();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
