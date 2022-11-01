#pragma once

#include "Meta/Aliases.h"
#include "Meta/Iterator.h"

namespace PPE {
namespace Meta {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// force wrapping a function call, a functor or a lambda in a function call
template <typename _FuncLike, typename... _Args>
NO_INLINE auto unlikely(_FuncLike funcLike, _Args... args) {
    return funcLike(std::forward<_Args>(args)...);
}
//----------------------------------------------------------------------------
// will call the trigger when leaving the scope using RAII
namespace details {
template <typename _Lambda>
struct on_scope_exit_t {
    _Lambda Trigger;
    on_scope_exit_t(_Lambda&& trigger) NOEXCEPT : Trigger(std::move(trigger)) {}
    ~on_scope_exit_t() { Trigger(); }
};
} //!details
template <typename _Lambda>
auto on_scope_exit(_Lambda&& trigger) NOEXCEPT {
    return details::on_scope_exit_t{ std::move(trigger) };
}
#define ON_SCOPE_EXIT(...) \
    const auto ANONYMIZE(scopeExit){ Meta::on_scope_exit(__VA_ARGS__) }
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t N>
struct TStaticBitset {
    CONSTEXPR static size_t NPos = UMax;
    CONSTEXPR static size_t BitsPerByte = sizeof(u8) * 8;
    CONSTEXPR static size_t NumBytes = (N + BitsPerByte - 1) / BitsPerByte;

    u8 Bytes[NumBytes]{ 0 };

    CONSTEXPR TStaticBitset() = default;

    CONSTEXPR TStaticBitset(const TStaticBitset& other) NOEXCEPT = default;
    CONSTEXPR TStaticBitset& operator =(const TStaticBitset& other) NOEXCEPT = default;

    CONSTEXPR size_t size() const NOEXCEPT {
        return N;
    }

    NODISCARD CONSTEXPR bool add(size_t i) NOEXCEPT {
        if (Likely(not test(i))) {
            set(i);
            return true;
        }
        return false;
    }

    CONSTEXPR void set(size_t i) NOEXCEPT {
        Assert(i < N);
        Bytes[i / BitsPerByte] |= (u8(1) << u8(i % BitsPerByte));
    }

    CONSTEXPR void reset(size_t i) NOEXCEPT {
        Assert(i < N);
        Bytes[i / BitsPerByte] &= ~(u8(1) << u8(i % BitsPerByte));
    }

    CONSTEXPR void set_if(size_t i, bool enabled) NOEXCEPT {
        if (enabled) set(i); else reset(i);
    }

    CONSTEXPR bool test(size_t i) const NOEXCEPT {
        Assert(i < N);
        return !!(Bytes[i / BitsPerByte] & (u8(1) << u8(i % BitsPerByte)));
    }

    CONSTEXPR size_t first_free(size_t i = 0) const NOEXCEPT {
        for (; i < N; ++i) {
            if (not test(i))
                return i;
        }
        return NPos;
    }

    CONSTEXPR void clear() NOEXCEPT {
        for (u8& b : Bytes)
            b = 0;
    }
};
//----------------------------------------------------------------------------
template <size_t N, typename T, T... _Values>
CONSTEXPR bool Uniq(std::integer_sequence<T, _Values...>) {
    TStaticBitset<N> set;
    return (set.add(_Values) && ...);
}
//----------------------------------------------------------------------------
template <size_t N, typename T, T... _Values>
CONSTEXPR bool Uniq_v = Uniq<N, T, _Values...>(
    std::integer_sequence<T, _Values...>{});
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
