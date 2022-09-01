#pragma once

#include "Core_fwd.h"

#include "IO/String_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicTextReader;
//----------------------------------------------------------------------------
using FTextReader = TBasicTextReader<char>;
using FWTextReader = TBasicTextReader<wchar_t>;
//----------------------------------------------------------------------------
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, bool* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, i8* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, i16* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, i32* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, i64* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, u8* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, u16* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, u32* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, u64* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, float* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, double* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, TBasicString<_Char>* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, long* p);
template <typename _Char>
bool operator >>(TBasicTextReader<_Char>& w, unsigned long* p);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
