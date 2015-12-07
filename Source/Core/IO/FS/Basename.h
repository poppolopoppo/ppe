#pragma once

#include "Core/Core.h"

#include "Core/IO/FS/BasenameNoExt.h"
#include "Core/IO/FS/Extname.h"
#include "Core/IO/FS/FileSystemProperties.h"

#include "Core/IO/StringSlice.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Basename {
public:
    Basename() {}
    ~Basename() {}

    Basename(const Core::BasenameNoExt& basenameNoExt, const Core::Extname& extname);

    Basename(const Basename& other);
    Basename& operator =(const Basename& other);

    Basename(const FileSystem::char_type* content);
    Basename& operator =(const FileSystem::char_type* content);

    Basename(const FileSystem::char_type* content, size_t length);
    Basename(const BasicStringSlice<FileSystem::char_type>& content);

    template <typename _CharTraits, typename _Allocator>
    Basename(const std::basic_string<FileSystem::char_type, _CharTraits, _Allocator>& content)
        : Basename(content.c_str(), content.size()) {}

    const Core::BasenameNoExt& BasenameNoExt() const { return _basenameNoExt; }
    const Core::Extname& Extname() const { return _extname; }

    bool empty() const { return _basenameNoExt.empty() && _extname.empty(); }

    bool HasExtension() const { return !_extname.empty(); }

    bool Equals(const Basename& other) const;
    bool Less(const Basename& other) const;

    String ToString() const;
    WString ToWString() const;

    size_t ToCStr(char *dst, size_t capacity) const;
    size_t ToWCStr(wchar_t *dst, size_t capacity) const;

    void Swap(Basename& other);

    size_t HashValue() const;

private:
    Core::BasenameNoExt _basenameNoExt;
    Core::Extname _extname;
};
//----------------------------------------------------------------------------
inline bool Basename::Equals(const Basename& other) const {
    return  _basenameNoExt == other._basenameNoExt &&
            _extname == other._extname;
}
//----------------------------------------------------------------------------
inline bool Basename::Less(const Basename& other) const {
    if (_basenameNoExt == other._basenameNoExt)
        return _extname < other._extname;
    else
        return _basenameNoExt < other._basenameNoExt;
}
//----------------------------------------------------------------------------
inline bool operator ==(const Basename& lhs, const Basename& rhs) {
    return lhs.Equals(rhs);
}
//----------------------------------------------------------------------------
inline bool operator !=(const Basename& lhs, const Basename& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
inline bool operator <(const Basename& lhs, const Basename& rhs) {
    return lhs.Less(rhs);
}
//----------------------------------------------------------------------------
inline bool operator >=(const Basename& lhs, const Basename& rhs) {
    return !operator <(lhs, rhs);
}
//----------------------------------------------------------------------------
inline void swap(Basename& lhs, Basename& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
inline hash_t hash_value(const Basename& token) {
    return token.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits>
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const Core::Basename& basename) {
    return oss << basename.BasenameNoExt() << basename.Extname();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
