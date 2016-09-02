#pragma once

#include "Core/Core.h"

#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

#include <iosfwd>

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
//      WString wstr = Format(L"alphabool={0:A}", true);
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
template <typename _Char, typename _Traits = std::char_traits<_Char>, typename _Arg0, typename... _Args>
void Format(std::basic_ostream<_Char, _Traits>& oss, const BasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Traits = std::char_traits<_Char>, typename _Arg0, typename... _Args>
void Format(std::basic_ostream<_Char, _Traits>& oss, const _Char (&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    Format(oss, MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
size_t Format(_Char* result, size_t capacity, const BasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
size_t Format(_Char* result, size_t capacity, const _Char (&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    return Format(result, capacity, MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char>, typename _Arg0, typename... _Args>
void Format(BasicString<_Char, _Traits>& result, const BasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args);
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Traits = std::char_traits<_Char>, typename _Arg0, typename... _Args>
void Format(BasicString<_Char, _Traits>& result, const _Char (&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    Format(result, MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Arg0, typename... _Args>
size_t Format(_Char(&staticArray)[_Dim], const BasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    return Format(&staticArray[0], _Dim, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, size_t _Dim2, typename _Arg0, typename... _Args>
size_t Format(_Char(&staticArray)[_Dim], const _Char (&format)[_Dim2], _Arg0&& arg0, _Args&&... args) {
    return Format(&staticArray[0], _Dim, MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char>, typename _Arg0, typename... _Args>
BasicString<_Char, _Traits> StringFormat(const BasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    BasicString<_Char, _Traits> result;
    Format(result, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    return result;
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Dim, typename _Traits = std::char_traits<_Char>, typename _Arg0, typename... _Args>
BasicString<_Char, _Traits> StringFormat(const _Char (&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
    BasicString<_Char, _Traits> result;
    Format(result, MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Arg>
String ToString(_Arg&& arg);
//----------------------------------------------------------------------------
template <typename _Arg>
WString ToWString(_Arg&& arg);
//----------------------------------------------------------------------------
inline String ToString(bool b) { return (b ? "true" : "false"); }
inline WString ToWString(bool b) { return (b ? L"true" : L"false"); }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity = 64>
class StaticFormat {
public:
    STATIC_ASSERT(std::is_pod<_Char>::value);

    template <typename _Arg0, typename... _Args>
    StaticFormat(const BasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
        _length = Format(_c_str, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    }

    template <size_t _Dim, typename _Arg0, typename... _Args>
    StaticFormat(const _Char (&format)[_Dim], _Arg0&& arg0, _Args&&... args) {
        _length = Format(_c_str, MakeStringView(format), std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    }

    StaticFormat(const StaticFormat& ) = delete;
    StaticFormat& operator =(const StaticFormat& ) = delete;

    StaticFormat(StaticFormat&& ) = delete;
    StaticFormat& operator =(StaticFormat&& ) = delete;

    MemoryView<const _Char> MakeView() const { return MemoryView<const _Char>(_c_str, _length); }

private:
    size_t _length;
    _Char _c_str[_Capacity];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/IO/Format-inl.h"
