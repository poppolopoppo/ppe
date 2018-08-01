#pragma once

#include "IO/Format.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicFormatFunctor_ {
    typedef void (*helper_type)(TBasicTextWriter<_Char>& , const void * );

    helper_type Helper;
    const void *Arg;

    TBasicFormatFunctor_(helper_type helper, const void *arg)
        : Helper(helper), Arg(arg) {}

    template <typename _Value>
    static void FromValue(TBasicTextWriter<_Char>& oss, const void *arg) {
        typedef Meta::TAddConst<_Value> value_type;
        oss << *reinterpret_cast<value_type*>(arg); // operator <<() resolved through ADL
    }

    template <typename _Pointer>
    static void FromPointer(TBasicTextWriter<_Char>& oss, const void *arg) {
        oss << reinterpret_cast<_Pointer>(arg); // operator <<() resolved through ADL
    }

    template <typename T>
    static typename std::enable_if<
        not std::is_pointer<T>::value,
        TBasicFormatFunctor_
    >::type Make(const T& value) {
        typedef Meta::TRemoveReference<T> value_type;
        return TBasicFormatFunctor_(&FromValue<value_type>, &value);
    }

    template <typename T>
    static TBasicFormatFunctor_ Make(const T* pointer) {
        return TBasicFormatFunctor_(&FromPointer<const T*>, pointer);
    }
};
//----------------------------------------------------------------------------
using FFormatFunctor_ = TBasicFormatFunctor_<char>;
using FWFormatFunctor_ = TBasicFormatFunctor_<wchar_t>;
//----------------------------------------------------------------------------
PPE_API void FormatArgs_(FTextWriter& oss, const FStringView& format, const TMemoryView<const FFormatFunctor_>& args);
PPE_API void FormatArgs_(FWTextWriter& oss, const FWStringView& format, const TMemoryView<const FWFormatFunctor_>& args);
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
void FormatArgs(TBasicTextWriter<_Char>& oss, const TBasicStringView<_Char>& format, const TBasicFormatArgList<_Char>& args) {
    details::FormatArgs_(oss, format, args);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
TBasicTextWriter<_Char>& Format(TBasicTextWriter<_Char>& oss, const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    // args are always passed by pointer, wrapped in a void *
    // this avoids unintended copies and de-correlates from actual types (_FormatArgs is defined in Format.cpp)
    typedef details::TBasicFormatFunctor_<_Char> formatfunc_type;
    const formatfunc_type functors[] = {
        formatfunc_type::Make(std::forward<_Arg0>(arg0)),
        formatfunc_type::Make(std::forward<_Args>(args))...
    };

    details::FormatArgs_(oss, format, MakeView(functors));

    return oss;
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
size_t Format(const TMemoryView<_Char>& dst, const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    Assert(not dst.empty());

    TBasicFixedSizeTextWriter<_Char> oss(dst);
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    oss << Eos;

    return oss.size();
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
void Format(TBasicString<_Char>& result, const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    TBasicStringBuilder<_Char> oss(std::move(result));

    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    oss.ToString(result);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
