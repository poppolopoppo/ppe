#pragma once

#include "Core.h"

#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicTextWriter;
//----------------------------------------------------------------------------
using FTextWriter = TBasicTextWriter<char>;
using FWTextWriter = TBasicTextWriter<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicFixedSizeTextWriter;
//----------------------------------------------------------------------------
using FFixedSizeTextWriter = TBasicFixedSizeTextWriter<char>;
using FWFixedSizeTextWriter = TBasicFixedSizeTextWriter<wchar_t>;
//----------------------------------------------------------------------------
TBasicTextWriter<char>& Crlf(TBasicTextWriter<char>& s);
TBasicTextWriter<wchar_t>& Crlf(TBasicTextWriter<wchar_t>& s);
TBasicTextWriter<char>& Eol(TBasicTextWriter<char>& s);
TBasicTextWriter<wchar_t>& Eol(TBasicTextWriter<wchar_t>& s);
TBasicTextWriter<char>& Eos(TBasicTextWriter<char>& s);
TBasicTextWriter<wchar_t>& Eos(TBasicTextWriter<wchar_t>& s);
template <typename _Char>
TBasicTextWriter<_Char>& Endl(TBasicTextWriter<_Char>& s);
//----------------------------------------------------------------------------
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, bool v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, i8 v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, i16 v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, i32 v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, i64 v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, u8 v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, u16 v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, u32 v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, u64 v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, float v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, double v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, const void* v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, const TBasicStringView<_Char>& v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, long v);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, unsigned long v);
template <typename _Char, size_t _Dim>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, const _Char(&v)[_Dim]);
template <typename _Char>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& w, _Char v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
