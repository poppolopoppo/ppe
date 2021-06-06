#pragma once

#include "Core.h"

#include "Maths/ScalarVector_fwd.h"

#include "Container/Hash.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/NumericLimits.h"

#include <algorithm>
#include <limits>
#include <type_traits>

// https://godbolt.org/z/3773ef

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Fwd declaration for promotion
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TScalarVectorExpr;
template < typename T, size_t _Dim>
constexpr auto ExpandScalarVectorExpr(const TScalarVectorExpr<T, _Dim>& v) noexcept;
template <typename U, typename T, size_t _Dim>
CONSTEXPR TScalarVector<U, _Dim> PromoteScalarVectorExpr(const TScalarVectorExpr<T, _Dim>& v) NOEXCEPT;
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR auto Length(const TScalarVector<T, _Dim>& v) NOEXCEPT;
template <typename T, size_t _Dim>
CONSTEXPR auto LengthSq(const TScalarVector<T, _Dim>& v) NOEXCEPT;
template <typename T, size_t _Dim>
CONSTEXPR auto Normalize(const TScalarVector<T, _Dim>& v) NOEXCEPT;
//----------------------------------------------------------------------------
// TScalarVectorExpr<> with CRTP
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TScalarVectorExpr {
    STATIC_ASSERT(_Dim);
    static CONSTEXPR size_t Dim = _Dim;
    using expr_type = T;

    FORCE_INLINE CONSTEXPR expr_type& ref() NOEXCEPT {
        return static_cast<expr_type&>(*this);
    }

    FORCE_INLINE CONSTEXPR const expr_type& ref() const NOEXCEPT {
        return static_cast<const expr_type&>(*this);
    }

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR auto get() NOEXCEPT {
        return ref().template get<_Idx>();
    }

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR auto get() const NOEXCEPT {
        return ref().template get<_Idx>();
    }

    template <typename U>
    auto BitCast() const NOEXCEPT;
    template <typename U>
    CONSTEXPR auto Cast() const NOEXCEPT;
    template <typename U>
    auto CastChecked() const NOEXCEPT;

    // those functions should expand the expression before operating for performance reasons.
    // workaround for user-defined conversion not being considered when doing template argument deduction,
    // using inline friend functions which are hence not templated.
    // https://stackoverflow.com/questions/3888082/implicit-conversion-not-happening
    // https://stackoverflow.com/questions/44834415/implicit-template-type-deduction-with-two-arguments-of-different-types

    inline friend CONSTEXPR auto Length(const TScalarVectorExpr& v) noexcept {
        return Length(ExpandScalarVectorExpr(v));
    }
    inline friend CONSTEXPR auto LengthSq(const TScalarVectorExpr& v) noexcept {
        return LengthSq(ExpandScalarVectorExpr(v));
    }
    inline friend CONSTEXPR auto Normalize(const TScalarVectorExpr& v) noexcept {
        return Normalize(ExpandScalarVectorExpr(v));
    }
};
//----------------------------------------------------------------------------
// TScalarVectorComponent<> : .x, .y, .z & .w
//----------------------------------------------------------------------------
template <typename T, size_t _Idx>
struct TScalarVectorComponent {
    using component_type = T;

    component_type data[1];

    CONSTEXPR TScalarVectorComponent() NOEXCEPT = default;

#if defined(CPP_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Warray-bounds"
#endif

    FORCE_INLINE CONSTEXPR TScalarVectorComponent(component_type v) NOEXCEPT {
        data[_Idx] = v;
    }

    FORCE_INLINE CONSTEXPR TScalarVectorComponent& operator =(component_type v) NOEXCEPT {
        data[_Idx] = v;
        return (*this);
    }

    FORCE_INLINE CONSTEXPR component_type& get() NOEXCEPT { return data[_Idx]; }
    FORCE_INLINE CONSTEXPR const component_type& get() const NOEXCEPT { return data[_Idx]; }

    FORCE_INLINE CONSTEXPR operator component_type& () NOEXCEPT { return data[_Idx]; }
    FORCE_INLINE CONSTEXPR operator const component_type& () const NOEXCEPT { return data[_Idx]; }

#if defined(CPP_CLANG)
#   pragma clang diagnostic pop
#endif

    FORCE_INLINE CONSTEXPR component_type& operator +=(component_type scalar) NOEXCEPT {
        return (get() += scalar);
    }
    FORCE_INLINE CONSTEXPR component_type& operator -=(component_type scalar) NOEXCEPT {
        return (get() -= scalar);
    }
    FORCE_INLINE CONSTEXPR component_type& operator *=(component_type scalar) NOEXCEPT {
        return (get() *= scalar);
    }
    FORCE_INLINE CONSTEXPR component_type& operator /=(component_type scalar) NOEXCEPT {
        return (get() /= scalar);
    }
};
//----------------------------------------------------------------------------
// TScalarComponent<> : traits for selecting compatible scalar types
//----------------------------------------------------------------------------
template <typename _Expr, typename T>
using TScalarComponent = Meta::TEnableIf<
    std::is_arithmetic_v<T>,
    std::common_type_t<
        T,
        decltype(std::declval<_Expr>().template get<0>())
    >
>;
//----------------------------------------------------------------------------
// TScalarVectorConstant<> : wraps a constant value
//----------------------------------------------------------------------------
template <typename T, int... _Values>
struct TScalarVectorConstant : TScalarVectorExpr<TScalarVectorConstant<T, _Values...>, sizeof...(_Values)> {
    static CONSTEXPR int values[sizeof...(_Values)] = { _Values... };
    using component_type = T;

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR component_type get() const NOEXCEPT { return static_cast<component_type>(values[_Idx]); }
};
//----------------------------------------------------------------------------
// TScalarVectorLiteral<> : expands a scalar value
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TScalarVectorLiteral : TScalarVectorExpr<TScalarVectorLiteral<T, _Dim>, _Dim> {
    using component_type = T;

    T value;

    FORCE_INLINE CONSTEXPR TScalarVectorLiteral(T _value) NOEXCEPT
        : value(_value)
    {}

    template <size_t>
    FORCE_INLINE CONSTEXPR component_type get() const NOEXCEPT { return value; }

    CONSTEXPR operator TScalarVector<T, _Dim> () const NOEXCEPT { return PromoteScalarVectorExpr<T>(*this); }
};
//----------------------------------------------------------------------------
// TScalarVectorBinaryOp<> : +, -, * & / used with another vector
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, typename _Op>
struct TScalarVectorBinaryOp : TScalarVectorExpr<TScalarVectorBinaryOp<_Lhs, _Rhs, _Op>, _Lhs::Dim> {
    using TScalarVectorExpr<TScalarVectorBinaryOp<_Lhs, _Rhs, _Op>, _Lhs::Dim>::Dim;
    STATIC_ASSERT(_Lhs::Dim == _Rhs::Dim);

    const typename _Lhs::expr_type& lhs;
    const typename _Rhs::expr_type& rhs;
    _Op op;

    FORCE_INLINE CONSTEXPR TScalarVectorBinaryOp(const _Lhs& _lhs, const _Rhs& _rhs, _Op&& _op) NOEXCEPT
        : lhs(_lhs.ref())
        , rhs(_rhs.ref())
        , op(std::move(_op))
    {}

    using component_type = std::decay_t<decltype(
        std::declval<_Op>()(
            std::declval<const _Lhs&>().template get<0>(),
            std::declval<const _Rhs&>().template get<0>() )
    )>;

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR component_type get() const NOEXCEPT {
        return op(lhs.template get<_Idx>(), rhs.template get<_Idx>());
    }
};
//----------------------------------------------------------------------------
template <typename U, typename V, size_t _Dim>
CONSTEXPR auto VECTORCALL operator +(const TScalarVectorExpr<U, _Dim>& lhs, const TScalarVectorExpr<V, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT { return a + b; });
}
//----------------------------------------------------------------------------
template <typename U, typename V, size_t _Dim>
CONSTEXPR auto VECTORCALL operator -(const TScalarVectorExpr<U, _Dim>& lhs, const TScalarVectorExpr<V, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT { return a - b; });
}
//----------------------------------------------------------------------------
template <typename U, typename V, size_t _Dim>
CONSTEXPR auto VECTORCALL operator *(const TScalarVectorExpr<U, _Dim>& lhs, const TScalarVectorExpr<V, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT { return a * b; });
}
//----------------------------------------------------------------------------
template <typename U, typename V, size_t _Dim>
CONSTEXPR auto VECTORCALL operator /(const TScalarVectorExpr<U, _Dim>& lhs, const TScalarVectorExpr<V, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorBinaryOp(lhs, rhs, [](auto a, auto b) CONSTEXPR NOEXCEPT { return a / b; });
}
//----------------------------------------------------------------------------
// TScalarVectorUnaryOp<> : unary operators or binary operators with scalar
//----------------------------------------------------------------------------
template <typename _Expr, typename _Op>
struct TScalarVectorUnaryOp : TScalarVectorExpr<TScalarVectorUnaryOp<_Expr, _Op>, _Expr::Dim> {
    using TScalarVectorExpr<TScalarVectorUnaryOp<_Expr, _Op>, _Expr::Dim>::Dim;

    const typename _Expr::expr_type& v;
    _Op op;

    FORCE_INLINE CONSTEXPR TScalarVectorUnaryOp(const _Expr& _v, _Op&& _op) NOEXCEPT
        : v(_v.ref())
        , op(std::move(_op))
    {}

    using component_type = std::decay_t<decltype(
        std::declval<_Op>()(
            std::declval<const _Expr&>().template get<0>() )
    )>;

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR component_type get() const NOEXCEPT {
        return op(v.template get<_Idx>());
    }
};
//----------------------------------------------------------------------------
template <typename U, size_t _Dim>
CONSTEXPR auto VECTORCALL operator -(const TScalarVectorExpr<U, _Dim>& e) NOEXCEPT {
    return TScalarVectorUnaryOp(e, [](auto x) CONSTEXPR NOEXCEPT { return (-x); });
}
template <typename U, size_t _Dim>
CONSTEXPR auto VECTORCALL  operator ~(const TScalarVectorExpr<U, _Dim>& e) NOEXCEPT {
    return TScalarVectorUnaryOp(e, [](auto x) CONSTEXPR NOEXCEPT { return (~x); });
}
//----------------------------------------------------------------------------
template <typename U, size_t _Dim, typename T, size_t _Idx>
CONSTEXPR auto VECTORCALL operator +(const TScalarVectorExpr<U, _Dim>& lhs, const TScalarVectorComponent<T, _Idx>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(lhs, [&rhs](auto x) CONSTEXPR NOEXCEPT { return (x + rhs); });
}
template <typename U, size_t _Dim, typename T, size_t _Idx>
CONSTEXPR auto VECTORCALL operator -(const TScalarVectorExpr<U, _Dim>& lhs, const TScalarVectorComponent<T, _Idx>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(lhs, [&rhs](auto x) CONSTEXPR NOEXCEPT { return (x - rhs); });
}
template <typename U, size_t _Dim, typename T, size_t _Idx>
CONSTEXPR auto VECTORCALL operator *(const TScalarVectorExpr<U, _Dim>& lhs, const TScalarVectorComponent<T, _Idx>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(lhs, [&rhs](auto x) CONSTEXPR NOEXCEPT { return (x * rhs); });
}
template <typename U, size_t _Dim, typename T, size_t _Idx>
CONSTEXPR auto VECTORCALL operator /(const TScalarVectorExpr<U, _Dim>& lhs, const TScalarVectorComponent<T, _Idx>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(lhs, [&rhs](auto x) CONSTEXPR NOEXCEPT { return (x / rhs); });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Idx, typename U, size_t _Dim>
CONSTEXPR auto VECTORCALL operator +(const TScalarVectorComponent<T, _Idx>& lhs, const TScalarVectorExpr<U, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(rhs, [&lhs](auto x) CONSTEXPR NOEXCEPT { return (lhs + x); });
}
template <typename T, size_t _Idx, typename U, size_t _Dim>
CONSTEXPR auto VECTORCALL operator -(const TScalarVectorComponent<T, _Idx>& lhs, const TScalarVectorExpr<U, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(rhs, [&lhs](auto x) CONSTEXPR NOEXCEPT { return (lhs - x); });
}
template <typename T, size_t _Idx, typename U, size_t _Dim>
CONSTEXPR auto VECTORCALL operator *(const TScalarVectorComponent<T, _Idx>& lhs, const TScalarVectorExpr<U, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(rhs, [&lhs](auto x) CONSTEXPR NOEXCEPT { return (lhs * x); });
}
template <typename T, size_t _Idx, typename U, size_t _Dim>
CONSTEXPR auto VECTORCALL operator /(const TScalarVectorComponent<T, _Idx>& lhs, const TScalarVectorExpr<U, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(rhs, [&lhs](auto x) CONSTEXPR NOEXCEPT { return (lhs / x); });
}
//----------------------------------------------------------------------------
template <typename U, size_t _Dim, typename V, typename _Result = TScalarComponent<TScalarVectorExpr<U, _Dim>, V> >
CONSTEXPR auto VECTORCALL operator +(const TScalarVectorExpr<U, _Dim>& lhs, const V& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(lhs, [rhs](auto x) CONSTEXPR NOEXCEPT -> _Result { return (x + rhs); });
}
template <typename U, size_t _Dim, typename V, typename _Result = TScalarComponent<TScalarVectorExpr<U, _Dim>, V> >
CONSTEXPR auto VECTORCALL operator -(const TScalarVectorExpr<U, _Dim>& lhs, const V& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(lhs, [rhs](auto x) CONSTEXPR NOEXCEPT -> _Result { return (x - rhs); });
}
template <typename U, size_t _Dim, typename V, typename _Result = TScalarComponent<TScalarVectorExpr<U, _Dim>, V> >
CONSTEXPR auto VECTORCALL operator *(const TScalarVectorExpr<U, _Dim>& lhs, const V& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(lhs, [rhs](auto x) CONSTEXPR NOEXCEPT -> _Result { return (x * rhs); });
}
template <typename U, size_t _Dim, typename V, typename _Result = TScalarComponent<TScalarVectorExpr<U, _Dim>, V> >
CONSTEXPR auto VECTORCALL operator /(const TScalarVectorExpr<U, _Dim>& lhs, const V& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(lhs, [rhs](auto x) CONSTEXPR NOEXCEPT -> _Result { return (x / rhs); });
}
//----------------------------------------------------------------------------
template <typename V, typename U, size_t _Dim, typename _Result = TScalarComponent<TScalarVectorExpr<U, _Dim>, V> >
CONSTEXPR auto VECTORCALL operator +(const V& lhs, const TScalarVectorExpr<U, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(rhs, [lhs](auto x) CONSTEXPR NOEXCEPT -> _Result { return (lhs + x); });
}
template <typename V, typename U, size_t _Dim, typename _Result = TScalarComponent<TScalarVectorExpr<U, _Dim>, V> >
CONSTEXPR auto VECTORCALL operator -(const V& lhs, const TScalarVectorExpr<U, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(rhs, [lhs](auto x) CONSTEXPR NOEXCEPT -> _Result { return (lhs - x); });
}
template <typename V, typename U, size_t _Dim, typename _Result = TScalarComponent<TScalarVectorExpr<U, _Dim>, V> >
CONSTEXPR auto VECTORCALL operator *(const V& lhs, const TScalarVectorExpr<U, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(rhs, [lhs](auto x) CONSTEXPR NOEXCEPT -> _Result { return (lhs * x); });
}
template <typename V, typename U, size_t _Dim, typename _Result = TScalarComponent<TScalarVectorExpr<U, _Dim>, V> >
CONSTEXPR auto VECTORCALL operator /(const V& lhs, const TScalarVectorExpr<U, _Dim>& rhs) NOEXCEPT {
    return TScalarVectorUnaryOp(rhs, [lhs](auto x) CONSTEXPR NOEXCEPT -> _Result { return (lhs / x); });
}
//----------------------------------------------------------------------------
// TScalarVectorAssignable<> : =, +=, -=, *= & /=, both with vectors and scalars
//----------------------------------------------------------------------------
template <typename _Expr, typename T, size_t _Dim>
struct TScalarVectorAssignable : TScalarVectorExpr<_Expr, _Dim> {
    using TScalarVectorExpr<_Expr, _Dim>::Dim;
    using TScalarVectorExpr<_Expr, _Dim>::get;

    using component_type = T;

    template <typename U>
    CONSTEXPR auto VECTORCALL operator =(const TScalarVectorExpr<U, Dim>& other) NOEXCEPT {
        return assign(other, [](auto& dst, const auto& src) CONSTEXPR NOEXCEPT { dst = src; });
    }

    template <typename U>
    CONSTEXPR auto VECTORCALL operator +=(const TScalarVectorExpr<U, Dim>& other) NOEXCEPT {
        return assign(other, [](auto& dst, const auto& src) CONSTEXPR NOEXCEPT { dst += src; });
    }
    template <typename U>
    CONSTEXPR auto VECTORCALL operator -=(const TScalarVectorExpr<U, Dim>& other) NOEXCEPT {
        return assign(other, [](auto& dst, const auto& src) CONSTEXPR NOEXCEPT { dst -= src; });
    }
    template <typename U>
    CONSTEXPR auto VECTORCALL operator *=(const TScalarVectorExpr<U, Dim>& other) NOEXCEPT {
        return assign(other, [](auto& dst, const auto& src) CONSTEXPR NOEXCEPT { dst *= src; });
    }
    template <typename U>
    CONSTEXPR auto VECTORCALL operator /=(const TScalarVectorExpr<U, Dim>& other) NOEXCEPT {
        return assign(other, [](auto& dst, const auto& src) CONSTEXPR NOEXCEPT { dst /= src; });
    }

    template <typename U, typename _Op>
    CONSTEXPR _Expr& VECTORCALL assign(const TScalarVectorExpr<U, Dim>& vec, _Op&& op) NOEXCEPT {
        Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT{
            ((op(static_cast<_Expr&>(*this).template get<idx>(), vec.template get<idx>())), ...);
        });
        return static_cast<_Expr&>(*this);
    }

    FORCE_INLINE CONSTEXPR _Expr& operator +=(component_type scalar) NOEXCEPT {
        return assign(scalar, [](auto& dst, const auto& src) CONSTEXPR NOEXCEPT { dst += src; });
    }
    FORCE_INLINE CONSTEXPR _Expr& operator -=(component_type scalar) NOEXCEPT {
        return assign(scalar, [](auto& dst, const auto& src) CONSTEXPR NOEXCEPT { dst -= src; });
    }
    FORCE_INLINE CONSTEXPR _Expr& operator *=(component_type scalar) NOEXCEPT {
        return assign(scalar, [](auto& dst, const auto& src) CONSTEXPR NOEXCEPT { dst *= src; });
    }
    FORCE_INLINE CONSTEXPR _Expr& operator /=(component_type scalar) NOEXCEPT {
        return assign(scalar, [](auto& dst, const auto& src) CONSTEXPR NOEXCEPT { dst /= src; });
    }

    template <typename _Op>
    CONSTEXPR _Expr& assign(component_type scalar, _Op&& op) NOEXCEPT {
        Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
            ((op(static_cast<_Expr&>(*this).template get<idx>(), scalar)), ...);
        });
        return static_cast<_Expr&>(*this);
    }
};
//----------------------------------------------------------------------------
// TScalarVectorSwizzle<> : .xy, .yx, .wzyx, ... which are assignable
//----------------------------------------------------------------------------
template <typename T, size_t... _Swizzle>
struct TScalarVectorSwizzle : TScalarVectorAssignable<TScalarVectorSwizzle<T, _Swizzle...>, T, sizeof...(_Swizzle)> {
    using parent_type = TScalarVectorAssignable<TScalarVectorSwizzle<T, _Swizzle...>, T, sizeof...(_Swizzle)>;
    using parent_type::Dim;
    using parent_type::operator =;
    using parent_type::operator +=;
    using parent_type::operator -=;
    using parent_type::operator *=;
    using parent_type::operator /=;

    using component_type = T;

    component_type data[Dim];

    CONSTEXPR TScalarVectorSwizzle() NOEXCEPT = default;

    static CONSTEXPR size_t indices[Dim] = { _Swizzle... };
    template <size_t _Idx>
    static CONSTEXPR size_t swizzled = indices[_Idx];

#if defined(CPP_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Warray-bounds"
#endif

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR component_type& get() NOEXCEPT { return data[swizzled<_Idx>]; }

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR const component_type& get() const NOEXCEPT { return data[swizzled<_Idx>]; }

#if defined(CPP_CLANG)
#   pragma clang diagnostic pop
#endif

    // specialization for trivial promotions when the swizzle indices are contiguous :

    using promote_type = decltype(PromoteScalarVectorExpr<T>(std::declval<parent_type&>()));
    using promote_const_type = decltype(PromoteScalarVectorExpr<T>(std::declval<const parent_type&>()));

#if !defined(_MSC_VER) || defined(CPP_CLANG) || defined(CPP_GCC) || (_MSC_VER >= 1927)
    CONSTEXPR operator promote_type () NOEXCEPT { return PromoteScalarVectorExpr<T>(*this); }
    CONSTEXPR operator promote_const_type () const NOEXCEPT { return PromoteScalarVectorExpr<T>(*this); }

#else // workaround a bug with MSVC compiler, #TODO remove when fixed since this is not valid C++
    // https://developercommunity.visualstudio.com/content/problem/149701/c2833-with-operator-decltype.html#reply-152822
    // => Seems to be fixed with VS19 16.7.0 Preview 3.1 at least, waiting for the fix to be released in the main branch
    constexpr operator auto () noexcept -> promote_type { return PromoteScalarVectorExpr<T>(*this); }
    constexpr operator auto () const noexcept -> promote_const_type { return PromoteScalarVectorExpr<T>(*this); }

#endif //!LOL
};
//----------------------------------------------------------------------------
// Equality comparaisons : ==, !=
//----------------------------------------------------------------------------
template <typename _Lhs, size_t _Dim, typename _Rhs>
CONSTEXPR bool operator ==(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
        return ((lhs.template get<idx>() == rhs.template get<idx>()) & ...); // use '&' instead '&&' to remove branches
    });
}
//----------------------------------------------------------------------------
template <typename _Lhs, typename _Rhs, size_t _Dim>
CONSTEXPR bool operator !=(const TScalarVectorExpr<_Lhs, _Dim>& lhs, const TScalarVectorExpr<_Rhs, _Dim>& rhs) NOEXCEPT {
    return (not operator ==(lhs, rhs));
}
//----------------------------------------------------------------------------
// Explicit cast : implemented as an unary operator
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto TScalarVectorExpr<T, _Dim>::BitCast() const NOEXCEPT {
    return TScalarVectorUnaryOp(*this, [](auto x) -> U { return bit_cast<U>(x); });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
CONSTEXPR auto TScalarVectorExpr<T, _Dim>::Cast() const NOEXCEPT {
    return TScalarVectorUnaryOp(*this, [](auto x) -> U { return static_cast<U>(x); });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto TScalarVectorExpr<T, _Dim>::CastChecked() const NOEXCEPT {
    return TScalarVectorUnaryOp(*this, [](auto x) -> U { return checked_cast<U>(x); });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TScalarVector;
//----------------------------------------------------------------------------
// TScalarVector<T, 1>, silly but needed by some templates
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVector<T, 1> : TScalarVectorAssignable<TScalarVector<T, 1>, T, 1> {
    static CONSTEXPR size_t Dim = 1;
    using component_type = T;
    using parent_type = TScalarVectorAssignable<TScalarVector<T, 1>, T, 1>;
    using parent_type::operator =;
    using parent_type::operator +=;
    using parent_type::operator -=;
    using parent_type::operator *=;
    using parent_type::operator /=;

    CONSTEXPR TScalarVector() NOEXCEPT = default;
    CONSTEXPR TScalarVector(Meta::FForceInit) NOEXCEPT : TScalarVector(Meta::MakeForceInit<T>()) {}

    CONSTEXPR TScalarVector(const TScalarVector& other) NOEXCEPT = default;
    CONSTEXPR TScalarVector& operator =(const TScalarVector& other) NOEXCEPT = default;

    FORCE_INLINE CONSTEXPR explicit TScalarVector(component_type broadcast) NOEXCEPT
        : data{ broadcast }
    {}

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(const TScalarVectorExpr<_Expr, 1>& e) NOEXCEPT
        : data{ e.template get<0>() }
    {}

    union {
        component_type data[Dim];

        TScalarVectorComponent<T, 0> x;
    };

    CONSTEXPR void Broadcast(T v) NOEXCEPT { x = v; }

#if USE_PPE_ASSERT
    component_type& operator [](size_t i) NOEXCEPT { Assert(i < Dim); return data[i]; }
    const component_type& operator [](size_t i) const NOEXCEPT { Assert(i < Dim); return data[i]; }
#else
    CONSTEXPR component_type& operator [](size_t i) NOEXCEPT { return data[i]; }
    CONSTEXPR const component_type& operator [](size_t i) const NOEXCEPT { return data[i]; }
#endif

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR component_type& get() NOEXCEPT { return data[_Idx]; }

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR const component_type& get() const NOEXCEPT { return data[_Idx]; }

    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> One{ component_type(1) };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MinusOne{ component_type(-1) };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> Zero{ component_type(0) };

    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MaxValue{ TNumericLimits<component_type>::MaxValue() };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MinValue{ TNumericLimits<component_type>::MinValue() };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> Lowest{ TNumericLimits<component_type>::Lowest() };
};
//----------------------------------------------------------------------------
// TScalarVector<T, 2>
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVector<T, 2> : TScalarVectorAssignable<TScalarVector<T, 2>, T, 2> {
    static CONSTEXPR size_t Dim = 2;
    using component_type = T;
    using parent_type = TScalarVectorAssignable<TScalarVector<T, 2>, T, 2>;
    using parent_type::operator =;
    using parent_type::operator +=;
    using parent_type::operator -=;
    using parent_type::operator *=;
    using parent_type::operator /=;

    CONSTEXPR TScalarVector() NOEXCEPT = default;
    CONSTEXPR TScalarVector(Meta::FForceInit) NOEXCEPT : TScalarVector(Meta::MakeForceInit<T>()) {}

    CONSTEXPR TScalarVector(const TScalarVector& other) NOEXCEPT = default;
    CONSTEXPR TScalarVector& operator =(const TScalarVector& other) NOEXCEPT = default;

    FORCE_INLINE CONSTEXPR explicit TScalarVector(component_type broadcast) NOEXCEPT
        : data{ broadcast, broadcast }
    {}

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(const TScalarVectorExpr<_Expr, 2>& e) NOEXCEPT
        : data{ e.template get<0>(),
                e.template get<1>() }
    {}

    FORCE_INLINE CONSTEXPR TScalarVector(component_type _x, component_type _y) NOEXCEPT
        : data{ _x, _y }
    {}

    union {
        component_type data[Dim];

        TScalarVectorComponent<T, 0> x;
        TScalarVectorComponent<T, 1> y;

        TScalarVectorSwizzle<T, 0, 1> xy;
        TScalarVectorSwizzle<T, 1, 0> yx;
    };

    CONSTEXPR void Broadcast(T v) NOEXCEPT { x = y = v; }
    CONSTEXPR auto Shift() const NOEXCEPT { return  TScalarVector<T, 1>(x); }

#if USE_PPE_ASSERT
    component_type& operator [](size_t i) NOEXCEPT { Assert(i < Dim); return data[i]; }
    const component_type& operator [](size_t i) const NOEXCEPT { Assert(i < Dim); return data[i]; }
#else
    CONSTEXPR component_type& operator [](size_t i) NOEXCEPT { return data[i]; }
    CONSTEXPR const component_type& operator [](size_t i) const NOEXCEPT { return data[i]; }
#endif

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR component_type& get() NOEXCEPT { return data[_Idx]; }

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR const component_type& get() const NOEXCEPT { return data[_Idx]; }

    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> One{ component_type(1) };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MinusOne{ component_type(-1) };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> Zero{ component_type(0) };

    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MaxValue{ TNumericLimits<component_type>::MaxValue() };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MinValue{ TNumericLimits<component_type>::MinValue() };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> Lowest{ TNumericLimits<component_type>::Lowest() };

    static CONSTEXPR TScalarVectorConstant<component_type, 1, 0> X{};
    static CONSTEXPR TScalarVectorConstant<component_type, 0, 1> Y{};
};
//----------------------------------------------------------------------------
// TScalarVector<T, 3>
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVector<T, 3> : TScalarVectorAssignable<TScalarVector<T, 3>, T, 3> {
    static CONSTEXPR size_t Dim = 3;
    using component_type = T;
    using parent_type = TScalarVectorAssignable<TScalarVector<T, 3>, T, 3>;
    using parent_type::operator =;
    using parent_type::operator +=;
    using parent_type::operator -=;
    using parent_type::operator *=;
    using parent_type::operator /=;

    CONSTEXPR TScalarVector() NOEXCEPT = default;
    CONSTEXPR TScalarVector(Meta::FForceInit) NOEXCEPT : TScalarVector(Meta::MakeForceInit<T>()) {}

    CONSTEXPR TScalarVector(const TScalarVector& other) NOEXCEPT = default;
    CONSTEXPR TScalarVector& operator =(const TScalarVector& other) NOEXCEPT = default;

    FORCE_INLINE CONSTEXPR explicit TScalarVector(component_type broadcast) NOEXCEPT
        : data{ broadcast, broadcast, broadcast }
    {}

    FORCE_INLINE CONSTEXPR TScalarVector(component_type _x, component_type _y, component_type _z) NOEXCEPT
        : data{ _x, _y, _z }
    {}

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(const TScalarVectorExpr<_Expr, 3>& e) NOEXCEPT
        : data{ e.template get<0>(),
                e.template get<1>(),
                e.template get<2>() }
    {}

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(component_type _x, const TScalarVectorExpr<_Expr, 2>& _yz) NOEXCEPT
        : data{ _x,
                _yz.template get<0>(),
                _yz.template get<1>() }
    {}

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(const TScalarVectorExpr<_Expr, 2>& _xy, component_type _z) NOEXCEPT
        : data{ _xy.template get<0>(),
                _xy.template get<1>(),
                _z }
    {}

    union {
        component_type data[Dim];

        TScalarVectorComponent<T, 0> x;
        TScalarVectorComponent<T, 1> y;
        TScalarVectorComponent<T, 2> z;

        TScalarVectorSwizzle<T, 0, 1> xy;
        TScalarVectorSwizzle<T, 0, 2> xz;
        TScalarVectorSwizzle<T, 1, 0> yx;
        TScalarVectorSwizzle<T, 1, 2> yz;
        TScalarVectorSwizzle<T, 2, 0> zx;
        TScalarVectorSwizzle<T, 2, 1> zy;

        TScalarVectorSwizzle<T, 0, 1, 2> xyz;
        TScalarVectorSwizzle<T, 0, 2, 1> xzy;
        TScalarVectorSwizzle<T, 1, 0, 2> yxz;
        TScalarVectorSwizzle<T, 1, 2, 0> yzx;
        TScalarVectorSwizzle<T, 2, 0, 1> zxy;
        TScalarVectorSwizzle<T, 2, 1, 0> zyx;
    };

    CONSTEXPR void Broadcast(T v) NOEXCEPT { x = y = z = v; }
    CONSTEXPR const auto& Shift() const NOEXCEPT { return xy; }

#if USE_PPE_ASSERT
    component_type& operator [](size_t i) NOEXCEPT { Assert(i < Dim); return data[i]; }
    const component_type& operator [](size_t i) const NOEXCEPT { Assert(i < Dim); return data[i]; }
#else
    CONSTEXPR component_type& operator [](size_t i) NOEXCEPT { return data[i]; }
    CONSTEXPR const component_type& operator [](size_t i) const NOEXCEPT { return data[i]; }
#endif

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR component_type& get() NOEXCEPT { return data[_Idx]; }

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR const component_type& get() const NOEXCEPT { return data[_Idx]; }

    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> One{ component_type(1) };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MinusOne{ component_type(-1) };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> Zero{ component_type(0) };

    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MaxValue{ TNumericLimits<component_type>::MaxValue() };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MinValue{ TNumericLimits<component_type>::MinValue() };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> Lowest{ TNumericLimits<component_type>::Lowest() };

    static CONSTEXPR TScalarVectorConstant<component_type, 1, 0, 0> X{};
    static CONSTEXPR TScalarVectorConstant<component_type, 0, 1, 0> Y{};
    static CONSTEXPR TScalarVectorConstant<component_type, 0, 0, 1> Z{};
};
//----------------------------------------------------------------------------
// TScalarVector<T, 4>
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVector<T, 4> : TScalarVectorAssignable<TScalarVector<T, 4>, T, 4> {
    static CONSTEXPR size_t Dim = 4;
    using component_type = T;
    using parent_type = TScalarVectorAssignable<TScalarVector<T, 4>, T, 4>;
    using parent_type::operator =;
    using parent_type::operator +=;
    using parent_type::operator -=;
    using parent_type::operator *=;
    using parent_type::operator /=;

    CONSTEXPR TScalarVector() NOEXCEPT = default;
    CONSTEXPR TScalarVector(Meta::FForceInit) NOEXCEPT : TScalarVector(Meta::MakeForceInit<T>()) {}

    CONSTEXPR TScalarVector(const TScalarVector& other) NOEXCEPT = default;
    CONSTEXPR TScalarVector& operator =(const TScalarVector& other) NOEXCEPT = default;

    FORCE_INLINE CONSTEXPR explicit TScalarVector(component_type broadcast) NOEXCEPT
        : data{ broadcast, broadcast, broadcast, broadcast }
    {}

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(const TScalarVectorExpr<_Expr, 4>& e) NOEXCEPT
        : data{ e.template get<0>(),
                e.template get<1>(),
                e.template get<2>(),
                e.template get<3>() }
    {}

    FORCE_INLINE CONSTEXPR TScalarVector(component_type _x, component_type _y, component_type _z, component_type _w) NOEXCEPT
        : data{ _x, _y, _z, _w }
    {}

    template <typename _Lo, typename _Hi>
    FORCE_INLINE CONSTEXPR TScalarVector(const TScalarVectorExpr<_Lo, 2>& _xy, const TScalarVectorExpr<_Hi, 2>& _zw) NOEXCEPT
        : data{ _xy.template get<0>(),
                _xy.template get<1>(),
                _zw.template get<0>(),
                _zw.template get<1>() }
    {}

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(const TScalarVectorExpr<_Expr, 2>& _xy, component_type _z, component_type _w) NOEXCEPT
        : data{ _xy.template get<0>(),
                _xy.template get<1>(),
                _z,
                _w }
    {}

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(const TScalarVectorExpr<_Expr, 3>& _xyz, component_type _w) NOEXCEPT
        : data{ _xyz.template get<0>(),
                _xyz.template get<1>(),
                _xyz.template get<2>(),
                _w }
    {}

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(component_type _x, const TScalarVectorExpr<_Expr, 3>& _yzw) NOEXCEPT
        : data{ _x,
                _yzw.template get<0>(),
                _yzw.template get<1>(),
                _yzw.template get<2>() }
    {}

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(component_type _x, component_type _y, const TScalarVectorExpr<_Expr, 2>& _zw) NOEXCEPT
        : data{ _x,
                _y,
                _zw.template get<0>(),
                _zw.template get<1>() }
    {}

    union {
        component_type data[Dim];

        TScalarVectorComponent<T, 0> x;
        TScalarVectorComponent<T, 1> y;
        TScalarVectorComponent<T, 2> z;
        TScalarVectorComponent<T, 3> w;

        TScalarVectorSwizzle<T, 0, 1> xy;
        TScalarVectorSwizzle<T, 0, 2> xz;
        TScalarVectorSwizzle<T, 0, 3> xw;
        TScalarVectorSwizzle<T, 1, 0> yx;
        TScalarVectorSwizzle<T, 1, 2> yz;
        TScalarVectorSwizzle<T, 1, 3> yw;
        TScalarVectorSwizzle<T, 2, 0> zx;
        TScalarVectorSwizzle<T, 2, 1> zy;
        TScalarVectorSwizzle<T, 2, 3> zw;
        TScalarVectorSwizzle<T, 3, 0> wx;
        TScalarVectorSwizzle<T, 3, 1> wy;
        TScalarVectorSwizzle<T, 3, 2> wz;

        TScalarVectorSwizzle<T, 0, 1, 2> xyz;
        TScalarVectorSwizzle<T, 0, 2, 1> xzy;
        TScalarVectorSwizzle<T, 0, 1, 3> xyw;
        TScalarVectorSwizzle<T, 0, 3, 1> xwy;
        TScalarVectorSwizzle<T, 0, 2, 3> xzw;
        TScalarVectorSwizzle<T, 0, 3, 2> xwz;
        TScalarVectorSwizzle<T, 1, 0, 2> yxz;
        TScalarVectorSwizzle<T, 1, 2, 0> yzx;
        TScalarVectorSwizzle<T, 1, 0, 3> yxw;
        TScalarVectorSwizzle<T, 1, 3, 0> ywx;
        TScalarVectorSwizzle<T, 1, 2, 3> yzw;
        TScalarVectorSwizzle<T, 1, 3, 2> ywz;
        TScalarVectorSwizzle<T, 2, 0, 1> zxy;
        TScalarVectorSwizzle<T, 2, 1, 0> zyx;
        TScalarVectorSwizzle<T, 2, 0, 3> zxw;
        TScalarVectorSwizzle<T, 2, 3, 0> zwx;
        TScalarVectorSwizzle<T, 2, 1, 3> zyw;
        TScalarVectorSwizzle<T, 2, 3, 1> zwy;
        TScalarVectorSwizzle<T, 3, 0, 1> wxy;
        TScalarVectorSwizzle<T, 3, 1, 0> wyx;
        TScalarVectorSwizzle<T, 3, 0, 2> wxz;
        TScalarVectorSwizzle<T, 3, 2, 0> wzx;
        TScalarVectorSwizzle<T, 3, 1, 2> wyz;
        TScalarVectorSwizzle<T, 3, 2, 1> wzy;

        TScalarVectorSwizzle<T, 0, 1, 2, 3> xyzw;
        TScalarVectorSwizzle<T, 0, 1, 3, 2> xywz;
        TScalarVectorSwizzle<T, 0, 2, 1, 3> xzyw;
        TScalarVectorSwizzle<T, 0, 2, 3, 1> xzwy;
        TScalarVectorSwizzle<T, 0, 3, 1, 2> xwyz;
        TScalarVectorSwizzle<T, 0, 3, 2, 1> xwzy;
        TScalarVectorSwizzle<T, 1, 0, 2, 3> yxzw;
        TScalarVectorSwizzle<T, 1, 0, 3, 2> yxwz;
        TScalarVectorSwizzle<T, 1, 2, 0, 3> yzxw;
        TScalarVectorSwizzle<T, 1, 2, 3, 0> yzwx;
        TScalarVectorSwizzle<T, 1, 3, 0, 2> ywxz;
        TScalarVectorSwizzle<T, 1, 3, 2, 0> ywzx;
        TScalarVectorSwizzle<T, 2, 0, 1, 3> zxyw;
        TScalarVectorSwizzle<T, 2, 0, 3, 1> zxwy;
        TScalarVectorSwizzle<T, 2, 1, 0, 3> zyxw;
        TScalarVectorSwizzle<T, 2, 1, 3, 0> zywx;
        TScalarVectorSwizzle<T, 2, 3, 0, 1> zwxy;
        TScalarVectorSwizzle<T, 2, 3, 1, 0> zwyx;
        TScalarVectorSwizzle<T, 3, 0, 1, 2> wxyz;
        TScalarVectorSwizzle<T, 3, 0, 2, 1> wxzy;
        TScalarVectorSwizzle<T, 3, 1, 0, 2> wyxz;
        TScalarVectorSwizzle<T, 3, 1, 2, 0> wyzx;
        TScalarVectorSwizzle<T, 3, 2, 0, 1> wzxy;
        TScalarVectorSwizzle<T, 3, 2, 1, 0> wzyx;
    };

    CONSTEXPR void Broadcast(T v) NOEXCEPT { x = y = z = w = v; }
    CONSTEXPR const auto& Shift() const NOEXCEPT { return xyz; }

#if USE_PPE_ASSERT
    component_type& operator [](size_t i) NOEXCEPT { Assert(i < Dim); return data[i]; }
    const component_type& operator [](size_t i) const NOEXCEPT { Assert(i < Dim); return data[i]; }
#else
    CONSTEXPR component_type& operator [](size_t i) NOEXCEPT { return data[i]; }
    CONSTEXPR const component_type& operator [](size_t i) const NOEXCEPT { return data[i]; }
#endif

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR component_type& get() NOEXCEPT { return data[_Idx]; }

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR const component_type& get() const NOEXCEPT { return data[_Idx]; }

    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> One{ component_type(1) };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MinusOne{ component_type(-1) };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> Zero{ component_type(0) };

    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MaxValue{ TNumericLimits<component_type>::MaxValue() };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MinValue{ TNumericLimits<component_type>::MinValue() };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> Lowest{ TNumericLimits<component_type>::Lowest() };

    static CONSTEXPR TScalarVectorConstant<component_type, 1, 0, 0, 0> X{};
    static CONSTEXPR TScalarVectorConstant<component_type, 0, 1, 0, 0> Y{};
    static CONSTEXPR TScalarVectorConstant<component_type, 0, 0, 1, 0> Z{};
    static CONSTEXPR TScalarVectorConstant<component_type, 0, 0, 0, 1> W{};
};
//----------------------------------------------------------------------------
// TScalarVector<T, N> : general fallback for larger vectors needed by some templates
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TScalarVector : TScalarVectorAssignable<TScalarVector<T, _Dim>, T, _Dim> {
    static CONSTEXPR size_t Dim = _Dim;
    using component_type = T;
    using parent_type = TScalarVectorAssignable<TScalarVector<T, _Dim>, T, _Dim>;
    using parent_type::operator =;
    using parent_type::operator +=;
    using parent_type::operator -=;
    using parent_type::operator *=;
    using parent_type::operator /=;

    CONSTEXPR TScalarVector() NOEXCEPT = default;
    CONSTEXPR TScalarVector(Meta::FForceInit) NOEXCEPT : TScalarVector(Meta::MakeForceInit<T>()) {}

    CONSTEXPR TScalarVector(const TScalarVector& other) NOEXCEPT = default;
    CONSTEXPR TScalarVector& operator =(const TScalarVector& other) NOEXCEPT = default;

    FORCE_INLINE CONSTEXPR explicit TScalarVector(component_type broadcast) NOEXCEPT {
        Broadcast(broadcast);
    }

    template <typename _Expr>
    FORCE_INLINE CONSTEXPR TScalarVector(const TScalarVectorExpr<_Expr, _Dim>& e) NOEXCEPT {
        Meta::static_for<Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
            ((data[idx] = e.template get<idx>()), ...);
        });
    }

    component_type data[Dim];

    CONSTEXPR void Broadcast(T v) NOEXCEPT {
        Meta::static_for<Dim>([&](auto... idx) CONSTEXPR NOEXCEPT {
            ((data[idx] = v), ...);
        });
    }
    CONSTEXPR const auto& Shift() const NOEXCEPT {
        return Meta::static_for<_Dim - 1>([this](auto... idx) NOEXCEPT {
            return reinterpret_cast<const TScalarVectorSwizzle<T, idx...>&>(this);
        });
    }

#if USE_PPE_ASSERT
    component_type& operator [](size_t i) NOEXCEPT { Assert(i < _Dim); return data[i]; }
    const component_type& operator [](size_t i) const NOEXCEPT { Assert(i < _Dim); return data[i]; }
#else
    CONSTEXPR component_type& operator [](size_t i) NOEXCEPT { return data[i]; }
    CONSTEXPR const component_type& operator [](size_t i) const NOEXCEPT { return data[i]; }
#endif

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR component_type& get() NOEXCEPT { return data[_Idx]; }

    template <size_t _Idx>
    FORCE_INLINE CONSTEXPR const component_type& get() const NOEXCEPT { return data[_Idx]; }

    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> One{ component_type(1) };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MinusOne{ component_type(-1) };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> Zero{ component_type(0) };

    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MaxValue{ TNumericLimits<component_type>::MaxValue() };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> MinValue{ TNumericLimits<component_type>::MinValue() };
    static CONSTEXPR TScalarVectorLiteral<component_type, Dim> Lowest{ TNumericLimits<component_type>::Lowest() };
};
//----------------------------------------------------------------------------
// ExpandScalarVectorExpr() : convert an expression to a vector of deduced type
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR auto ExpandScalarVectorExpr(const TScalarVectorExpr<T, _Dim>& v) NOEXCEPT {
    using component_type = std::decay_t<decltype(
        std::declval<const TScalarVectorExpr<T, _Dim>&>().template get<0>()
    )>;
    return PromoteScalarVectorExpr<component_type>(v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR const TScalarVector<T, _Dim>& ExpandScalarVectorExpr(const TScalarVector<T, _Dim>& v) NOEXCEPT {
    return v; // specialize for TVector<> : skip copy with trivial projection
}
//----------------------------------------------------------------------------
// PromoteScalarVectorExpr() : convert an expression to a vector
//----------------------------------------------------------------------------
template <typename U, typename T, size_t _Dim>
CONSTEXPR TScalarVector<U, _Dim> PromoteScalarVectorExpr(const TScalarVectorExpr<T, _Dim>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR NOEXCEPT -> TScalarVector<U, _Dim> {
        return { v.template get<idx>()... };
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
CONSTEXPR const TScalarVector<T, _Dim>& PromoteScalarVectorExpr(const TScalarVector<T, _Dim>& v) NOEXCEPT {
    return v; // avoid copy for TScalarVector<>
}
//----------------------------------------------------------------------------
// PromoteScalarVectorExpr() specializations to avoid copy
//----------------------------------------------------------------------------
#define PPE_MATH_PROMOTEVECTOREXPR_NOCOPY_SWIZZLE(_DIM, _OFF, ...) \
    template <typename T> \
    FORCE_INLINE CONSTEXPR TScalarVector<T, _DIM>& PromoteScalarVectorExpr(TScalarVectorAssignable<TScalarVectorSwizzle<T, __VA_ARGS__>, T, _DIM>& v) NOEXCEPT { \
        return reinterpret_cast<TScalarVector<T, _DIM>&>(static_cast<TScalarVectorSwizzle<T, __VA_ARGS__>&>(v).data[_OFF]); \
    } \
    template <typename T> \
    FORCE_INLINE CONSTEXPR const TScalarVector<T, _DIM>& PromoteScalarVectorExpr(const TScalarVectorAssignable<TScalarVectorSwizzle<T, __VA_ARGS__>, T, _DIM>& v) NOEXCEPT { \
        return reinterpret_cast<const TScalarVector<T, _DIM>&>(static_cast<const TScalarVectorSwizzle<T, __VA_ARGS__>&>(v).data[_OFF]); \
    }

// All contiguous swizzles :

PPE_MATH_PROMOTEVECTOREXPR_NOCOPY_SWIZZLE(2, 0,/* xy   */0, 1)
PPE_MATH_PROMOTEVECTOREXPR_NOCOPY_SWIZZLE(2, 1,/* yz   */1, 2)
PPE_MATH_PROMOTEVECTOREXPR_NOCOPY_SWIZZLE(2, 2,/* zw   */2, 3)

PPE_MATH_PROMOTEVECTOREXPR_NOCOPY_SWIZZLE(3, 0,/* xyz  */0, 1, 2)
PPE_MATH_PROMOTEVECTOREXPR_NOCOPY_SWIZZLE(3, 1,/* yzw  */1, 2, 3)

PPE_MATH_PROMOTEVECTOREXPR_NOCOPY_SWIZZLE(4, 0,/* xyzw */0, 1, 2, 3)

#undef PPE_MATH_PROMOTEVECTOREXPR_NOCOPY_SWIZZLE
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// All scalar vectors are considered as pods
PPE_ASSUME_TEMPLATE_AS_POD(TScalarVector<T COMMA _Dim>, typename T, size_t _Dim)
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
hash_t hash_value(const TScalarVector<T, _Dim>& v) NOEXCEPT {
    return hash_as_pod(v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void swap(TScalarVector<T, _Dim>& lhs, TScalarVector<T, _Dim>& rhs) NOEXCEPT {
    return std::swap(lhs.data, rhs.data);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename T, size_t _Dim >
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TScalarVector<T, _Dim>& v) {
    oss << STRING_LITERAL(_Char, '[') << v.data[0];
    forrange(i, 1, _Dim)
        oss << STRING_LITERAL(_Char, ", ") << v.data[i];
    oss << STRING_LITERAL(_Char, ']');
    return oss;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
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
    static CONSTEXPR value_type Nan() NOEXCEPT { return value_type( scalar_type::Nan() ); }
    static CONSTEXPR value_type Zero() NOEXCEPT { return value_type( scalar_type::Zero() ); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
