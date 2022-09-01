#pragma once

#include "Core_fwd.h"

#include "IO/TextReader_fwd.h"

#include "IO/String.h"
#include "IO/StringView.h"
#include "Memory/PtrRef.h"

namespace PPE {
class IBufferedStreamWriter;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FBaseTextReader {
public:
    explicit FBaseTextReader(TPtrRef<IBufferedStreamReader> istream) NOEXCEPT;
    ~FBaseTextReader();

    CONSTF u32 Base() const { return _base; }
    void SetBase(FTextFormat::EBase value) { _base = value; }

    CONSTF IBufferedStreamReader& Stream() const { return _istream; }

    bool Eof() const NOEXCEPT;
    std::streamoff Tell() const NOEXCEPT;
    void Seek(std::streamoff off);
    void Reset();

protected:
    TPtrRef<IBufferedStreamReader> _istream;
    u32 _base{ 10 };
};
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicTextReader : public FBaseTextReader {
public:
    using charset_func = bool (*)(_Char ch) NOEXCEPT;
    using string_type = TBasicString<_Char>;
    using stringconversion_type = TBasicStringConversion<_Char>;

    explicit TBasicTextReader(TPtrRef<IBufferedStreamReader> istream) NOEXCEPT
    :   FBaseTextReader(istream) {
    }

    bool Expect(_Char ch);
    bool Read(_Char* ch);

    bool Read(bool* p);
    bool Read(i8* p);
    bool Read(i16* p);
    bool Read(i32* p);
    bool Read(i64* p);
    bool Read(u8* p);
    bool Read(u16* p);
    bool Read(u32* p);
    bool Read(u64* p);
    bool Read(float* p);
    bool Read(double* p);
    bool Read(string_type* p);

    bool SkipSpaces() { return SkipCharset(&IsSpace); }
    bool SkipCharset(charset_func charset);

    bool ReadLine(string_type* line);
    bool ReadWord(string_type* word);
    bool ReadCharset(string_type* parsed, charset_func charset);
    bool ReadCharset(stringconversion_type* parsed, charset_func charset);

    inline friend void swap(TBasicTextReader& lhs, TBasicTextReader& rhs) NOEXCEPT {
        using std::swap;
        swap(lhs._istream, rhs._istream);
        swap(lhs._read, rhs._read);
    }

private:
    string_type _read;
};
//----------------------------------------------------------------------------
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, bool* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, i8* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, i16* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, i32* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, i64* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, u8* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, u16* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, u32* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, u64* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, float* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, double* p) { return r.Read(p); }
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& r, TBasicString<_Char>* p) { return r.Read(p); }
//----------------------------------------------------------------------------
template <typename _Char, typename T>
bool operator >>(TBasicTextReader<_Char>& r, T* p) {
    typename TBasicTextReader<_Char>::FParsedToken token;
    if (r.Next(&token, &IsIdentifier) && token.Conv >> p)
        return true;
    r.Rollback(token);
    return false;
}
//----------------------------------------------------------------------------
template <> PPE_CORE_API bool TBasicTextReader<char>::Expect(char ch);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(char* ch);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(bool* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(i8* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(i16* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(i32* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(i64* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(u8* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(u16* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(u32* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(u64* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(float* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(double* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::Read(FString* p);
template <> PPE_CORE_API bool TBasicTextReader<char>::SkipCharset(charset_func charset);
template <> PPE_CORE_API bool TBasicTextReader<char>::ReadLine(string_type* line);
template <> PPE_CORE_API bool TBasicTextReader<char>::ReadWord(string_type* word);
template <> PPE_CORE_API bool TBasicTextReader<char>::ReadCharset(string_type* parsed, charset_func charset);
template <> PPE_CORE_API bool TBasicTextReader<char>::ReadCharset(stringconversion_type* parsed, charset_func charset);
//----------------------------------------------------------------------------
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Expect(wchar_t ch);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(wchar_t* ch);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(bool* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(i8* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(i16* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(i32* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(i64* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(u8* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(u16* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(u32* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(u64* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(float* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(double* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::Read(FWString* p);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::SkipCharset(charset_func charset);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::ReadLine(string_type* line);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::ReadWord(string_type* word);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::ReadCharset(string_type* parsed, charset_func charset);
template <> PPE_CORE_API bool TBasicTextReader<wchar_t>::ReadCharset(stringconversion_type* parsed, charset_func charset);
//----------------------------------------------------------------------------
#ifndef EXPORT_PPE_RUNTIME_CORE_TEXTREADER
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TBasicTextReader<char>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TBasicTextReader<wchar_t>;
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
