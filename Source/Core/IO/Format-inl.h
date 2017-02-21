#pragma once

#include "Core/IO/Format.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
struct TFormatFunctor_ {
    typedef void (*helper_type)(std::basic_ostream<_Char, _Traits>& , const void * );

    helper_type _helper;
    const void *_pArg;

    TFormatFunctor_(helper_type helper, const void *pArg)
        : _helper(helper), _pArg(pArg) {}

    template <typename _Value>
    static void FromValue(std::basic_ostream<_Char, _Traits>& oss, const void *pArg) {
        typedef Meta::TAddConst<_Value> value_type;
        oss << *reinterpret_cast<value_type *>(pArg); // operator <<() resolved through KDL
    }

    template <typename _Pointer>
    static void FromPointer(std::basic_ostream<_Char, _Traits>& oss, const void *pArg) {
        oss << reinterpret_cast<_Pointer>(pArg); // operator <<() resolved through KDL
    }

    template <typename T>
    static typename std::enable_if<
        not std::is_pointer<T>::value,
        TFormatFunctor_
    >::type Make(const T& value) {
        typedef Meta::TRemoveReference<T> value_type;
        return TFormatFunctor_(&FromValue<value_type>, &value);
    }

    template <typename T>
    static TFormatFunctor_ Make(const T* pointer) {
        return TFormatFunctor_(&FromPointer<const T*>, pointer);
    }
};
//----------------------------------------------------------------------------
void _FormatArgs(std::basic_ostream<char>& oss, const FStringView& format, const TMemoryView<const TFormatFunctor_<char>>& args);
void _FormatArgs(std::basic_ostream<wchar_t>& oss, const FWStringView& format, const TMemoryView<const TFormatFunctor_<wchar_t>>& args);
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
using TBasicFormatArgList = TMemoryView< const details::TFormatFunctor_<_Char, _Traits> >;
typedef TBasicFormatArgList<char>    FormatArgList;
typedef TBasicFormatArgList<wchar_t> FormatArgListW;
//----------------------------------------------------------------------------
template <typename _Char>
void FormatArgs(std::basic_ostream<_Char>& oss, const TBasicStringView<_Char>& format, const TBasicFormatArgList<_Char>& args) {
    details::_FormatArgs(oss, format, args);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Arg0, typename... _Args>
void Format(std::basic_ostream<_Char, _Traits>& oss, const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    // args are always passed by pointer, wrapped in a void *
    // this avoids unintended copies and decorrelates from actual types (_FormatArgs is defined in Format.cpp)
    const details::TFormatFunctor_<_Char, _Traits> functors[] = {
        details::TFormatFunctor_<_Char, _Traits>::Make(std::forward<_Arg0>(arg0)),
        details::TFormatFunctor_<_Char, _Traits>::Make(std::forward<_Args>(args))...
    };

    details::_FormatArgs(oss, format, MakeView(functors));
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
size_t Format(_Char* result, size_t capacity, const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    Assert(result);
    Assert(capacity);

    TBasicOCStrStream<_Char, std::char_traits<_Char>> oss(result, checked_cast<std::streamsize>(capacity));
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    oss.ForceEOS();

    const size_t n = checked_cast<size_t>(oss.size());
    Assert('\0' == result[n - 1]);
    return (n - 1); // skip EOS
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Arg0, typename... _Args>
void Format(TBasicString<_Char, _Traits>& result, const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    STACKLOCAL_BASICOCSTRSTREAM(_Char, oss, 2048);
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    result.assign(oss.Pointer(), checked_cast<size_t>(oss.size()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Arg>
FString ToString(_Arg&& arg) {
    STACKLOCAL_OCSTRSTREAM(oss, 2048);
    oss << arg;
    return FString(oss.Pointer(), checked_cast<size_t>(oss.size()) );
}
//----------------------------------------------------------------------------
template <typename _Arg>
FWString ToWString(_Arg&& arg) {
    STACKLOCAL_WOCSTRSTREAM(oss, 2048);
    oss << arg;
    return FWString(oss.Pointer(), checked_cast<size_t>(oss.size()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
