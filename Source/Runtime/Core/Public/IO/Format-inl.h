#pragma once

#include "StreamProvider.h"

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

    static TBasicFormatFunctor_ Make(const _Char* cstr) {
        return TBasicFormatFunctor_{
            [](TBasicTextWriter<_Char>& oss, const void *arg) {
                oss << MakeCStringView(static_cast<const _Char*>(arg));
            },
            cstr };
    }
};
//----------------------------------------------------------------------------
using FFormatFunctor_ = TBasicFormatFunctor_<char>;
using FWFormatFunctor_ = TBasicFormatFunctor_<wchar_t>;
//----------------------------------------------------------------------------
NODISCARD PPE_CORE_API bool UnsafeFormatArgs_(FTextWriter& oss, FConstChar unsafeFormat, TMemoryView<const FFormatFunctor_> args);
NODISCARD PPE_CORE_API bool UnsafeFormatArgs_(FWTextWriter& oss, FConstWChar unsafeFormat, TMemoryView<const FWFormatFunctor_> args);
//----------------------------------------------------------------------------
PPE_CORE_API void FormatArgs_(FTextWriter& oss, FStringLiteral format, TMemoryView<const FFormatFunctor_> args);
PPE_CORE_API void FormatArgs_(FWTextWriter& oss, FWStringLiteral format, TMemoryView<const FWFormatFunctor_> args);
//----------------------------------------------------------------------------
PPE_CORE_API void FormatRecord_(FTextWriter& oss, TMemoryView<const FFormatFunctor_> record);
PPE_CORE_API void FormatRecord_(FWTextWriter& oss, TMemoryView<const FWFormatFunctor_> record);
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
void FormatArgs(TBasicTextWriter<_Char>& oss, TBasicStringLiteral<_Char> format, TBasicFormatArgList<_Char> args) {
    details::FormatArgs_(oss, format, args);
}
//----------------------------------------------------------------------------
template <typename _Char>
NODISCARD bool UnsafeFormatArgs(TBasicTextWriter<_Char>& oss, TBasicConstChar<_Char> unsafeFormat, TBasicFormatArgList<_Char> args) {
    return details::UnsafeFormatArgs_(oss, unsafeFormat, args);
}
//----------------------------------------------------------------------------
template <typename _Char>
void FormatRecord(TBasicTextWriter<_Char>& oss, const TBasicFormatArgList<_Char>& record) {
    details::FormatRecord_(oss, record);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
TBasicTextWriter<_Char>& Format(TBasicTextWriter<_Char>& oss, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args) {
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
size_t Format(const TMemoryView<_Char>& dst, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args) {
    Assert(not dst.empty());

    TBasicFixedSizeTextWriter<_Char> oss(dst);
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);
    oss.NullTerminated();

    return (oss.size() - 1);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
size_t StreamFormat(IStreamWriter& dst, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args) {
    const std::streamoff startOff = dst.TellO();

    TBasicTextWriter<_Char> oss{ &dst };
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    return checked_cast<size_t>(dst.TellO() - startOff);
}
//----------------------------------------------------------------------------
template <typename _Char, typename _Arg0, typename... _Args>
void Format(TBasicString<_Char>& result, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args) {
    TBasicStringBuilder<_Char> oss(std::move(result));

    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    oss.ToString(result);
}
//----------------------------------------------------------------------------
template <typename _Char, size_t _Capacity, typename _Arg0, typename... _Args>
const _Char* Format(TBasicStaticString<_Char, _Capacity>& dst, TBasicStringLiteral<_Char> format, _Arg0&& arg0, _Args&&... args) {
    STATIC_ASSERT(_Capacity > 0);

    TBasicFixedSizeTextWriter<_Char> oss(dst.Buf());
    Format(oss, format, std::forward<_Arg0>(arg0), std::forward<_Args>(args)...);

    dst.Resize(oss.size(), true);
    return dst.NullTerminated();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
CONSTEXPR bool TBasicFormatTraits<_Char>:: IsValidManipFlag(_Char ch) {
    switch (ch) {
    case multiply:
    case zero:
    case fmt_alpha:
    case fmt_ALPHA:
    case fmt_bin:
    case fmt_BIN:
    case fmt_dec:
    case fmt_DEC:
    case fmt_hex:
    case fmt_HEX:
    case fmt_oct:
    case fmt_OCT:
    case fmt_fixed:
    case fmt_FIXED:
    case fmt_scient:
    case fmt_SCIENT:
    case fmt_UPPER:
    case fmt_lower:
    case fmt_Capital:
    case fmt_minus:
    case fmt_compact:
    case fmt_NONCOMPACT:
    case fmt_zeropad:
    case fmt_center:
    case fmt_truncR:
    case fmt_truncL:
    case fmt_escape:
    case fmt_quote:
        return true;
    default:
        return false;
    }
}
//----------------------------------------------------------------------------
namespace details {
template <typename _Char>
constexpr EValidateFormat ValidateFormatManip_(_Char ch) noexcept {
    switch (ch) {
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
        return EValidateFormat::Valid;

    default:
        using traits_type = TBasicFormatTraits<_Char>;
        return (traits_type::IsValidManipFlag(ch)
            ? EValidateFormat::Valid
            : EValidateFormat::InvalidFormatManip);
    }
}
template <typename _Char>
constexpr EValidateFormat ValidateFormatString_(const _Char* fmt, size_t len, size_t numArgs) noexcept {
    if (numArgs > 10) // not handled by validation, but can actually be handled by Format()
        return EValidateFormat::TooManyArguments;

    size_t lastArgIndex = size_t(-1);
    size_t unusedArgs = ((size_t(1) << numArgs) - 1);
    for (size_t i = 0; i <= len - 2; ++i) {
        if (fmt[i] == STRING_LITERAL(_Char, '\0')) return EValidateFormat::InvalidFormatString;
        if (fmt[i] == STRING_LITERAL(_Char, '{')) {
            size_t argIndex;
            if (fmt[i + 1] >= STRING_LITERAL(_Char, '0') && fmt[i + 1] <= STRING_LITERAL(_Char, '9')) {
                argIndex = (size_t(fmt[i + 1]) - STRING_LITERAL(_Char, '0'));
                i += 1;
            }
            else {
                argIndex = (lastArgIndex + 1);
            }
            lastArgIndex = argIndex;

            if (argIndex >= numArgs)
                return EValidateFormat::ArgumentOutOfBounds; // argument index out of bounds

            switch (fmt[i + 1]) {
            case STRING_LITERAL(_Char, '}'): // short format
                i += 1;
                break;
            case STRING_LITERAL(_Char, ':'): // validate long format
                i += 2;
                while (i < len && fmt[i] != STRING_LITERAL(_Char, '}')) {
                    if (const EValidateFormat error = ValidateFormatManip(fmt[i++]); error != EValidateFormat::Valid)
                        return error;
                }
                break;
            default: // invalid format (assuming 1 digit)
                continue; // could be a user '{', ignoring
            }

            if (fmt[i] == STRING_LITERAL(_Char, '}')) // ignore unclosed/invalid format clauses
                unusedArgs &= ~(size_t(1) << argIndex);
        }
    }
    return (0 == unusedArgs
        ? EValidateFormat::Valid
        : EValidateFormat::UnusedArguments); // each arg should be used at least once
}
} //!details
//----------------------------------------------------------------------------
constexpr EValidateFormat ValidateFormatManip(char ch) noexcept {
    return details::ValidateFormatManip_(ch);
}
//----------------------------------------------------------------------------
constexpr EValidateFormat ValidateFormatManip(wchar_t ch) noexcept {
    return details::ValidateFormatManip_(ch);
}
//----------------------------------------------------------------------------
constexpr EValidateFormat ValidateFormatString(const char* fmt, size_t len, size_t numArgs) noexcept {
    return details::ValidateFormatString_(fmt, len, numArgs);
}
//----------------------------------------------------------------------------
constexpr EValidateFormat ValidateFormatString(const wchar_t* fmt, size_t len, size_t numArgs) noexcept {
    return details::ValidateFormatString_(fmt, len, numArgs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
