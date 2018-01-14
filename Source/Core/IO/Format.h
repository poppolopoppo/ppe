#pragma once

#include "Core/Core.h"

#include "Core/IO/String_fwd.h"
#include "Core/IO/TextWriter_fwd.h"

/*
// Format() usage samples :
// -------------------------
//
//ex1:
//      wchar_t buffer[1024];
//      size_t length = Format(buffer, L"string = {2}, decimal = {0}, float = {1}\n", "test", 42, 0.123456f);
//      std::cout << buffer;
//      ->  L"string = 0.123456, decimal = test, float = 42\n"
//
//ex2:
//      FWString wstr = Format(L"alphabool={0:A}", true);
//      ->  L"num=1 alphabool=true"
//
//ex2:
//      Format(std::cout, "string = {0:10} {0:-10U}, decimal = {1:8X} {1:#8x}, float = {2:f3} {2:10f4}\n", "test", 0xBADCAFE, -0.123456f);
//      ->  "string =       test test      , decimal =  BADCAFE 0badcafe, float = -0.123    -0.1235\n"
//
//ex3:
//      Format(std::cout, "{0*16}\n", '-');
//      ->  "----------------\n"
//      Format(std::cout, "{0:#4*4}\n", 42);
//      ->  "0042004200420042\n"
//
*/

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define INLINE_FORMAT(_CAPACITY, _FORMAT, ...) \
    ::Core::Format(INLINE_MALLOCA(char, _CAPACITY).MakeView(), (_FORMAT), __VA_ARGS__)
//----------------------------------------------------------------------------
#define INLINE_WFORMAT(_CAPACITY, _FORMAT, ...) \
    ::Core::Format(INLINE_MALLOCA(wchar_t, _CAPACITY).MakeView(), (_FORMAT), __VA_ARGS__)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
void Format(TMemoryView<_Char>& dst, const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
void Format(TBasicString<_Char>& dst, const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
void Format(TBasicTextWriter<_Char>& dst, const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
TBasicString<_Char> StringFormat(const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    TBasicString<_Char> result;
    Format(result, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, size_t _Dim2, typename _Arg0, typename... _Args>
void Format(_Char(&dst)[_Dim], const _Char(&format)[_Dim2], _Arg0&& arg0, _Args&&... args) {
    Format(MakeView(dst), MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
void Format(TMemoryView<_Char>& dst, const _Char(&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    Format(dst, MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
void Format(TBasicString<_Char>& dst, const _Char(&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    Format(dst, MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
void Format(TBasicTextWriter<_Char>& dst, const _Char(&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    Format(dst, MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
TBasicString<_Char> StringFormat(const _Char(&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    return StringFormat(MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/Format-inl.h"
