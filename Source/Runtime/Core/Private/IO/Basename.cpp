#include "stdafx.h"

#include "IO/Basename.h"

#include "Container/Hash.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringConversion.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::is_pod_v<FBasename>);
//----------------------------------------------------------------------------
NODISCARD static bool ParseBasename_(const FileSystem::FStringView& str, FBasenameNoExt& basenameNoExt, FExtname& extname) {
    basenameNoExt = FBasenameNoExt();
    extname = FExtname();

    if (str.empty())
        return true;

#if USE_PPE_ASSERT
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
FBasename::FBasename(const PPE::FBasenameNoExt& basenameNoExt, const PPE::FExtname& extname) NOEXCEPT
:   _basenameNoExt(basenameNoExt), _extname(extname) {}
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
    VerifyRelease( ParseBasename_(content, _basenameNoExt, _extname) );
}
//----------------------------------------------------------------------------
FBasename& FBasename::operator =(const FileSystem::FStringView& content) {
    Assert(not content.empty());
    VerifyRelease( ParseBasename_(content, _basenameNoExt, _extname) );
    return *this;
}
//----------------------------------------------------------------------------
FBasename FBasename::RemoveExtname() const {
    return FBasename(_basenameNoExt, FExtname());
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
    return oss.Written().ShiftBack();
}
//----------------------------------------------------------------------------
FWStringView FBasename::ToWCStr(wchar_t *dst, size_t capacity) const {
    FWFixedSizeTextWriter oss(dst, capacity);
    oss << *this << Eos;
    return oss.Written().ShiftBack();
}
//----------------------------------------------------------------------------
void FBasename::Swap(FBasename& other) NOEXCEPT {
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
bool operator >>(const FStringConversion& iss, FBasename* basename) {
    return (FWStringConversion(UTF_8_TO_WCHAR(iss.Input)) >> basename);
}
//----------------------------------------------------------------------------
PPE_CORE_API bool operator >>(const FWStringConversion& iss, FBasename* basename) {
    FBasenameNoExt basenameNoExt;
    FExtname extname;
    if (ParseBasename_(iss.Input, basenameNoExt, extname)) {
        *basename = { basenameNoExt, extname };
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
