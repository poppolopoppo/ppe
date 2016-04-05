#pragma once

#include "Core/IO/Format.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits = std::char_traits<_Char> >
struct _FormatFunctor {
    typedef void (*helper_type)(std::basic_ostream<_Char, _Traits>& , const void * );

    helper_type _helper;
    const void *_pArg;

    _FormatFunctor(helper_type helper, const void *pArg)
        : _helper(helper), _pArg(pArg) {}

    template <typename _Value>
    static void FromValue(std::basic_ostream<_Char, _Traits>& oss, const void *pArg) {
        typedef typename std::add_const<_Value>::type value_type;
        oss << *reinterpret_cast<value_type *>(pArg); // operator <<() resolved through KDL
    }

    template <typename _Pointer>
    static void FromPointer(std::basic_ostream<_Char, _Traits>& oss, const void *pArg) {
        oss << reinterpret_cast<_Pointer>(pArg); // operator <<() resolved through KDL
    }

    template <typename T>
    static typename std::enable_if<
        not std::is_pointer<T>::value,
        _FormatFunctor
    >::type Make(const T& value) {
        typedef typename std::remove_reference<T>::type value_type;
        return _FormatFunctor(&FromValue<value_type>, &value);
    }

    template <typename T>
    static _FormatFunctor Make(const T* pointer) {
        return _FormatFunctor(&FromPointer<const T*>, pointer);
    }
};
//----------------------------------------------------------------------------
void _FormatArgs(std::basic_ostream<char>& oss, const StringSlice& format, const MemoryView<const _FormatFunctor<char>>& args);
void _FormatArgs(std::basic_ostream<wchar_t>& oss, const WStringSlice& format, const MemoryView<const _FormatFunctor<wchar_t>>& args);
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Arg0, typename... _Args>
void Format(std::basic_ostream<_Char, _Traits>& oss, const BasicStringSlice<_Char>& format, _Arg0&& arg0, _Args&&... args) {

    // args are always passed by pointer, wrapped in a void *
    // this avoids unintended copies and decorrelates from actual types (_FormatArgs is defined in Format.cpp)
    const details::_FormatFunctor<_Char, _Traits> functors[] = {
        details::_FormatFunctor<_Char, _Traits>::Make(std::forward<_Arg0>(arg0)),
        details::_FormatFunctor<_Char, _Traits>::Make(std::forward<_Args>(args))...
    };

    details::_FormatArgs(oss, format, MakeView(functors));
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
size_t Format(_Char* result, size_t capacity, const BasicStringSlice<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    Assert(result);
    Assert(capacity);

    BasicOCStrStream<_Char, std::char_traits<_Char>> oss(result, checked_cast<std::streamsize>(capacity));
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    oss.ForceEOS();

    return checked_cast<size_t>(oss.size());
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename _Arg0, typename... _Args>
void Format(BasicString<_Char, _Traits>& result, const BasicStringSlice<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    STACKLOCAL_BASICOCSTRSTREAM(_Char, oss, 2048);
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    result.assign(oss.Pointer(), checked_cast<size_t>(oss.size()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Arg>
String ToString(_Arg&& arg) {
    STACKLOCAL_OCSTRSTREAM(oss, 2048);
    oss << arg;
    return String(oss.Pointer(), checked_cast<size_t>(oss.size()) );
}
//----------------------------------------------------------------------------
template <typename _Arg>
WString ToWString(_Arg&& arg) {
    STACKLOCAL_WOCSTRSTREAM(oss, 2048);
    oss << arg;
    return WString(oss.Pointer(), checked_cast<size_t>(oss.size()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
