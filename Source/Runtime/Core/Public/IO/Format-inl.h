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
    static Meta::TEnableIf<not std::is_pointer_v<T>, TBasicFormatFunctor_> Make(const T& value) {
        typedef Meta::TRemoveReference<T> value_type;
        return TBasicFormatFunctor_{ &FromValue<value_type>, &value };
    }

    template <typename T>
    static TBasicFormatFunctor_ Make(const T* pointer) {
        return TBasicFormatFunctor_{ &FromPointer<const T*>, pointer };
    }
};
//----------------------------------------------------------------------------
using FFormatFunctor_ = TBasicFormatFunctor_<char>;
using FWFormatFunctor_ = TBasicFormatFunctor_<wchar_t>;
//----------------------------------------------------------------------------
PPE_CORE_API void FormatArgs_(FTextWriter& oss, const FStringView& format, const TMemoryView<const FFormatFunctor_>& args);
PPE_CORE_API void FormatArgs_(FWTextWriter& oss, const FWStringView& format, const TMemoryView<const FWFormatFunctor_>& args);
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename T>
auto MakeFormatArg(const T& arg) {
    typedef details::TBasicFormatFunctor_<_Char> formatfunc_type;
    return formatfunc_type::Make(arg);
}
//----------------------------------------------------------------------------
template <typename _Char>
auto MakeFormatLambda(typename details::template TBasicFormatFunctor_<_Char>::helper_type fmt, const void* arg) {
    typedef details::TBasicFormatFunctor_<_Char> formatfunc_type;
    return formatfunc_type{ fmt, arg };
}
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
template <typename _Char, size_t _Capacity, typename _Arg0, typename... _Args>
const _Char* Format(TBasicStaticString<_Char, _Capacity>& dst, const TBasicStringView<_Char>& format, _Arg0&& arg0, _Args&&... args) {
    STATIC_ASSERT(_Capacity > 0);

    TBasicFixedSizeTextWriter<_Char> oss(dst.Buf());
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    dst.Resize(oss.size(), true);
    return dst.NullTerminated();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename _Char>
constexpr bool ValidateFormatManip_(_Char ch) noexcept {
    switch (ch) {
    case STRING_LITERAL(_Char, '*'):
    case STRING_LITERAL(_Char, '0'):
    case STRING_LITERAL(_Char, '1'):
    case STRING_LITERAL(_Char, '2'):
    case STRING_LITERAL(_Char, '3'):
    case STRING_LITERAL(_Char, '4'):
    case STRING_LITERAL(_Char, '5'):
    case STRING_LITERAL(_Char, '6'):
    case STRING_LITERAL(_Char, '7'):
    case STRING_LITERAL(_Char, '8'):
    case STRING_LITERAL(_Char, '9'):
    case STRING_LITERAL(_Char, 'a'):case STRING_LITERAL(_Char, 'A'):
    case STRING_LITERAL(_Char, 'b'):case STRING_LITERAL(_Char, 'B'):
    case STRING_LITERAL(_Char, 'd'):case STRING_LITERAL(_Char, 'D'):
    case STRING_LITERAL(_Char, 'x'):case STRING_LITERAL(_Char, 'X'):
    case STRING_LITERAL(_Char, 'o'):case STRING_LITERAL(_Char, 'O'):
    case STRING_LITERAL(_Char, 'f'):case STRING_LITERAL(_Char, 'F'):
    case STRING_LITERAL(_Char, 's'):case STRING_LITERAL(_Char, 'S'):
    case STRING_LITERAL(_Char, 'U'):
    case STRING_LITERAL(_Char, 'l'):
    case STRING_LITERAL(_Char, 'C'):
    case STRING_LITERAL(_Char, '-'):
    case STRING_LITERAL(_Char, '#'):
    case STRING_LITERAL(_Char, '@'):
    case STRING_LITERAL(_Char, '/'):
        return true;
    default:
        return false;
    }
}
template <typename _Char>
constexpr bool ValidateFormatString_(const _Char* fmt, size_t len, size_t numArgs) noexcept {
    size_t unusedArgs = ((size_t(1) << numArgs) - 1);
    for (size_t i = 0; i < len - 2; ++i) {
        if (fmt[i] == STRING_LITERAL(_Char, '\0')) return false;
        if (fmt[i] == STRING_LITERAL(_Char, '{') && fmt[i + 1] >= STRING_LITERAL(_Char, '0') && fmt[i + 1] <= STRING_LITERAL(_Char, '9')) {
            const size_t argIndex = (size_t(fmt[i + 1]) - STRING_LITERAL(_Char, '0'));
            if (argIndex >= numArgs)
                return false; // argument index out of bounds

            switch (fmt[i + 2]) {
            case STRING_LITERAL(_Char, '}'): // short format
                i += 2;
                break;
            case STRING_LITERAL(_Char, ':'): // validate long format
                i += 3;
                while (i < len && fmt[i] != STRING_LITERAL(_Char, '}'))
                    if (not ValidateFormatManip(fmt[i++]))
                        return false;
                break;
            default: // invalid format (assuming 1 digit)
                continue; // could be a user '{', ignoring
            }

            if (fmt[i] == STRING_LITERAL(_Char, '}')) // ignore unclosed/invalid format clauses
                unusedArgs &= ~(size_t(1) << argIndex);
        }
    }
    return (0 == unusedArgs); // each arg should be used at least once
}
} //!details
//----------------------------------------------------------------------------
constexpr bool ValidateFormatManip(char ch) noexcept {
    return details::ValidateFormatManip_(ch);
}
//----------------------------------------------------------------------------
constexpr bool ValidateFormatManip(wchar_t ch) noexcept {
    return details::ValidateFormatManip_(ch);
}
//----------------------------------------------------------------------------
constexpr bool ValidateFormatString(const char* fmt, size_t len, size_t numArgs) noexcept {
    return details::ValidateFormatString_(fmt, len, numArgs);
}
//----------------------------------------------------------------------------
constexpr bool ValidateFormatString(const wchar_t* fmt, size_t len, size_t numArgs) noexcept {
    return details::ValidateFormatString_(fmt, len, numArgs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
