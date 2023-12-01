#pragma once

#include "Core.h"

#include "Maths/ScalarVector_fwd.h"

#include "Container/Hash.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/NumericLimits.h"
#include "Meta/Utility.h"

#include <algorithm>
#include <limits>
#include <type_traits>

// V1.0: https://godbolt.org/z/dEvY3n1zq
// V2.0: https://godbolt.org/z/vExYecheW
// V2.1: https://godbolt.org/z/1vTv6z9sG

PRAGMA_DISABLE_RUNTIMECHECKS

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Begin TScalarVector<> implementation with support for:
// - swizzling
// - type promotion
// - deferred vector construction
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// TScalarVectorExprBase<T, N, Expr>
//----------------------------------------------------------------------------
namespace details {
template <typename T, u32 _Dim, typename _Expr>
struct TScalarVectorExprBase : _Expr {
    using _Expr::_Expr;

    using component_type = T;
    using reference_type = decltype(std::declval<_Expr&>().Get(0));
    using const_reference_type = decltype(std::declval<const _Expr&>().Get(0));

    static CONSTEXPR const u32 dim = _Dim;

    CONSTEXPR TScalarVectorExprBase() = default;

    template <u32 _Index>
    NODISCARD CONSTEXPR reference_type Get() {
        static_assert(_Index < _Dim, "out-of-bounds");
        return _Expr::Get(_Index);
    }
    template <u32 _Index>
    NODISCARD CONSTEXPR const_reference_type Get() const {
        static_assert(_Index < _Dim, "out-of-bounds");
        return _Expr::Get(_Index);
    }

    NODISCARD CONSTEXPR reference_type Get(u32 index) {
        Assert(index < _Dim && "out-of-bounds");
        return _Expr::Get(index);
    }
    NODISCARD CONSTEXPR const_reference_type Get(u32 index) const {
        Assert(index < _Dim && "out-of-bounds");
        return _Expr::Get(index);
    }

    NODISCARD CONSTEXPR reference_type operator [](u32 index) { return Get(index); }
    NODISCARD CONSTEXPR const_reference_type operator [](u32 index) const { return Get(index); }

    NODISCARD CONSTEXPR T HSum() const {
        return Meta::static_for<_Dim>([this](auto... idx) {
            return (this->Get<idx>() + ...);
        });
    }

    NODISCARD CONSTEXPR T MaxComponent() const {
        return Meta::static_for<_Dim - 1>([this](auto... idx) {
            T result = Get<0>();
            FOLD_EXPR(result = Max(result, this->Get<idx + 1>()));
            return result;
        });
    }
    NODISCARD CONSTEXPR T MinComponent() const {
        return Meta::static_for<_Dim - 1>([this](auto... idx) {
            T result = Get<0>();
            FOLD_EXPR(result = Min(result, this->Get<idx + 1>()));
            return result;
        });
    }

    NODISCARD CONSTEXPR u32 MaxComponentIndex() const {
        return Meta::static_for<_Dim - 1>([this](auto... idx) {
            u32 result = 0;
            FOLD_EXPR(result = this->Get(result) < this->Get<idx + 1>() ? static_cast<u32>(idx + 1) : result);
            return result;
        });
    }
    NODISCARD CONSTEXPR u32 MinComponentIndex() const {
        return Meta::static_for<_Dim - 1>([this](auto... idx) {
            u32 result = 0;
            FOLD_EXPR(result = this->Get(result) > this->Get<idx + 1>() ? static_cast<u32>(idx + 1) : result);
            return result;
        });
    }

    NODISCARD CONSTEXPR auto Extend(const T& value) const {
        return Meta::static_for<_Dim>([&](auto... idx) {
            return TScalarVector<T, sizeof...(idx) + 1>{ this->Get<idx>()..., value };
        });
    }

    template <u32 _Dim2, typename _Other>
    NODISCARD CONSTEXPR auto Extend(const TScalarVectorExpr<T, _Dim2, _Other>& other) const {
        return Meta::static_for<_Dim>([&](auto... idx) {
            return Meta::static_for<_Dim2>([&](auto... jdx) {
                return TScalarVector<T, sizeof...(idx) + sizeof...(jdx)>{ this->Get<idx>()..., other.template Get<jdx>()... };
            });
        });
    }

    template <u32 _Count = 1>
    NODISCARD CONSTEXPR auto Shift() const {
        return Meta::static_for<(_Dim > _Count ? _Dim - _Count : 1)>([this](auto... idx) {
            return TScalarVector<T, sizeof...(idx)>(this->Get<idx>()...);
        });
    }

    template <u32... _Idx>
    NODISCARD CONSTEXPR auto Shuffle() const {
        return TScalarVector<T, sizeof...(_Idx)>(this->Get<_Idx>()...);
    }
};
} //!details
//----------------------------------------------------------------------------
// TScalarVectorExpr<T, N, Expr>
//----------------------------------------------------------------------------
namespace details {
template <typename T, u32 _Dim, typename _Expr>
struct TScalarVectorExpr : TScalarVectorExprBase<T, _Dim, _Expr> {
    using TScalarVectorExprBase<T, _Dim, _Expr>::TScalarVectorExprBase;
    using TScalarVectorExprBase<T, _Dim, _Expr>::Get;
    using TScalarVectorExprBase<T, _Dim, _Expr>::operator [];

    CONSTEXPR TScalarVectorExpr() = default;
    CONSTEXPR ~TScalarVectorExpr() = default;

    TScalarVectorExpr(const TScalarVectorExpr&) = delete;
    TScalarVectorExpr& operator =(const TScalarVectorExpr&) = delete;

    CONSTEXPR TScalarVectorExpr(TScalarVectorExpr&&) = default;
    TScalarVectorExpr& operator =(TScalarVectorExpr&&) = delete;
};
template <typename T, u32 _Dim, typename _Expr>
struct TScalarVectorExpr<T, _Dim, TScalarVectorAssignable<T, _Dim, _Expr>> : TScalarVectorExprBase<T, _Dim, TScalarVectorAssignable<T, _Dim, _Expr>> {
    using TScalarVectorExprBase<T, _Dim, TScalarVectorAssignable<T, _Dim, _Expr>>::TScalarVectorExprBase;
    using TScalarVectorExprBase<T, _Dim, TScalarVectorAssignable<T, _Dim, _Expr>>::Get;
    using TScalarVectorExprBase<T, _Dim, TScalarVectorAssignable<T, _Dim, _Expr>>::operator [];
    using TScalarVectorAssignable<T, _Dim, _Expr>::Assign;

    CONSTEXPR TScalarVectorExpr() = default;
    CONSTEXPR ~TScalarVectorExpr() = default;

    CONSTEXPR TScalarVectorExpr(const TScalarVectorExpr& other) {
        Assign(other);
    }
    CONSTEXPR TScalarVectorExpr& operator =(const TScalarVectorExpr& other) {
        Assign(other);
        return (*this);
    }

    CONSTEXPR TScalarVectorExpr(TScalarVectorExpr&& rvalue) NOEXCEPT {
        Assign(std::move(rvalue));
    }
    CONSTEXPR TScalarVectorExpr& operator =(TScalarVectorExpr&& rvalue) NOEXCEPT {
        Assign(std::move(rvalue));
        return (*this);
    }

    template <typename _Other>
    CONSTEXPR TScalarVectorExpr(const TScalarVectorExpr<T, _Dim, _Other>& other) NOEXCEPT {
        Assign(other);
    }

    template <typename _Other>
    CONSTEXPR TScalarVectorExpr& operator =(const TScalarVectorExpr<T, _Dim, _Other>& other) NOEXCEPT {
        Assign(other);
        return (*this);
    }
};
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator ==(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return ((lhs.template Get<idx>() == rhs.template Get<idx>()) && ...);
    });
}
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator !=(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) NOEXCEPT {
    return not operator ==(lhs, rhs);
}
} //!namespace details
//----------------------------------------------------------------------------
// TScalarVectorAxis<T, Axis>
//----------------------------------------------------------------------------
namespace details {
template <typename T, u32 _Axis>
struct TScalarVectorAxis {
    CONSTEXPR T Get(u32 index) const {
        return static_cast<T>(index == _Axis ? 1 : 0);
    }
};
} //!namespace details
//----------------------------------------------------------------------------
// TScalarVectorLiteral<T>
//----------------------------------------------------------------------------
namespace details {
template <typename T>
struct TScalarVectorLiteral {
    const T Literal;

    CONSTEXPR explicit TScalarVectorLiteral(T&& literal) : Literal(std::move(literal)) {}

    CONSTEXPR const T& Get(u32) const {
        return Literal;
    }
};
template <u32 _Dim, typename T>
inline CONSTEXPR auto MakeScalarVectorLiteral(T&& literal) {
    using literal_type = TScalarVectorLiteral<std::decay_t<T>>;
    return TScalarVectorExpr<std::decay_t<T>, _Dim, literal_type>{ std::forward<T>(literal) };
}
// use comma operator to extend scalar vectors, should be alright?
// ex:  int4 xy01 = (v.xy, 0, 1);
template <typename T, u32 _Dim, typename _Expr>
inline CONSTEXPR auto operator ,(T lhs, const TScalarVectorExpr<T, _Dim, _Expr>& rhs) {
    return details::MakeScalarVectorLiteral<1>(std::move(lhs)).Extend(rhs);
}
template <typename T, u32 _Dim, typename _Expr>
inline CONSTEXPR auto operator ,(const TScalarVectorExpr<T, _Dim, _Expr>& lhs, T rhs) {
    return lhs.Extend(rhs);
}
template <typename T, u32 _Dim, typename _Lhs, u32 _Dim2, typename _Rhs>
inline CONSTEXPR auto operator ,(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim2, _Rhs>& rhs) {
    return lhs.Extend(rhs);
}
} //!namespace details
//----------------------------------------------------------------------------
// TScalarVectorRef<T, N, _Ref>
//----------------------------------------------------------------------------
namespace details {
template <typename T, u32 _Dim, typename _Ref, bool _Const>
struct TScalarVectorRef {
    using expression_type = Meta::TAddConstIFN<
        TScalarVectorExpr<T, _Dim, _Ref>,
        _Const >;

    expression_type* Ref{};

    CONSTEXPR explicit TScalarVectorRef(expression_type* ref) : Ref(ref) {}

    CONSTEXPR auto Get(u32 index) const {
        return Ref->Get(index);
    }
};
template <typename T, u32 _Dim, typename _Ref>
NODISCARD CONSTEXPR auto MakeScalarVectorRef(TScalarVectorExpr<T, _Dim, _Ref>& ref) {
    return TScalarVectorExpr<T, _Dim, TScalarVectorRef<T, _Dim, _Ref, false>>{ std::addressof(ref) };
}
template <typename T, u32 _Dim, typename _Ref>
NODISCARD CONSTEXPR auto MakeScalarVectorRef(const TScalarVectorExpr<T, _Dim, _Ref>& ref) {
    return TScalarVectorExpr<T, _Dim, TScalarVectorRef<T, _Dim, _Ref, true>>{ std::addressof(ref) };
}
} //!namespace details
//----------------------------------------------------------------------------
// TScalarVectorAssignable<T, N>
//----------------------------------------------------------------------------
namespace details {
template <typename T, u32 _Dim, typename _Expr>
struct TScalarVectorAssignable : _Expr {
    using _Expr::_Expr;
    using _Expr::Get;
    using _Expr::data;

    static_assert(sizeof(T)* _Dim <= sizeof(_Expr::data), "invalid assignable vector dimension");

    CONSTEXPR TScalarVectorAssignable(Meta::FForceInit) NOEXCEPT {
        Broadcast(Meta::MakeForceInit<T>());
    }

    template <typename _Cast, typename _Other>
    CONSTEXPR explicit TScalarVectorAssignable(const TScalarVectorExpr<_Cast, _Dim, _Other>& other) NOEXCEPT {
        Meta::static_for<_Dim>([&](auto... idx) {
            FOLD_EXPR(Get(idx) = static_cast<T>(other.template Get<idx>()));
        });
    }

    template <typename _Other>
    CONSTEXPR void Assign(const TScalarVectorExpr<T, _Dim, _Other>& other) NOEXCEPT {
        Meta::static_for<_Dim>([&](auto... idx) {
            FOLD_EXPR(Get(idx) = other.template Get<idx>());
        });
    }

    template <typename _Other>
    CONSTEXPR void Assign(TScalarVectorExpr<T, _Dim, _Other>&& rvalue) NOEXCEPT {
        Meta::static_for<_Dim>([&](auto... idx) {
            FOLD_EXPR(Get(idx) = std::move(rvalue.template Get<idx>()));
        });
    }

    CONSTEXPR void Broadcast(const T& value) NOEXCEPT {
        Meta::static_for<_Dim>([&](auto... idx) {
            FOLD_EXPR(Get(idx) = value);
        });
    }

    friend CONSTEXPR void swap(
        TScalarVectorExpr<T, _Dim, TScalarVectorAssignable>& lhs,
        TScalarVectorExpr<T, _Dim, TScalarVectorAssignable>& rhs) NOEXCEPT {
        Meta::static_for<_Dim>([&](auto... idx) NOEXCEPT{
            FOLD_EXPR(std::swap(lhs.template Get<idx>(), rhs.template Get<idx>()));
        });
    }

    static CONSTEXPR const TScalarVectorExpr<T, _Dim, TScalarVectorLiteral<T>> One{ static_cast<T>(1) };
    static CONSTEXPR const TScalarVectorExpr<T, _Dim, TScalarVectorLiteral<T>> Zero{ static_cast<T>(0) };
    static CONSTEXPR const TScalarVectorExpr<T, _Dim, TScalarVectorLiteral<T>> MinusOne{ static_cast<T>(-1) };

    static CONSTEXPR const TScalarVectorExpr<T, _Dim, TScalarVectorLiteral<T>> MaxValue{ TNumericLimits<T>::MaxValue() };
    static CONSTEXPR const TScalarVectorExpr<T, _Dim, TScalarVectorLiteral<T>> MinValue{ TNumericLimits<T>::MinValue() };
    static CONSTEXPR const TScalarVectorExpr<T, _Dim, TScalarVectorLiteral<T>> Lowest{ TNumericLimits<T>::Lowest() };

};
} //!namespace details
//----------------------------------------------------------------------------
// TScalarVectorShuffle<T, N, _Shuffle...>
//----------------------------------------------------------------------------
namespace details {
template <typename T, u32 _Dim, u32... _Shuffle>
struct TScalarVectorShuffle {
    static CONSTEXPR const u32 component_indices[sizeof...(_Shuffle)] = {_Shuffle...};
    STATIC_ASSERT(((_Shuffle < _Dim) && ...));

    template <u32>
    using shuffle_type = T;

    T data[_Dim];

    CONSTEXPR TScalarVectorShuffle() = default;
    CONSTEXPR TScalarVectorShuffle(shuffle_type<_Shuffle>... args) {
        Meta::static_for<sizeof...(_Shuffle)>([&](auto... idx) {
            FOLD_EXPR(Get(idx) = args);
        });
    }

    CONSTEXPR T& Get(u32 index) {
        return data[component_indices[index]];
    }
    CONSTEXPR const T& Get(u32 index) const {
        return data[component_indices[index]];
    }
};
template <typename T, u32 _Dim, u32... _Shuffle>
using TMakeScalarVectorShuffleExpr = TScalarVectorExpr<T,
    sizeof...(_Shuffle),
    std::conditional_t<
        Meta::Uniq_v<_Dim, u32, _Shuffle...>,
        // assignable
        TScalarVectorAssignable<T, sizeof...(_Shuffle),
            TScalarVectorShuffle<T, _Dim, _Shuffle...>>,
        // non-assignable
        TScalarVectorShuffle<T, _Dim, _Shuffle...>
    >>;
} //!namespace details
//----------------------------------------------------------------------------
// TScalarVectorStorage<T, N>
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
struct TScalarVectorStorage {
    T data[_Dim];

    CONSTEXPR TScalarVectorStorage() = default;
    CONSTEXPR TScalarVectorStorage(Meta::FNoInit) NOEXCEPT {}

    CONSTEXPR explicit TScalarVectorStorage(T broadcast) NOEXCEPT {
        Meta::static_for<_Dim>([&](auto... idx) {
            ((data[idx] = broadcast), ...);
        });
    }

    // forced to use a variadic template with SFINAE to handle N-ary constructor
    template <typename... _Args, decltype(decltype(data){std::forward<_Args>(std::declval<_Args>())...})* = nullptr>
    CONSTEXPR explicit TScalarVectorStorage(_Args&&... args) NOEXCEPT
    :   data{ std::forward<_Args>(args)... }
    {}

    CONSTEXPR T& Get(u32 index) {
        return data[index];
    }
    CONSTEXPR const T& Get(u32 index) const {
        return data[index];
    }
};
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVectorStorage<T, 1> {
    union {
        T data[1];

        struct {
            T x;
        };
    };

    CONSTEXPR TScalarVectorStorage() NOEXCEPT : data{} {}
    CONSTEXPR TScalarVectorStorage(Meta::FNoInit) NOEXCEPT {}
    CONSTEXPR explicit TScalarVectorStorage(T _x) NOEXCEPT : data{ _x } {}

    CONSTEXPR T& Get(u32 index) {
        return data[index];
    }
    CONSTEXPR const T& Get(u32 index) const {
        return data[index];
    }

    CONSTEXPR void Set(T _x) {
        x = _x;
    }

    static CONSTEXPR const TScalarVectorExpr<T, 1, TScalarVectorAxis<T, 0>> X{};
};
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVectorStorage<T, 2> {
    union {
        T data[2];

        struct {
            T x, y;
        };

        TMakeScalarVectorShuffleExpr<T, 2, 0, 1> xy;
        TMakeScalarVectorShuffleExpr<T, 2, 1, 0> yx;

        TMakeScalarVectorShuffleExpr<T, 2, 0, 0> xx;
        TMakeScalarVectorShuffleExpr<T, 2, 1, 1> yy;

        TMakeScalarVectorShuffleExpr<T, 2, 0, 0, 0> xxx;
        TMakeScalarVectorShuffleExpr<T, 2, 1, 1, 1> yyy;

        TMakeScalarVectorShuffleExpr<T, 2, 0, 0, 0, 0> xxxx;
        TMakeScalarVectorShuffleExpr<T, 2, 1, 1, 1, 1> yyyy;
    };

    CONSTEXPR TScalarVectorStorage() NOEXCEPT : data{} {}
    CONSTEXPR TScalarVectorStorage(Meta::FNoInit) NOEXCEPT {}
    CONSTEXPR TScalarVectorStorage(T _x, T _y) NOEXCEPT : data{ _x, _y } {}

    CONSTEXPR explicit TScalarVectorStorage(T broadcast) NOEXCEPT : TScalarVectorStorage(broadcast, broadcast) {}

    CONSTEXPR T& Get(u32 index) {
        return data[index];
    }
    CONSTEXPR const T& Get(u32 index) const {
        return data[index];
    }

    CONSTEXPR void Set(T _x, T _y) {
        x = _x; y = _y;
    }

    static CONSTEXPR const TScalarVectorExpr<T, 2, TScalarVectorAxis<T, 0>> X{};
    static CONSTEXPR const TScalarVectorExpr<T, 2, TScalarVectorAxis<T, 1>> Y{};
};
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVectorStorage<T, 3> {
    union {
        T data[3];

        struct {
            T x, y, z;
        };

        TMakeScalarVectorShuffleExpr<T, 3, 0, 1> xy;
        TMakeScalarVectorShuffleExpr<T, 3, 0, 2> xz;
        TMakeScalarVectorShuffleExpr<T, 3, 1, 0> yx;
        TMakeScalarVectorShuffleExpr<T, 3, 1, 2> yz;
        TMakeScalarVectorShuffleExpr<T, 3, 2, 0> zx;
        TMakeScalarVectorShuffleExpr<T, 3, 2, 1> zy;

        TMakeScalarVectorShuffleExpr<T, 3, 0, 1, 2> xyz;
        TMakeScalarVectorShuffleExpr<T, 3, 0, 2, 1> xzy;
        TMakeScalarVectorShuffleExpr<T, 3, 1, 0, 2> yxz;
        TMakeScalarVectorShuffleExpr<T, 3, 1, 2, 0> yzx;
        TMakeScalarVectorShuffleExpr<T, 3, 2, 0, 1> zxy;
        TMakeScalarVectorShuffleExpr<T, 3, 2, 1, 0> zyx;

        TMakeScalarVectorShuffleExpr<T, 3, 0, 0> xx;
        TMakeScalarVectorShuffleExpr<T, 3, 1, 1> yy;
        TMakeScalarVectorShuffleExpr<T, 3, 2, 2> zz;

        TMakeScalarVectorShuffleExpr<T, 3, 0, 0, 0> xxx;
        TMakeScalarVectorShuffleExpr<T, 3, 1, 1, 1> yyy;
        TMakeScalarVectorShuffleExpr<T, 3, 2, 2, 2> zzz;

        TMakeScalarVectorShuffleExpr<T, 3, 0, 0, 0, 0> xxxx;
        TMakeScalarVectorShuffleExpr<T, 3, 1, 1, 1, 1> yyyy;
        TMakeScalarVectorShuffleExpr<T, 3, 2, 2, 2, 2> zzzz;

    };

    CONSTEXPR TScalarVectorStorage() NOEXCEPT : data{} {}
    CONSTEXPR TScalarVectorStorage(Meta::FNoInit) NOEXCEPT {}
    CONSTEXPR TScalarVectorStorage(T _x, T _y, T _z) NOEXCEPT : data{ _x, _y, _z } {}

    CONSTEXPR explicit TScalarVectorStorage(T broadcast) NOEXCEPT : TScalarVectorStorage(broadcast, broadcast, broadcast) {}

    template <typename _Expr>
    CONSTEXPR TScalarVectorStorage(T _x, const TScalarVectorExpr<T, 2, _Expr>& yz) NOEXCEPT : TScalarVectorStorage(_x, yz.template Get<0>(), yz.template Get<1>()) {}
    template <typename _Expr>
    CONSTEXPR TScalarVectorStorage(const TScalarVectorExpr<T, 2, _Expr>& xy, T _z) NOEXCEPT : TScalarVectorStorage(xy.template Get<0>(), xy.template Get<1>(), _z) {}

    CONSTEXPR T& Get(u32 index) {
        return data[index];
    }
    CONSTEXPR const T& Get(u32 index) const {
        return data[index];
    }

    CONSTEXPR void Set(T _x, T _y, T _z) {
        x = _x; y = _y; z = _z;
    }

    static CONSTEXPR const TScalarVectorExpr<T, 3, TScalarVectorAxis<T, 0>> X{};
    static CONSTEXPR const TScalarVectorExpr<T, 3, TScalarVectorAxis<T, 1>> Y{};
    static CONSTEXPR const TScalarVectorExpr<T, 3, TScalarVectorAxis<T, 2>> Z{};
};
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVectorStorage<T, 4> {
    union {
        T data[4];

        struct {
            T x, y, z, w;
        };

        TMakeScalarVectorShuffleExpr<T, 4, 0, 0> xx;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 1> yy;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 2> zz;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 3> ww;

        TMakeScalarVectorShuffleExpr<T, 4, 0, 0, 0> xxx;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 1, 1> yyy;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 2, 2> zzz;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 2, 2> www;

        TMakeScalarVectorShuffleExpr<T, 4, 0, 0, 0, 0> xxxx;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 1, 1, 1> yyyy;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 2, 2, 2> zzzz;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 3, 3, 3> wwww;

        TMakeScalarVectorShuffleExpr<T, 4, 0, 1> xy;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 2> xz;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 3> xw;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 0> yx;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 2> yz;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 3> yw;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 0> zx;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 1> zy;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 3> zw;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 0> wx;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 1> wy;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 2> wz;

        TMakeScalarVectorShuffleExpr<T, 4, 0, 1, 2> xyz;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 2, 1> xzy;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 1, 3> xyw;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 3, 1> xwy;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 2, 3> xzw;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 3, 2> xwz;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 0, 2> yxz;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 2, 0> yzx;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 0, 3> yxw;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 3, 0> ywx;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 2, 3> yzw;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 3, 2> ywz;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 0, 1> zxy;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 1, 0> zyx;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 0, 3> zxw;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 3, 0> zwx;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 1, 3> zyw;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 3, 1> zwy;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 0, 1> wxy;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 1, 0> wyx;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 0, 2> wxz;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 2, 0> wzx;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 1, 2> wyz;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 2, 1> wzy;

        TMakeScalarVectorShuffleExpr<T, 4, 0, 1, 2, 3> xyzw;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 1, 3, 2> xywz;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 2, 1, 3> xzyw;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 2, 3, 1> xzwy;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 3, 1, 2> xwyz;
        TMakeScalarVectorShuffleExpr<T, 4, 0, 3, 2, 1> xwzy;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 0, 2, 3> yxzw;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 0, 3, 2> yxwz;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 2, 0, 3> yzxw;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 2, 3, 0> yzwx;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 3, 0, 2> ywxz;
        TMakeScalarVectorShuffleExpr<T, 4, 1, 3, 2, 0> ywzx;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 0, 1, 3> zxyw;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 0, 3, 1> zxwy;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 1, 0, 3> zyxw;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 1, 3, 0> zywx;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 3, 0, 1> zwxy;
        TMakeScalarVectorShuffleExpr<T, 4, 2, 3, 1, 0> zwyx;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 0, 1, 2> wxyz;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 0, 2, 1> wxzy;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 1, 0, 2> wyxz;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 1, 2, 0> wyzx;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 2, 0, 1> wzxy;
        TMakeScalarVectorShuffleExpr<T, 4, 3, 2, 1, 0> wzyx;
    };

    CONSTEXPR TScalarVectorStorage() NOEXCEPT : data{} {}
    CONSTEXPR TScalarVectorStorage(Meta::FNoInit) NOEXCEPT {}
    CONSTEXPR TScalarVectorStorage(T _x, T _y, T _z, T _w) NOEXCEPT : data{ _x, _y, _z, _w } {}

    CONSTEXPR explicit TScalarVectorStorage(T broadcast) NOEXCEPT : TScalarVectorStorage(broadcast, broadcast, broadcast, broadcast) {}

    template <typename _Expr>
    CONSTEXPR TScalarVectorStorage(T _x, T _y, const TScalarVectorExpr<T, 2, _Expr>& zw) NOEXCEPT : TScalarVectorStorage(_x, _y, zw.template Get<0>(), zw.template Get<1>()) {}
    template <typename _Expr>
    CONSTEXPR TScalarVectorStorage(const TScalarVectorExpr<T, 2, _Expr>& xy, T _z, T _w) NOEXCEPT : TScalarVectorStorage(xy.template Get<0>(), xy.template Get<1>(), _z, _w) {}
    template <typename _Expr>
    CONSTEXPR TScalarVectorStorage(const TScalarVectorExpr<T, 2, _Expr>& xy, const TScalarVectorExpr<T, 2, _Expr>& zw) NOEXCEPT : TScalarVectorStorage(xy.template Get<0>(), xy.template Get<1>(), zw.template Get<0>(), zw.template Get<1>()) {}

    template <typename _Expr>
    CONSTEXPR TScalarVectorStorage(T _x, const TScalarVectorExpr<T, 3, _Expr>& yzw) NOEXCEPT : TScalarVectorStorage(_x, yzw.template Get<0>(), yzw.template Get<1>(), yzw.template Get<2>()) {}
    template <typename _Expr>
    CONSTEXPR TScalarVectorStorage(const TScalarVectorExpr<T, 3, _Expr>& xyz, T _w) NOEXCEPT : TScalarVectorStorage(xyz.template Get<0>(), xyz.template Get<1>(), xyz.template Get<2>(), _w) {}

    CONSTEXPR T& Get(u32 index) {
        return data[index];
    }
    CONSTEXPR const T& Get(u32 index) const {
        return data[index];
    }

    CONSTEXPR void Set(T _x, T _y, T _z, T _w) {
        x = _x; y = _y; z = _z; w = _w;
    }

    static CONSTEXPR const TScalarVectorExpr<T, 4, TScalarVectorAxis<T, 0>> X{};
    static CONSTEXPR const TScalarVectorExpr<T, 4, TScalarVectorAxis<T, 1>> Y{};
    static CONSTEXPR const TScalarVectorExpr<T, 4, TScalarVectorAxis<T, 2>> Z{};
    static CONSTEXPR const TScalarVectorExpr<T, 4, TScalarVectorAxis<T, 3>> W{};
};
}; //!namespace details
//----------------------------------------------------------------------------
// unary operator -, ~, !
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Prm, typename _Op>
struct TScalarVectorUnaryOp : private _Op {
    const TScalarVectorExpr<T, _Dim, _Prm>& Prm;

    CONSTEXPR explicit TScalarVectorUnaryOp(
        const TScalarVectorExpr<T, _Dim, _Prm>& prm,
        _Op&& unaryOp)
        : _Op(std::move(unaryOp))
        , Prm(prm)
    {}

    CONSTEXPR auto Get(u32 index) const {
        return _Op::operator ()(Prm.Get(index));
    }
};
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Prm, typename _Op>
NODISCARD CONSTEXPR auto MakeScalarVectorOp(
    const TScalarVectorExpr<T, _Dim, _Prm>& prm,
    _Op&& unaryOp) {
    using op_type = TScalarVectorUnaryOp<T, _Dim, _Prm, _Op>;
    using destination_type = decltype(unaryOp(std::declval<T>()));
    return TScalarVectorExpr<destination_type, _Dim, op_type>{ prm,std::forward<_Op>(unaryOp) };
}
//----------------------------------------------------------------------------k
} //!namespace details
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_UNARYOP_DECL(_FunctionName, ...) \
    template <typename T, u32 _Dim, typename _Prm> \
    inline CONSTEXPR auto _FunctionName(const details::TScalarVectorExpr<T, _Dim, _Prm>& prm) { \
        return details::MakeScalarVectorOp(prm, __VA_ARGS__); \
    }
#define PPE_SCALARVECTOR_UNARYOP_FUNC(_FunctionName) \
    PPE_SCALARVECTOR_UNARYOP_DECL(_FunctionName, [](const T& x) CONSTEXPR { return PPE::_FunctionName(x); })
//----------------------------------------------------------------------------
namespace details {
PPE_SCALARVECTOR_UNARYOP_DECL(operator !, std::bit_not<T>{})
PPE_SCALARVECTOR_UNARYOP_DECL(operator -, std::negate<T>{})
PPE_SCALARVECTOR_UNARYOP_DECL(operator ~, Meta::TUnaryComplement<T>{})
} //!namespace details
//----------------------------------------------------------------------------
//#undef PPE_SCALARVECTOR_UNARYOP_DECL // let it spill for ScalarVectorHelpers.h
//#undef PPE_SCALARVECTOR_UNARYOP_FUNC
//----------------------------------------------------------------------------
// binary operator +, -, *, /, %, &, |, ^
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs, typename _Op>
struct TScalarVectorBinaryOp : private _Op {
    TScalarVectorExpr<T, _Dim, _Lhs> Lhs;
    TScalarVectorExpr<T, _Dim, _Rhs> Rhs;

    CONSTEXPR explicit TScalarVectorBinaryOp(
        TScalarVectorExpr<T, _Dim, _Lhs>&& __restrict lhs,
        TScalarVectorExpr<T, _Dim, _Rhs>&& __restrict rhs,
        _Op&& binaryOp)
        : _Op(std::move(binaryOp))
        , Lhs(std::move(lhs)), Rhs(std::move(rhs))
    {}

    CONSTEXPR auto Get(u32 index) const {
        return _Op::operator ()(Lhs.Get(index), Rhs.Get(index));
    }
};
//----------------------------------------------------------------------------
template <typename T, u32 _Dim, typename _Lhs, typename _Rhs, typename _Op>
NODISCARD CONSTEXPR auto MakeScalarVectorOp(
    TScalarVectorExpr<T, _Dim, _Lhs>&& __restrict lhs,
    TScalarVectorExpr<T, _Dim, _Rhs>&& __restrict rhs,
    _Op&& binaryOp) {
    using op_type = TScalarVectorBinaryOp<T, _Dim, _Lhs, _Rhs, _Op>;
    using destination_type = decltype(binaryOp(std::declval<T>(), std::declval<T>()));
    return TScalarVectorExpr<destination_type, _Dim, op_type>{ std::move(lhs), std::move(rhs),std::forward<_Op>(binaryOp) };
}
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_BINARYOP_DECL(_FunctionName, ...) \
    template <typename T, u32 _Dim, typename _Lhs, typename _Rhs> \
    inline CONSTEXPR auto _FunctionName(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) { \
        return details::MakeScalarVectorOp(MakeScalarVectorRef(lhs), MakeScalarVectorRef(rhs), __VA_ARGS__); \
    } \
    template <typename T, u32 _Dim, typename _Lhs> \
    inline CONSTEXPR auto _FunctionName(const details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, T rhs) { \
        return details::MakeScalarVectorOp(MakeScalarVectorRef(lhs), details::MakeScalarVectorLiteral<_Dim>(std::move(rhs)), __VA_ARGS__); \
    } \
    template <typename T, u32 _Dim, typename _Rhs> \
    inline CONSTEXPR auto _FunctionName(T lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) { \
        return details::MakeScalarVectorOp(details::MakeScalarVectorLiteral<_Dim>(std::move(lhs)), MakeScalarVectorRef(rhs), __VA_ARGS__); \
    }
#define PPE_SCALARVECTOR_BINARYOP_OPERATOR(_FunctionName, ...) \
    PPE_SCALARVECTOR_BINARYOP_DECL(_FunctionName, __VA_ARGS__) \
    template <typename T, u32 _Dim, typename _Lhs, typename _Rhs> \
    inline CONSTEXPR auto& CONCAT(_FunctionName, =)(details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const details::TScalarVectorExpr<T, _Dim, _Rhs>& rhs) { \
        lhs = details::MakeScalarVectorOp(MakeScalarVectorRef(lhs), MakeScalarVectorRef(rhs), __VA_ARGS__); \
        return lhs; \
    } \
    template <typename T, u32 _Dim, typename _Lhs> \
    inline CONSTEXPR auto& CONCAT(_FunctionName, =)(details::TScalarVectorExpr<T, _Dim, _Lhs>& lhs, T rhs) { \
        lhs = details::MakeScalarVectorOp(MakeScalarVectorRef(lhs), details::MakeScalarVectorLiteral<_Dim>(std::move(rhs)), __VA_ARGS__); \
        return lhs; \
    }
#define PPE_SCALARVECTOR_BINARYOP_FUNC(_FunctionName) \
    PPE_SCALARVECTOR_BINARYOP_DECL(_FunctionName, [](const T& a, const T& b) CONSTEXPR { return PPE::_FunctionName(a, b); })
//----------------------------------------------------------------------------
namespace details {
PPE_SCALARVECTOR_BINARYOP_OPERATOR(operator +, std::plus<T>{})
PPE_SCALARVECTOR_BINARYOP_OPERATOR(operator -, std::minus<T>{})
PPE_SCALARVECTOR_BINARYOP_OPERATOR(operator *, std::multiplies<T>{})
PPE_SCALARVECTOR_BINARYOP_OPERATOR(operator /, std::divides<T>{})
PPE_SCALARVECTOR_BINARYOP_OPERATOR(operator &, std::bit_and<T>{})
PPE_SCALARVECTOR_BINARYOP_OPERATOR(operator |, std::bit_or<T>{})
PPE_SCALARVECTOR_BINARYOP_OPERATOR(operator ^, std::bit_xor<T>{})
} //!namespace details
//----------------------------------------------------------------------------
PPE_SCALARVECTOR_BINARYOP_DECL(EqualMask, std::equal_to<T>{})
PPE_SCALARVECTOR_BINARYOP_DECL(NotEqualMask, std::not_equal_to<T>{})
PPE_SCALARVECTOR_BINARYOP_DECL(LessMask, std::less<T>{})
PPE_SCALARVECTOR_BINARYOP_DECL(LessEqualMask, std::less_equal<T>{})
PPE_SCALARVECTOR_BINARYOP_DECL(GreaterMask, std::greater<T>{})
PPE_SCALARVECTOR_BINARYOP_DECL(GreaterEqualMask, std::greater_equal<T>{})
//----------------------------------------------------------------------------
//#undef PPE_SCALARVECTOR_BINARYOP_DECL  // let it spill for ScalarVectorHelpers.h
//#undef PPE_SCALARVECTOR_BINARYOP_OPERATOR
//#undef PPE_SCALARVECTOR_BINARYOP_FUNC
//----------------------------------------------------------------------------
// helpers
//----------------------------------------------------------------------------
namespace details {
template <typename _Char, typename T, u32 _Dim, typename _Expr>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TScalarVectorExpr<T, _Dim, _Expr>& v) {
    oss << STRING_LITERAL(_Char, '[') << v.data[0];
    forrange(i, 1, _Dim)
        oss << STRING_LITERAL(_Char, ", ") << v.data[i];
    oss << STRING_LITERAL(_Char, ']');
    return oss;
}
template <typename T, u32 _Dim, typename _Expr>
hash_t hash_value(const TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&v](auto... idx) {
        return hash_tuple(v.template Get<idx>()...);
    });
}
//----------------------------------------------------------------------------
// All scalar vectors are considered as pods
PPE_ASSUME_TEMPLATE_AS_POD(TScalarVector<T COMMA _Dim>, typename T, u32 _Dim)
//----------------------------------------------------------------------------
} //!namespace details
//----------------------------------------------------------------------------
template <typename _Dst, typename T, u32 _Dim, typename _Expr>
CONSTEXPR auto bit_cast(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) {
    return details::MakeScalarVectorOp(v, [](const T& x) CONSTEXPR { return PPE::bit_cast<_Dst>(x); });
}
//----------------------------------------------------------------------------
template <typename _Dst, typename T, u32 _Dim, typename _Expr>
CONSTEXPR auto checked_cast(const details::TScalarVectorExpr<T, _Dim, _Expr>& v) {
    return details::MakeScalarVectorOp(v, [](const T& x) CONSTEXPR { return PPE::checked_cast<_Dst>(x); });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
struct TNumericLimits< TScalarVector<T, _Dim> > {
    typedef TScalarVector<T, _Dim> value_type;
    typedef TNumericLimits<T> scalar_type;

    STATIC_CONST_INTEGRAL(bool, is_integer, scalar_type::is_integer);
    STATIC_CONST_INTEGRAL(bool, is_modulo,  scalar_type::is_modulo);
    STATIC_CONST_INTEGRAL(bool, is_signed,  scalar_type::is_signed);

    static CONSTEXPR value_type DefaultValue() NOEXCEPT { return value_type( scalar_type::DefaultValue() ); }
    static CONSTEXPR value_type Epsilon() NOEXCEPT { return value_type( scalar_type::Epsilon() ); }
    static CONSTEXPR value_type Inf() NOEXCEPT { return value_type( scalar_type::Inf() ); }
    static CONSTEXPR value_type MaxValue() NOEXCEPT { return value_type( scalar_type::MaxValue() ); }
    static CONSTEXPR value_type MinValue() NOEXCEPT { return value_type( scalar_type::MinValue() ); }
    static CONSTEXPR value_type LowestValue() NOEXCEPT { return value_type(scalar_type::LowestValue()); }
    static CONSTEXPR value_type Nan() NOEXCEPT { return value_type( scalar_type::Nan() ); }
    static CONSTEXPR value_type Zero() NOEXCEPT { return value_type( scalar_type::Zero() ); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#if defined(BUILD_LINK_DYNAMIC)
#   ifdef __clang__
// Temporary workaround for LLVM 17: remove when LNK2001 link issue if fixed
#       define EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(T, N)
#       define EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(T, N)
#   else
#       define EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(T, N) EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) details::TScalarVectorExpr<T, N, details::TScalarVectorAssignable<T, N, details::TScalarVectorStorage<T, N> >>
#       define EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(T, N)
#   endif
#else
#   define EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(T, N) EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) details::TScalarVectorExpr<T, N, details::TScalarVectorAssignable<T, N, details::TScalarVectorStorage<T, N> >>
#   define EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(T, N) EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) details::TScalarVectorExpr<T, N, details::TScalarVectorAssignable<T, N, details::TScalarVectorStorage<T, N> >>
#endif

#ifndef EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL
#   define EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(T, N) EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) details::TScalarVectorExpr<T, N, details::TScalarVectorAssignable<T, N, details::TScalarVectorStorage<T, N> >>
#endif
#ifndef EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF
#   define EXTERN_RUNTIME_CORE_SCALARVECTOR_DEF(T, N) EXTERN_TEMPLATE_STRUCT_DEF(PPE_CORE_API) details::TScalarVectorExpr<T, N, details::TScalarVectorAssignable<T, N, details::TScalarVectorStorage<T, N> >>
#endif

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(bool, 1);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(bool, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(bool, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(bool, 4);
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(int, 1);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(int, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(int, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(int, 4);
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(unsigned int, 1);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(unsigned int, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(unsigned int, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(unsigned int, 4);
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(float, 1);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(float, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(float, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(float, 4);
//----------------------------------------------------------------------------
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(double, 1);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(double, 2);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(double, 3);
EXTERN_RUNTIME_CORE_SCALARVECTOR_DECL(double, 4);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

PRAGMA_RESTORE_RUNTIMECHECKS
