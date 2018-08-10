#pragma once

#include "Core.h"

#include "IO/BasenameNoExt.h"
#include "IO/Extname.h"
#include "IO/FileSystemProperties.h"

#include "IO/String_fwd.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FBasename {
public:
    FBasename() {}
    ~FBasename() {}

    FBasename(const FBasenameNoExt& basenameNoExt, const FExtname& extname);

    FBasename(const FBasename& other);
    FBasename& operator =(const FBasename& other);

    FBasename(const FileSystem::FString& content);
    FBasename& operator =(const FileSystem::FString& content);

    FBasename(const FileSystem::FStringView& content);
    FBasename& operator =(const FileSystem::FStringView& content);

    template <size_t _Dim>
    FBasename(const FileSystem::char_type (&content)[_Dim]) : FBasename(MakeStringView(content)) {}
    template <size_t _Dim>
    FBasename& operator =(const FileSystem::char_type (&content)[_Dim]) { return operator =(MakeStringView(content)); }

    bool empty() const { return _basenameNoExt.empty() && _extname.empty(); }

    const FBasenameNoExt& BasenameNoExt() const { return _basenameNoExt; }
    const FExtname& Extname() const { return _extname; }

    void SetBasenameNoExt(const FBasenameNoExt& basenameNoExt) { _basenameNoExt = basenameNoExt; }

    bool HasExtname() const { return !_extname.empty(); }
    void SetExtname(const FExtname& extName) { _extname = extName; }
    FBasename ReplaceExtname(const FExtname& extName) {
        FBasename other(*this);
        other.SetExtname(extName);
        return other;
    }

    FBasename RemoveExtname() const;

    bool Equals(const FBasename& other) const;
    bool Less(const FBasename& other) const;

    FString ToString() const;
    FWString ToWString() const;

    FStringView ToCStr(char *dst, size_t capacity) const;
    FWStringView ToWCStr(wchar_t *dst, size_t capacity) const;

    void Swap(FBasename& other);

    size_t HashValue() const;

private:
    FBasenameNoExt _basenameNoExt;
    FExtname _extname;
};
//----------------------------------------------------------------------------
inline FBasename operator +(const FBasenameNoExt& bname, const FExtname& ext) {
    return FBasename(bname, ext);
}
//----------------------------------------------------------------------------
inline bool FBasename::Equals(const FBasename& other) const {
    return  _basenameNoExt == other._basenameNoExt &&
            _extname == other._extname;
}
//----------------------------------------------------------------------------
inline bool FBasename::Less(const FBasename& other) const {
    return (_basenameNoExt == other._basenameNoExt)
        ? _extname < other._extname
        : _basenameNoExt < other._basenameNoExt;
}
//----------------------------------------------------------------------------
inline bool operator ==(const FBasename& lhs, const FBasename& rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
inline bool operator !=(const FBasename& lhs, const FBasename& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline bool operator <(const FBasename& lhs, const FBasename& rhs) {
    return lhs.Less(rhs);
}
//----------------------------------------------------------------------------
inline bool operator >=(const FBasename& lhs, const FBasename& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
inline void swap(FBasename& lhs, FBasename& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const FBasename& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PPE_CORE_API FTextWriter& operator <<(FTextWriter& oss, const FBasename& basename);
PPE_CORE_API FWTextWriter& operator <<(FWTextWriter& oss, const FBasename& basename);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
