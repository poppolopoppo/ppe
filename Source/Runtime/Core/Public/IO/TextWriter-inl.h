#pragma once

#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline TBasicTextWriter<char>& Crlf(TBasicTextWriter<char>& s) {
    s.Put("\r\n");
    return s;
}
//----------------------------------------------------------------------------
inline TBasicTextWriter<wchar_t>& Crlf(TBasicTextWriter<wchar_t>& s) {
    s.Put(L"\r\n");
    return s;
}
//----------------------------------------------------------------------------
inline TBasicTextWriter<char>& Eol(TBasicTextWriter<char>& s) {
    if (s.Format().Misc() & FTextFormat::Crlf)
        s.Put("\r\n");
    else
        s.Put('\n');
    return s;
}
//----------------------------------------------------------------------------
inline TBasicTextWriter<wchar_t>& Eol(TBasicTextWriter<wchar_t>& s) {
    if (s.Format().Misc() & FTextFormat::Crlf)
        s.Put(L"\r\n");
    else
        s.Put(L'\n');
    return s;
}
//----------------------------------------------------------------------------
inline TBasicTextWriter<char>& Eos(TBasicTextWriter<char>& s) {
    s.Put('\0');
    return s;
}
//----------------------------------------------------------------------------
inline TBasicTextWriter<wchar_t>& Eos(TBasicTextWriter<wchar_t>& s) {
    s.Put(L'\0');
    return s;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& Endl(TBasicTextWriter<_Char>& s) {
    s << Eol;
    s.Flush();
    return s;
}
//----------------------------------------------------------------------------
inline TBasicTextWriter<char>& Tab(TBasicTextWriter<char>& s) {
    s.Put('\t');
    return s;
}
//----------------------------------------------------------------------------
inline TBasicTextWriter<wchar_t>& Tab(TBasicTextWriter<wchar_t>& s) {
    s.Put(L'\t');
    return s;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, const FTextFormat& v) {
    s.Format() = v;
    return s;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, const FTextFormat::FFloat& v) {
    s.Format().SetFloat(v.Float);
    s.Format().SetPrecision(v.Precision);
    return s;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, FTextFormat::EBase v) {
    s.Format().SetBase(v);
    return s;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, FTextFormat::ECase v) {
    s.Format().SetCase(v);
    return s;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, FTextFormat::EFloat v) {
    s.Format().SetFloat(v);
    return s;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& s, FTextFormat::EMisc v) {
    s.Format().SetMisc(s.Format().Misc() + v);
    return s;
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator >>(TBasicTextWriter<_Char>& s, FTextFormat::EMisc v) {
    s.Format().SetMisc(s.Format().Misc() - v);
    return s;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
