#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/BasenameNoExt.h"
#include "Core/IO/FS/Extname.h"
#include "Core/IO/FS/FileSystemProperties.h"

#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FBasename {
public:
    FBasename() {}
    ~FBasename() {}

    FBasename(const FBasenameNoExt& basenameNoExt, const FExtname& extname);

    FBasename(const FBasename& other);
    FBasename& operator =(const FBasename& other);

    FBasename(const FileSystem::FStringView& content);
    FBasename& operator =(const FileSystem::FStringView& content);

    template <size_t _Dim>
    FBasename(const FileSystem::char_type (&content)[_Dim]) : FBasename(MakeStringView(content)) {}
    template <size_t _Dim>
    FBasename& operator =(const FileSystem::char_type (&content)[_Dim]) { return operator =(MakeStringView(content)); }

    template <typename _CharTraits, typename _Allocator>
    FBasename(const std::basic_string<FileSystem::char_type, _CharTraits, _Allocator>& content)
        : FBasename(MakeStringView(content)) {}

    bool empty() const { return _basenameNoExt.empty() && _extname.empty(); }

    const FBasenameNoExt& BasenameNoExt() const { return _basenameNoExt; }
    const FExtname& Extname() const { return _extname; }

    void SetBasenameNoExt(const FBasenameNoExt& basenameNoExt) { _basenameNoExt = basenameNoExt; }
    void SetExtname(const FExtname& extName) { _extname = extName; }

    bool HasExtname() const { return !_extname.empty(); }

    bool Equals(const FBasename& other) const;
    bool Less(const FBasename& other) const;

    FString ToString() const;
    FWString ToWString() const;

    size_t ToCStr(char *dst, size_t capacity) const;
    size_t ToWCStr(wchar_t *dst, size_t capacity) const;

    void Swap(FBasename& other);

    size_t HashValue() const;

private:
    FBasenameNoExt _basenameNoExt;
    FExtname _extname;
};
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
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Core::FBasename& basename) {
    return oss << basename.BasenameNoExt() << basename.Extname();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
