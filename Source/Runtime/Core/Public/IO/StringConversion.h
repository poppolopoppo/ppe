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
    using stringview_type = TBasicStringView<_Char>;
    using iterator = typename stringview_type::iterator;

    using EBase = FTextFormat::EBase;

    stringview_type Input;
    EBase Base{ FTextFormat::Decimal };

    TBasicStringConversion() = default;

    TBasicStringConversion(stringview_type input) NOEXCEPT : Input(input) {}
    TBasicStringConversion& operator =(stringview_type input) NOEXCEPT {
        Input = input;
        return (*this);
    }

    TBasicStringConversion(iterator first, iterator last) NOEXCEPT : Input(first, last) {}
    TBasicStringConversion(const TPair<iterator, iterator>& span) NOEXCEPT : Input(span) {}

    bool operator >>(bool* dst) const NOEXCEPT {
        if (EqualsI(Input, STRING_LITERAL(_Char, "true")) ||
            EqualsI(Input, STRING_LITERAL(_Char, "1")) ) {
            *dst = true;
            return true;
        }
        if (EqualsI(Input, STRING_LITERAL(_Char, "false")) ||
            EqualsI(Input, STRING_LITERAL(_Char, "0")) ) {
            *dst = false;
            return true;
        }
        return false;
    }

    bool operator >>(i32* dst) const NOEXCEPT { return Atoi(dst, Input, EBase_ToInt(Base)); }
    bool operator >>(i64* dst) const NOEXCEPT { return Atoi(dst, Input, EBase_ToInt(Base)); }

    bool operator >>(u32* dst) const NOEXCEPT { return Atoi(dst, Input, EBase_ToInt(Base)); }
    bool operator >>(u64* dst) const NOEXCEPT { return Atoi(dst, Input, EBase_ToInt(Base)); }

    bool operator >>(i8* dst) const NOEXCEPT { return CheckedIntConversion_(dst); }
    bool operator >>(i16* dst) const NOEXCEPT { return CheckedIntConversion_(dst); }
    bool operator >>(u8* dst) const NOEXCEPT { return CheckedIntConversion_(dst); }
    bool operator >>(u16* dst) const NOEXCEPT { return CheckedIntConversion_(dst); }

    bool operator >>(float* dst) const NOEXCEPT { return Atof(dst, Input); }
    bool operator >>(double* dst) const NOEXCEPT { return Atod(dst, Input); }

    bool operator >>(stringview_type* dst) const NOEXCEPT { *dst = Input; return true; }

    template <typename T>
    bool operator >>(Meta::TOptional<T>* dst) const NOEXCEPT {
        T tmp{ Default };
        if (operator >>(&tmp)) {
            dst->emplace(std::move(tmp));
            return true;
        }
        return false;
    }

    template <typename _Numeric>
    _Numeric To() const {
        _Numeric result{ Default };
        VerifyRelease( operator >>(&result) );
        return result;
    }

private:
    template <typename _Int>
    bool CheckedIntConversion_(_Int* dst) const NOEXCEPT {
        Meta::TConditional<std::is_signed_v<_Int>, i64, u64> tmp;
        if (Atoi(&tmp, Input, EBase_ToInt(Base))) {
            *dst = checked_cast<_Int>(tmp);
            return true;
        }
        return false;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE