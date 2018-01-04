#pragma once

#include "Core/IO/TextWriter.h"

namespace Core {
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
TBasicTextManipulator<_Char> FTextFormat::PadCenter(size_t width) {
    return [width](TBasicTextWriter<_Char>& s) -> TBasicTextWriter<_Char>& {
        s.Format().SetPadding(FTextFormat::Padding_Center);
        s.Format().SetWidth(width);
        return s;
    };
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> FTextFormat::PadLeft(size_t width) {
    return [width](TBasicTextWriter<_Char>& s) -> TBasicTextWriter<_Char>& {
        s.Format().SetPadding(FTextFormat::Padding_Left);
        s.Format().SetWidth(width);
        return s;
    };
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> FTextFormat::PadRight(size_t width) {
    return [width](TBasicTextWriter<_Char>& s) -> TBasicTextWriter<_Char>& {
        s.Format().SetPadding(FTextFormat::Padding_Right);
        s.Format().SetWidth(width);
        return s;
    };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> FTextFormat::PadCenter(size_t width, _Char fill) {
    return [width, fill](TBasicTextWriter<_Char>& s) -> TBasicTextWriter<_Char>& {
        s.Format().SetPadding(FTextFormat::Padding_Center);
        s.Format().SetWidth(width);
        s.SetFillChar(fill);
        return s;
    };
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> FTextFormat::PadLeft(size_t width, _Char fill) {
    return [width, fill](TBasicTextWriter<_Char>& s) -> TBasicTextWriter<_Char>& {
        s.Format().SetPadding(FTextFormat::Padding_Left);
        s.Format().SetWidth(width);
        s.SetFillChar(fill);
        return s;
    };
}
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextManipulator<_Char> FTextFormat::PadRight(size_t width, _Char fill) {
    return [width, fill](TBasicTextWriter<_Char>& s) -> TBasicTextWriter<_Char>& {
        s.Format().SetPadding(FTextFormat::Padding_Right);
        s.Format().SetWidth(width);
        s.SetFillChar(fill);
        return s;
    };
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
} //!namespace Core
