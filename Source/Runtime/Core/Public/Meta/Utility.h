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
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t N>
struct TStaticBitset {
    CONSTEXPR static size_t NPos = size_t(-1);

    bool Flags[N];

    CONSTEXPR TStaticBitset() NOEXCEPT
        : Flags{ 0 }
    {}

    CONSTEXPR TStaticBitset(const TStaticBitset& other) NOEXCEPT = default;
    CONSTEXPR TStaticBitset& operator =(const TStaticBitset& other) NOEXCEPT = default;

    CONSTEXPR void set(size_t i) NOEXCEPT {
        Flags[i] = true;
    }

    CONSTEXPR bool test(size_t i) const NOEXCEPT {
        return Flags[i];
    }

    CONSTEXPR size_t first_free(size_t i = 0) const NOEXCEPT {
        for (; i < N; ++i)
            if (Flags[i] == false)
                return i;

        return NPos;
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Pred>
CONSTEXPR void StaticQuicksort(T* p, int n, const _Pred& pred) NOEXCEPT {
    if (n > 0) {
        using std::swap;

        int m = 0;

        for (int i = 1; i < n; ++i) {
            if (pred(p[i], p[0]))
                swap(p[++m], p[i]);
        }

        swap(p[0], p[m]);

        StaticQuicksort(p, m, pred);
        StaticQuicksort(p + m + 1, n - m - 1, pred);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Meta
} //!namespace PPE
