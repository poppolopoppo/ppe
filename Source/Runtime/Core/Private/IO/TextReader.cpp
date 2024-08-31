// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#define EXPORT_PPE_RUNTIME_CORE_TEXTREADER

#include "IO/TextReader.h"

#include "IO/StreamProvider.h"
#include "IO/StringConversion.h"
#include "IO/StringView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
bool TextReader_Expect_(IBufferedStreamReader& iss, _Char ch) {
    _Char read;
    if (iss.Peek(read) && ch == read) {
        Verify(iss.ReadPOD(&read));
        Assert_NoAssume(read == ch);
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename _Char, typename T>
bool TextReader_ReadAndConvert_(TBasicTextReader<_Char>& r, T* p, bool (*charset)(_Char ch) NOEXCEPT) {
    const std::streamoff before = r.Tell();

    r.SkipSpaces();

    TBasicStringConversion<_Char> conv;
    if (r.ReadCharset(&conv, charset) && conv >> p)
        return true;

    r.Seek(before);
    return false;
}
//----------------------------------------------------------------------------
template <typename _Char, typename T>
bool TextReader_ReadUnsignedInt_(TBasicTextReader<_Char>& r, T* p) {
    return TextReader_ReadAndConvert_(r, p, &IsDigit);
}
//----------------------------------------------------------------------------
template <typename _Char, typename T>
bool TextReader_ReadSignedInt_(TBasicTextReader<_Char>& r, T* p) {
    const std::streamoff before = r.Tell();

    r.SkipSpaces();

    const bool negative = r.Expect(STRING_LITERAL(_Char, '-'));

    if (TextReader_ReadAndConvert_(r, p, &IsDigit)) {
        if (negative)
            *p = -*p;
        return true;
    }

    r.Seek(before);
    return false;
}
//----------------------------------------------------------------------------
template <typename _Char, typename T>
bool TextReader_ReadFloat_(TBasicTextReader<_Char>& r, T* p) {
    const std::streamoff before = r.Tell();

    r.SkipSpaces();

    const bool negative = r.Expect(STRING_LITERAL(_Char, '-'));
    bool (*float_charset)(_Char) NOEXCEPT = [](_Char ch) NOEXCEPT -> bool {
        return IsDigit(ch) || ch == STRING_LITERAL(_Char, '.');
    };

    if (TextReader_ReadAndConvert_(r, p, float_charset)) {
        if (negative)
            *p = -*p;
        return true;
    }

    r.Seek(before);
    return false;
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TextReader_AppendLine_(IBufferedStreamReader& iss, const TAppendable<_Char>& outp) {
    Assert_NoAssume(iss.IsSeekableI());

    bool success = false;

    _Char ch;
    while (iss.Peek(ch)) {
        success = true;

        if (ch == STRING_LITERAL(_Char, '\n')) {
            iss.ReadPOD(&ch);
            break;
        }
        if (ch == STRING_LITERAL(_Char, '\r')) {
            iss.ReadPOD(&ch); // look at next character for CRLF
            if (iss.Peek(ch) && ch == STRING_LITERAL(_Char, '\n')) {
                iss.ReadPOD(&ch);
                break;
            }

            iss.SeekI(-static_cast<std::streamoff>(sizeof(_Char)), ESeekOrigin::Relative);
        }

        Verify(iss.ReadPOD(&ch));

        outp.push_back(ch);
    }

    return success;
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TextReader_ReadLine_(IBufferedStreamReader& iss, TBasicString<_Char>* s) {
    if (!!s) {
        s->clear();
        return TextReader_AppendLine_(iss, MakeAppendable(*s));
    }
    return TextReader_AppendLine_<_Char>(iss, Default);
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TextReader_AppendWord_(IBufferedStreamReader& iss, const TAppendable<_Char>& outp) {
    bool success = false;

    _Char ch; // skip spaces
    while (iss.Peek(ch) and IsSpace(ch)) {
        Verify(iss.ReadPOD(&ch));
        Assert_NoAssume(IsSpace(ch));
    }

    while (iss.ReadPOD(&ch) and not IsSpace(ch)) {
        success = true;
        outp.push_back(ch);
    }

    return success;
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TextReader_ReadWord_(IBufferedStreamReader& iss, TBasicString<_Char>* s) {
    if (!!s) {
        s->clear();
        return TextReader_AppendWord_(iss, MakeAppendable(*s));
    }
    return TextReader_AppendWord_<_Char>(iss, Default);
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TextReader_SkipCharset_(IBufferedStreamReader& iss, bool (*charset)(_Char ch) NOEXCEPT) {
    bool success = false;

    _Char ch;
    while (iss.Peek(ch) and charset(ch)) {
        success = true;
        Verify(iss.ReadPOD(&ch));
        Assert_NoAssume(charset(ch));
    }

    return success;
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TextReader_AppendCharset_(IBufferedStreamReader& iss, const TAppendable<_Char>& outp, bool (*charset)(_Char ch) NOEXCEPT) {
    bool success = false;

    _Char ch;
    while (iss.Peek(ch) and charset(ch)) {
        success = true;
        Verify(iss.ReadPOD(&ch));
        Assert_NoAssume(charset(ch));

        outp.push_back(ch);
    }

    return success;
}
//----------------------------------------------------------------------------
template <typename _Char>
bool TextReader_ReadCharset_(IBufferedStreamReader& iss, TBasicString<_Char>* s, bool (*charset)(_Char ch) NOEXCEPT) {
    if (!!s) {
        s->clear();
        return TextReader_AppendCharset_(iss, MakeAppendable(*s), charset);
    }
    return TextReader_AppendCharset_<_Char>(iss, Default, charset);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBaseTextReader::FBaseTextReader(TPtrRef<IBufferedStreamReader> istream) NOEXCEPT
:   _istream(istream) {
    Assert(_istream);
}
//----------------------------------------------------------------------------
FBaseTextReader::~FBaseTextReader() = default;
//----------------------------------------------------------------------------
bool FBaseTextReader::Eof() const NOEXCEPT {
    return _istream->Eof();
}
//----------------------------------------------------------------------------
std::streamoff FBaseTextReader::Tell() const NOEXCEPT {
    return _istream->TellI();
}
//----------------------------------------------------------------------------
void FBaseTextReader::Seek(std::streamoff offset) {
    _istream->SeekI(offset, ESeekOrigin::Begin);
}
//----------------------------------------------------------------------------
void FBaseTextReader::Reset() {
    _istream->SeekI(0, ESeekOrigin::Begin);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <> bool TBasicTextReader<char>::Expect(char ch) { return TextReader_Expect_(*_istream, ch); }
template <> bool TBasicTextReader<char>::Read(char* ch) { return _istream->ReadPOD(ch); }
template <> bool TBasicTextReader<char>::Read(bool* p) { return TextReader_ReadUnsignedInt_(*this, p); }
template <> bool TBasicTextReader<char>::Read(i8* p) { return TextReader_ReadSignedInt_(*this, p); }
template <> bool TBasicTextReader<char>::Read(i16* p) { return TextReader_ReadSignedInt_(*this, p); }
template <> bool TBasicTextReader<char>::Read(i32* p) { return TextReader_ReadSignedInt_(*this, p); }
template <> bool TBasicTextReader<char>::Read(i64* p) { return TextReader_ReadSignedInt_(*this, p); }
template <> bool TBasicTextReader<char>::Read(u8* p) { return TextReader_ReadUnsignedInt_(*this, p); }
template <> bool TBasicTextReader<char>::Read(u16* p) { return TextReader_ReadUnsignedInt_(*this, p); }
template <> bool TBasicTextReader<char>::Read(u32* p) { return TextReader_ReadUnsignedInt_(*this, p); }
template <> bool TBasicTextReader<char>::Read(u64* p) { return TextReader_ReadUnsignedInt_(*this, p); }
template <> bool TBasicTextReader<char>::Read(float* p) { return TextReader_ReadFloat_(*this, p); }
template <> bool TBasicTextReader<char>::Read(double* p) { return TextReader_ReadFloat_(*this, p); }
template <> bool TBasicTextReader<char>::Read(string_type* p) { return ReadWord(p); }
template <> bool TBasicTextReader<char>::SkipSpaces() { return SkipCharset(&IsSpace); }
template <> bool TBasicTextReader<char>::SkipCharset(charset_func charset) { return TextReader_SkipCharset_(*_istream, charset); }
template <> bool TBasicTextReader<char>::ReadLine(string_type* line) { return TextReader_ReadLine_(*_istream, line); }
template <> bool TBasicTextReader<char>::ReadWord(string_type* word) { return TextReader_ReadWord_(*_istream, word); }
template <> bool TBasicTextReader<char>::ReadIdentifier(string_type* id) { return TextReader_ReadCharset_(*_istream, id, &IsIdentifier); }
template <> bool TBasicTextReader<char>::ReadCharset(string_type* parsed, charset_func charset) { return TextReader_ReadCharset_(*_istream, parsed, charset); }
template <> bool TBasicTextReader<char>::ReadLine(const stringappendable_type& line) { return TextReader_AppendLine_(*_istream, line); }
template <> bool TBasicTextReader<char>::ReadWord(const stringappendable_type& word) { return TextReader_AppendWord_(*_istream, word); }
template <> bool TBasicTextReader<char>::ReadIdentifier(const stringappendable_type& id) { SkipSpaces(); return TextReader_AppendCharset_(*_istream, id, &IsIdentifier); }
template <> bool TBasicTextReader<char>::ReadCharset(const stringappendable_type& parsed, charset_func charset) { return TextReader_AppendCharset_(*_istream, parsed, charset); }
template <> bool TBasicTextReader<char>::ReadCharset(stringconversion_type* parsed, charset_func charset) {
    if (TextReader_ReadCharset_(*_istream, &_read, charset)) {
        *parsed = stringconversion_type{ _read, _base };
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <> bool TBasicTextReader<wchar_t>::Expect(wchar_t ch) { return TextReader_Expect_(*_istream, ch); }
template <> bool TBasicTextReader<wchar_t>::Read(wchar_t* p) { return _istream->ReadPOD(p); }
template <> bool TBasicTextReader<wchar_t>::Read(bool* p) { return TextReader_ReadUnsignedInt_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(i8* p) { return TextReader_ReadSignedInt_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(i16* p) { return TextReader_ReadSignedInt_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(i32* p) { return TextReader_ReadSignedInt_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(i64* p) { return TextReader_ReadSignedInt_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(u8* p) { return TextReader_ReadUnsignedInt_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(u16* p) { return TextReader_ReadUnsignedInt_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(u32* p) { return TextReader_ReadUnsignedInt_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(u64* p) { return TextReader_ReadUnsignedInt_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(float* p) { return TextReader_ReadFloat_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(double* p) { return TextReader_ReadFloat_(*this, p); }
template <> bool TBasicTextReader<wchar_t>::Read(string_type* p) { return ReadWord(p); }
template <> bool TBasicTextReader<wchar_t>::SkipSpaces() { return SkipCharset(&IsSpace); }
template <> bool TBasicTextReader<wchar_t>::SkipCharset(charset_func charset) { return TextReader_SkipCharset_(*_istream, charset); }
template <> bool TBasicTextReader<wchar_t>::ReadLine(string_type* line) { return TextReader_ReadLine_(*_istream, line); }
template <> bool TBasicTextReader<wchar_t>::ReadWord(string_type* word) { return TextReader_ReadWord_(*_istream, word); }
template <> bool TBasicTextReader<wchar_t>::ReadIdentifier(string_type* id) { return TextReader_ReadCharset_(*_istream, id, &IsIdentifier); }
template <> bool TBasicTextReader<wchar_t>::ReadCharset(string_type* parsed, charset_func charset) { return TextReader_ReadCharset_(*_istream, parsed, charset); }
template <> bool TBasicTextReader<wchar_t>::ReadLine(const stringappendable_type& line) { return TextReader_AppendLine_(*_istream, line); }
template <> bool TBasicTextReader<wchar_t>::ReadWord(const stringappendable_type& word) { return TextReader_AppendWord_(*_istream, word); }
template <> bool TBasicTextReader<wchar_t>::ReadIdentifier(const stringappendable_type& id) { SkipSpaces(); return TextReader_AppendCharset_(*_istream, id, &IsIdentifier); }
template <> bool TBasicTextReader<wchar_t>::ReadCharset(const stringappendable_type& parsed, charset_func charset) { return TextReader_AppendCharset_(*_istream, parsed, charset); }
template <> bool TBasicTextReader<wchar_t>::ReadCharset(stringconversion_type* parsed, charset_func charset) {
    if (TextReader_ReadCharset_(*_istream, &_read, charset)) {
        *parsed = stringconversion_type{ _read, _base };
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicTextReader<char>;
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBasicTextReader<wchar_t>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
