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
    s.Put('\n');
    return s;
}
//----------------------------------------------------------------------------
inline TBasicTextWriter<wchar_t>& Eol(TBasicTextWriter<wchar_t>& s) {
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
template <typename _Char>
TBasicTextWriter<_Char>& FTextFormat::DontPad(TBasicTextWriter<_Char>& s) {
    s.Format().SetPadding(FTextFormat::Padding_None);
    return s;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> FTextFormat::Pad(EPadding padding, size_t width, _Char fill) {
    return [padding, width, fill](TBasicTextWriter<_Char>& s) -> TBasicTextWriter<_Char>& {
        s.Format().SetPadding(padding);

        if (INDEX_NONE != width)
            s.Format().SetWidth(width);

        if (_Char() != fill)
            s.SetFillChar(fill);

        return s;
    };
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> FTextFormat::PadCenter(size_t width, _Char fill) {
    return Pad(Padding_Center, width, fill);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> FTextFormat::PadLeft(size_t width, _Char fill) {
    return Pad(Padding_Left, width, fill);
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> FTextFormat::PadRight(size_t width, _Char fill) {
    return Pad(Padding_Right, width, fill);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> FTextFormat::SetFill(_Char ch) {
    return [ch](TBasicTextWriter<_Char>& s) -> TBasicTextWriter<_Char>& {
        s.SetFillChar(ch);
        return s;
    };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
