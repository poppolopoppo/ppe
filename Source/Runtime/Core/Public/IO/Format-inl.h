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
PPE_CORE_API void FormatArgs_(FTextWriter& oss, const FStringView& format, const TMemoryView<const FFormatFunctor_>& args);
PPE_CORE_API void FormatArgs_(FWTextWriter& oss, const FWStringView& format, const TMemoryView<const FWFormatFunctor_>& args);
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
constexpr bool ValidateFormatManip(char ch) noexcept {
    switch (ch) {
    case '*':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case 'a':
    case 'A':
    case 'b':
    case 'B':
    case 'd':
    case 'D':
    case 'x':
    case 'X':
    case 'o':
    case 'O':
    case 'f':
    case 'F':
    case 's':
    case 'S':
    case 'U':
    case 'l':
    case 'C':
    case '-':
    case '#':
    case '@':
    case '/':
        return true;
    default:
        return false;
    }
}
//----------------------------------------------------------------------------
constexpr bool ValidateFormatManip(wchar_t ch) noexcept {
    switch (ch) {
    case L'*':
    case L'0':
    case L'1':
    case L'2':
    case L'3':
    case L'4':
    case L'5':
    case L'6':
    case L'7':
    case L'8':
    case L'9':
    case L'a':
    case L'A':
    case L'b':
    case L'B':
    case L'd':
    case L'D':
    case L'x':
    case L'X':
    case L'o':
    case L'O':
    case L'f':
    case L'F':
    case L's':
    case L'S':
    case L'U':
    case L'l':
    case L'C':
    case L'-':
    case L'#':
    case L'@':
    case L'/':
        return true;
    default:
        return false;
    }
}
//----------------------------------------------------------------------------
constexpr bool ValidateFormatString(const char* fmt, size_t len, size_t numArgs) noexcept {
    size_t unusedArgs = ((1ul << numArgs) - 1);
    for (size_t i = 0; i < len - 2; ++i) {
        if (fmt[i] == '\0') return false;
        if (fmt[i] == '{' && fmt[i + 1] >= '0' && fmt[i + 1] <= '9') {
            const size_t argIndex = (fmt[i + 1] - '0');
            if (argIndex >= numArgs)
                return false; // argument index out of bounds

            switch (fmt[i + 2]) {
            case '}': // short format
                i += 2;
                break;
            case ':': // validate long format
                i += 3;
                while (i < len && fmt[i] != '}')
                    if (not ValidateFormatManip(fmt[i++]))
                        return false;
                break;
            default: // invalid format (assuming 1 digit)
                continue; // could be a user '{', ignoring
            }

            if (fmt[i] == '}') // ignore unclosed/invalid format clauses
                unusedArgs &= ~(size_t(1) << argIndex);
        }
    }
    return (0 == unusedArgs); // each arg should be used at least once
}
//----------------------------------------------------------------------------
constexpr bool ValidateFormatString(const wchar_t* fmt, size_t len, size_t numArgs) noexcept {
    size_t unusedArgs = ((1ul << numArgs) - 1);
    for (size_t i = 0; i < len - 2; ++i) {
        if (fmt[i] == L'\0') return false;
        if (fmt[i] == L'{' && fmt[i + 1] >= L'0' && fmt[i + 1] <= L'9') {
            const size_t argIndex = (fmt[i + 1] - L'0');
            if (argIndex >= numArgs)
                return false; // argument index out of bounds

            switch (fmt[i + 2]) {
            case L'}': // short format
                i += 2;
                break;
            case L':': // validate long format
                i += 3;
                while (i < len && fmt[i] != L'}')
                    if (not ValidateFormatManip(fmt[i++]))
                        return false;
                break;
            default: // invalid format (assuming 1 digit)
                continue; // could be a user '{', ignoring
            }

            if (fmt[i] == L'}') // ignore unclosed/invalid format clauses
                unusedArgs &= ~(size_t(1) << argIndex);
        }
    }
    return (0 == unusedArgs); // each arg should be used at least once
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
