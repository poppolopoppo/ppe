#pragma once

#include "Core_fwd.h"

#include "IO/String_fwd.h"

#include "Container/Pair.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Meta/Optional.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicStringConversion {
    using stringliteral_type = TBasicStringLiteral<_Char>;
    using stringview_type = TBasicStringView<_Char>;
    using iterator = typename stringview_type::iterator;

    stringview_type Input;
    u32 Base{ 10 };

    TBasicStringConversion() = default;

    TBasicStringConversion(stringview_type input) NOEXCEPT : Input(input) {}
    TBasicStringConversion(stringview_type input, u32 base) NOEXCEPT : Input(input), Base(base) {}
    TBasicStringConversion& operator =(stringview_type input) NOEXCEPT {
        Input = input;
        return (*this);
    }

    TBasicStringConversion(stringliteral_type input) NOEXCEPT : TBasicStringConversion(input.MakeView()) {}
    TBasicStringConversion(stringliteral_type input, u32  base) NOEXCEPT : TBasicStringConversion(input.MakeView(), base) {}
    TBasicStringConversion& operator =(stringliteral_type input) NOEXCEPT {
        Input = input.empty();
        return (*this);
    }

    TBasicStringConversion(iterator first, iterator last) NOEXCEPT : Input(first, last) {}
    TBasicStringConversion(const TPair<iterator, iterator>& span) NOEXCEPT : Input(span) {}

    NODISCARD bool empty() const { return Input.empty(); }

    NODISCARD bool operator >>(bool* dst) const NOEXCEPT {
        if (EqualsI(Input, MakeStringView(STRING_LITERAL(_Char, "true"))) ||
            EqualsI(Input, MakeStringView(STRING_LITERAL(_Char, "1"))) ) {
            *dst = true;
            return true;
        }
        if (EqualsI(Input, MakeStringView(STRING_LITERAL(_Char, "false"))) ||
            EqualsI(Input, MakeStringView(STRING_LITERAL(_Char, "0"))) ) {
            *dst = false;
            return true;
        }
        return false;
    }

    NODISCARD bool operator >>(i32* dst) const NOEXCEPT { return Atoi(dst, Input, Base); }
    NODISCARD bool operator >>(i64* dst) const NOEXCEPT { return Atoi(dst, Input, Base); }

    NODISCARD bool operator >>(u32* dst) const NOEXCEPT { return Atoi(dst, Input, Base); }
    NODISCARD bool operator >>(u64* dst) const NOEXCEPT { return Atoi(dst, Input, Base); }

    NODISCARD bool operator >>(i8* dst) const NOEXCEPT { return CheckedIntConversion_(dst); }
    NODISCARD bool operator >>(i16* dst) const NOEXCEPT { return CheckedIntConversion_(dst); }
    NODISCARD bool operator >>(u8* dst) const NOEXCEPT { return CheckedIntConversion_(dst); }
    NODISCARD bool operator >>(u16* dst) const NOEXCEPT { return CheckedIntConversion_(dst); }

    NODISCARD bool operator >>(float* dst) const NOEXCEPT { return Atof(dst, Input); }
    NODISCARD bool operator >>(double* dst) const NOEXCEPT { return Atod(dst, Input); }

    NODISCARD bool operator >>(stringview_type* dst) const NOEXCEPT { *dst = Input; return true; }

    template <typename T>
    using if_has_stringconversion_t = decltype(std::declval<const TBasicStringConversion&>() >> std::declval<T*>());

    template <typename T, if_has_stringconversion_t<T>* = nullptr >
    NODISCARD bool operator >>(Meta::TOptional<T>* dst) const NOEXCEPT {
        T tmp{ Default };
        if (operator >>(&tmp)) {
            dst->emplace(std::move(tmp));
            return true;
        }
        return false;
    }

    template <typename T, if_has_stringconversion_t<T>* = nullptr >
    NODISCARD T ConvertTo() const {
        T result = Meta::MakeForceInit<T>();
        VerifyRelease( ConvertTo(&result) );
        return result;
    }

    template <typename T, if_has_stringconversion_t<T>* = nullptr >
    NODISCARD bool ConvertTo(T* dst) const {
        Assert(dst);
        return (*this >> dst); // Can be expanded thanks to ADL
    }

    template <typename T>
    NODISCARD bool ConvertToIFP(T* dst) const {
        Assert(dst);
        IF_CONSTEXPR(Meta::has_defined_v<if_has_stringconversion_t, T>) {
            return (*this >> dst); // Can be expanded thanks to ADL
        }
        else {
            return false; // Unsupported type
        }
    }

private:
    template <typename _Int>
    NODISCARD bool CheckedIntConversion_(_Int* dst) const NOEXCEPT {
        Meta::TConditional<std::is_signed_v<_Int>, i64, u64> tmp;
        if (Atoi(&tmp, Input, Base)) {
            *dst = checked_cast<_Int>(tmp);
            return true;
        }
        return false;
    }
};
//----------------------------------------------------------------------------
template <typename _Char>
TBasicStringConversion(TBasicStringLiteral<_Char>) NOEXCEPT -> TBasicStringConversion<_Char>;
template <typename _Char>
TBasicStringConversion(TBasicStringView<_Char>) NOEXCEPT -> TBasicStringConversion<_Char>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
