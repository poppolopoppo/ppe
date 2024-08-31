#pragma once

#include "Container/Stack.h"

#include "Meta/Enum.h"
#include "Meta/ForRange.h"

#include "Memory/MemoryView.h"
#include "IO/StringView.h"
#include "IO/TextReader_fwd.h"
#include "IO/TextWriter_fwd.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct enum_info_t {
    TMemoryView<const u64> values;
    TMemoryView<const FStringView> names;
    size_t num{ 0 };

    NODISCARD constexpr size_t index_from_value(u64 value) const {
        return IndexOf(values, value);
    }

    NODISCARD constexpr size_t index_from_name(const FStringView& name) const {
        return IndexOf(names, name);
    }

    NODISCARD constexpr size_t index_from_name_i(const FStringView& name) const {
        const auto it = names.FindIf([&](const FStringView& other) -> bool {
            return EqualsI(name, other);
        });
        return std::distance(names.begin(), it);
    }

    NODISCARD constexpr FStringView value_to_name(u64 value) const {
        const size_t id = index_from_value(value);
        return (id < num ? names[id] : FStringView{});
    }

    template <typename T, std::enable_if_t<std::is_enum_v<T>>* = nullptr>
    NODISCARD constexpr size_t index_from_value(T value) const {
        return index_from_value(static_cast<u64>(value));
    }

    template <typename T, std::enable_if_t<std::is_enum_v<T>>* = nullptr>
    NODISCARD constexpr FStringView value_to_name(T value) const {
        return value_to_name(static_cast<u64>(value));
    }

    template <typename _Char>
    NODISCARD bool read_index(TBasicTextReader<_Char>& iss, size_t* pIndex) const {
        TFixedSizeStack<_Char, 50> tmp;
        if (iss.ReadIdentifier(MakeAppendable(tmp))) {
            *pIndex = index_from_name_i(tmp.MakeView());
            return (*pIndex < num);
        }
        return false;
    }

    template <typename _Char>
    TBasicTextWriter<_Char>& write_value(TBasicTextWriter<_Char>& oss, u64 value) const {
        return oss << value_to_name(value);
    }

    template <typename _Char, typename T, std::enable_if_t<std::is_enum_v<T>>* = nullptr>
    NODISCARD bool read(TBasicTextReader<_Char>& iss, T* pValue) const {
        if (size_t id; read_index(iss, &id)) {
            *pValue = static_cast<T>(values[id]);
            return true;
        }
        return false;
    }

    template <typename _Char, typename T, std::enable_if_t<std::is_enum_v<T>>* = nullptr>
    TBasicTextWriter<_Char>& write(TBasicTextWriter<_Char>& oss, T value) const {
        return write_value(oss, static_cast<u64>(value));
    }
};
//----------------------------------------------------------------------------
struct autoenum_value_t {
    u64 value;

    autoenum_value_t() = default;

    template <typename T, std::enable_if_t<std::is_enum_v<T>>* = nullptr>
    constexpr autoenum_value_t(T enum_value)
    :   value(static_cast<u64>(enum_value))
    {}

    constexpr autoenum_value_t& operator =(i64) { return (*this); }
};
//----------------------------------------------------------------------------
struct autoenum_name_t {
    FStringView value;

    autoenum_name_t() = default;

    constexpr autoenum_name_t(const FStringView& v)
    :   value(v) {
        size_t len = value.size();
        forrange(i, 0, len) {
            if (not IsIdentifier(v[i])) {
                len = i;
                break;
            }
        }
        value = value.CutBefore(len);
    }
};
//----------------------------------------------------------------------------
#define _PPE_DETAILS_AUTOENUM_VALUE_DECL(X) ((::PPE::Meta::autoenum_value_t)X).value
#define _PPE_DETAILS_AUTOENUM_NAME_DECL(X) (::PPE::Meta::autoenum_name_t{CONCAT(STRINGIZE(X),_view)}).value
//----------------------------------------------------------------------------
#define PPE_DEFINE_AUTOENUM(_NAME, _TYPE, ...) \
    enum class _NAME : _TYPE { \
        __VA_ARGS__ \
    }; \
    namespace details { \
    struct CONCAT(_NAME, _info_t) { \
        using enum _NAME; \
        static constexpr std::array values = { PP_FOREACH_ARGS(_PPE_DETAILS_AUTOENUM_VALUE_DECL, __VA_ARGS__) }; \
        static constexpr std::array names = { PP_FOREACH_ARGS(_PPE_DETAILS_AUTOENUM_NAME_DECL, __VA_ARGS__) }; \
        NODISCARD static consteval ::PPE::Meta::enum_info_t get() { return { \
            .values = values, \
            .names = names, \
            .num = names.size(), \
        }; }\
    }; \
    } /*!namespace Meta*/ \
    NODISCARD inline consteval ::PPE::Meta::enum_info_t enum_info(_NAME) { \
        return details::CONCAT(_NAME, _info_t)::get(); \
    } \
    template <typename _Char> \
    NODISCARD inline bool operator >>(TBasicTextReader<_Char>& iss, _NAME* value) { \
        return enum_info(_NAME{}).read(iss, value); \
    } \
    template <typename _Char> \
    inline TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, _NAME value) { \
        return enum_info(_NAME{}).write(oss, value); \
    }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
