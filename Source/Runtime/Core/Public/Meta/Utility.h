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
// https://youtu.be/zGWj7Qo_POY?si=xNdVOFyULLz39YOO&t=629
namespace details {
template <typename _FuncLike>
struct scope_exit_impl : _FuncLike {
    template <typename... _Args>
    CONSTEXPR scope_exit_impl(_Args&& ...args)
        : _FuncLike(std::forward<_Args>(args)...)
    {}
    CONSTEXPR ~scope_exit_impl() {
        static_cast<_FuncLike&>(*this)();
    }
};
struct make_scope_exit_impl {
    template <typename _FuncLike>
    CONSTEXPR auto operator ->*(_FuncLike&& onScopeExit) const NOEXCEPT {
        return scope_exit_impl<_FuncLike>{ std::forward<_FuncLike>(onScopeExit) };
    }
};
} //!details
template <typename _FuncLike>
CONSTEXPR auto make_scope_exit(_FuncLike&& onScopeExit) NOEXCEPT {
    return details::scope_exit_impl<_FuncLike>(std::forward<_FuncLike>(onScopeExit));
}
// ON_SCOPE_EXIT([&]{ <scope guard code> });
#define ON_SCOPE_EXIT(...) const auto ANONYMIZE(scopeExit) = ::PPE::Meta::make_scope_exit(__VA_ARGS__)
// on_scope_exit { <scope guard code> };
#define DEFERRED const auto ANONYMIZE(scopeExit) = ::PPE::Meta::details::make_scope_exit_impl{} ->* [&]() -> void
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
