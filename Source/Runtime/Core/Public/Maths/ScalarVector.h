#pragma once

#include "Core.h"

#include "Maths/ScalarVector_fwd.h"

#include "Container/Hash.h"
#include "IO/TextWriter_fwd.h"
#include "Meta/NumericLimits.h"

#include <algorithm>
#include <limits>
#include <type_traits>

// https://godbolt.org/z/dEvY3n1zq

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
template <typename T, size_t _Dim>
struct TScalarVector;
//----------------------------------------------------------------------------
// TScalarVectorExpr<T, N, Expr>
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr = void>
struct TScalarVectorExpr {
    using component_type = T;
    static CONSTEXPR const size_t dim = _Dim;

    CONSTEXPR TScalarVectorExpr() = default;

    NODISCARD CONSTEXPR operator TScalarVector<T, _Dim>() const { return Expand(); }

    NODISCARD CONSTEXPR _Expr& ref() {
        return static_cast<_Expr&>(*this);
    }
    NODISCARD CONSTEXPR const _Expr& ref() const {
        return static_cast<const _Expr&>(*this);
    }

    template <size_t _Index>
    NODISCARD CONSTEXPR component_type& get() {
        static_assert(_Index < _Dim, "out-of-bounds");
        return ref().template get<_Index>();
    }
    template <size_t _Index>
    NODISCARD CONSTEXPR component_type get() const {
        static_assert(_Index < _Dim, "out-of-bounds");
        return ref().template get<_Index>();
    }

    NODISCARD CONSTEXPR auto HSum() const;

    NODISCARD CONSTEXPR auto MaxComponent() const;
    NODISCARD CONSTEXPR auto MinComponent() const;

    NODISCARD CONSTEXPR TScalarVector<T, _Dim> Expand() const;
    NODISCARD CONSTEXPR auto Shift() const;
    template <size_t... _Idx>
    NODISCARD CONSTEXPR TScalarVector<T, sizeof...(_Idx)> Shuffle() const;

};
//----------------------------------------------------------------------------
// Specialization for broadcast constant
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TScalarVectorExpr<T, _Dim, void> {
    using component_type = T;
    static CONSTEXPR const size_t dim = _Dim;

    const component_type literal{};

    CONSTEXPR explicit TScalarVectorExpr(const component_type& value) : literal(value) {}

    template <size_t _Index>
    NODISCARD CONSTEXPR auto& get() {
        return literal;
    }
    template <size_t _Index>
    NODISCARD CONSTEXPR auto get() const {
        return literal;
    }

    NODISCARD CONSTEXPR auto HSum() const {
        return Meta::static_for<_Dim>([this](auto... idx) {
            return (this->template get<idx>() + ...);
        });
    }

    NODISCARD CONSTEXPR auto MaxComponent() const { return literal; }
    NODISCARD CONSTEXPR auto MinComponent() const { return literal; }

    template <typename _Transform, typename A>
    NODISCARD static CONSTEXPR auto Map(_Transform callable, const TScalarVectorExpr<T, _Dim, A>& a);
    template <typename _Transform, typename A, typename B>
    NODISCARD static CONSTEXPR auto Map(_Transform callable, const TScalarVectorExpr<T, _Dim, A>& a, const TScalarVectorExpr<T, _Dim, B>& b);
    template <typename _Transform, typename A, typename B, typename C>
    NODISCARD static CONSTEXPR auto Map(_Transform callable, const TScalarVectorExpr<T, _Dim, A>& a, const TScalarVectorExpr<T, _Dim, B>& b, const TScalarVectorExpr<T, _Dim, C>& c);

};
//----------------------------------------------------------------------------
// TScalarVectorLiteral<>
//----------------------------------------------------------------------------
namespace details {
template <typename _x, typename... _yzw>
CONSTEXPR auto MakeScalarVector(const _x& x, _yzw... yzw) {
    return TScalarVector<_x, 1+sizeof...(_yzw)>{ x, yzw... };
}
} //!details
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Dim>
using TScalarVectorLiteral = TScalarVectorExpr<T, _Dim>;
} //!details
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Dim, size_t _Axis>
struct TScalarVectorAxis : TScalarVectorExpr<T, _Dim, TScalarVectorAxis<T, _Dim, _Axis>> {
    using parent_type = TScalarVectorExpr<T, _Dim, TScalarVectorAxis<T, _Dim, _Axis>>;
    using typename parent_type::component_type;

    template <size_t _Index>
    NODISCARD CONSTEXPR auto get() const {
        return (_Axis == _Index ? static_cast<T>(1) : static_cast<T>(0));
    }
};
template <typename T, size_t _Dim, size_t _Axis>
struct TScalarVectorAxisDecl : TScalarVectorAxisDecl<T, _Dim, _Axis-1> {};
template <typename T, size_t _Dim>
struct TScalarVectorAxisDecl<T, _Dim, 0> {
    static CONSTEXPR const TScalarVectorAxis<T, _Dim, 0> X{};
};
template <typename T, size_t _Dim>
struct TScalarVectorAxisDecl<T, _Dim, 1> : TScalarVectorAxisDecl<T, _Dim, 0> {
    static CONSTEXPR const TScalarVectorAxis<T, _Dim, 1> Y{};
};
template <typename T, size_t _Dim>
struct TScalarVectorAxisDecl<T, _Dim, 2> : TScalarVectorAxisDecl<T, _Dim, 1> {
    static CONSTEXPR const TScalarVectorAxis<T, _Dim, 2> Z{};
};
template <typename T, size_t _Dim>
struct TScalarVectorAxisDecl<T, _Dim, 3> : TScalarVectorAxisDecl<T, _Dim, 2> {
    static CONSTEXPR const TScalarVectorAxis<T, _Dim, 3> W{};
};
} //!details
//----------------------------------------------------------------------------
// TScalarVectorAssignable<>
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Dim, typename _Expr, size_t... _Idx>
struct EMPTY_BASES TScalarVectorAssignable : TScalarVectorExpr<T, _Dim, _Expr>, TScalarVectorAxisDecl<T, _Dim, _Dim-1> {
    using parent_type = TScalarVectorExpr<T, _Dim, _Expr>;
    using typename parent_type::component_type;

    CONSTEXPR TScalarVectorAssignable() = default;

    CONSTEXPR TScalarVectorAssignable(Meta::FForceInit) {
        Broadcast(Meta::DefaultValue<component_type>());
    }

    template <size_t _Index>
    using get_component_t = T;

    CONSTEXPR TScalarVectorAssignable(const get_component_t<_Idx>&... in) {
        FOLD_EXPR( this->template get<_Idx>() = in );
    }

    template <typename _Other>
    CONSTEXPR TScalarVectorAssignable(const TScalarVectorExpr<T, _Dim, _Other>& src) {
        Assign(src);
    }
    template <typename _Other>
    CONSTEXPR TScalarVectorAssignable& operator =(const TScalarVectorExpr<T, _Dim, _Other>& src) {
        return Assign(src);
    }

    template <typename _Src, typename _Other>
    using enable_if_assignable_t = std::enable_if_t<
        std::is_nothrow_assignable_v<T&, _Src>
    >;

    template <typename _Src, typename _Other, typename = enable_if_assignable_t<_Src, _Other> >
    CONSTEXPR explicit TScalarVectorAssignable(const TScalarVectorExpr<_Src, _Dim, _Other>& src) {
        Assign(src);
    }

    NODISCARD CONSTEXPR auto& data() { return TScalarVectorExpr<T, _Dim, _Expr>::ref().data; }
    NODISCARD CONSTEXPR const auto& data() const { return TScalarVectorExpr<T, _Dim, _Expr>::ref().data; }

    NODISCARD CONSTEXPR component_type& operator [](size_t idx) {
        Assert(idx < _Dim);
        return data()[idx];
    }
    NODISCARD CONSTEXPR const component_type& operator [](size_t idx) const {
        Assert(idx < _Dim);
        return data()[idx];
    }

    template <typename _Src, typename _Other = void, typename = enable_if_assignable_t<_Src, _Other> >
    CONSTEXPR _Expr& Assign(const TScalarVectorExpr<_Src, _Dim, _Other>& src) NOEXCEPT;
    CONSTEXPR _Expr& Broadcast(const component_type& value) NOEXCEPT;
    CONSTEXPR _Expr& Set(const get_component_t<_Idx>&... in) NOEXCEPT;

    NODISCARD CONSTEXPR TMemoryView<component_type> MakeView() NOEXCEPT;
    NODISCARD CONSTEXPR TMemoryView<const component_type> MakeView() const;

    NODISCARD CONSTEXPR size_t MaxComponentIndex() const;
    NODISCARD CONSTEXPR size_t MinComponentIndex() const;

    static CONSTEXPR const TScalarVectorLiteral<T, _Dim> Zero{ static_cast<T>(0) };
    static CONSTEXPR const TScalarVectorLiteral<T, _Dim> One{ static_cast<T>(1) };
    static CONSTEXPR const TScalarVectorLiteral<T, _Dim> MinusOne{ static_cast<T>(-1) };

    static CONSTEXPR const TScalarVectorLiteral<T, _Dim> MaxValue{ TNumericLimits<T>::MaxValue() };
    static CONSTEXPR const TScalarVectorLiteral<T, _Dim> MinValue{ TNumericLimits<T>::MinValue() };
    static CONSTEXPR const TScalarVectorLiteral<T, _Dim> Lowest{ TNumericLimits<T>::Lowest() };

    template <typename _Other>
    friend void swap(TScalarVectorAssignable& lhs, TScalarVectorAssignable<T, _Dim, _Other, _Idx...>& rhs) {
        using std::swap;
        FOLD_EXPR( swap(lhs.template get<_Idx>(), rhs.template get<_Idx>()) );
    }

};
template <typename T, size_t _Dim, typename _Expr, typename _Indices>
struct TMakeScalarVectorAssignable_;
template <typename T, size_t _Dim, typename _Expr, size_t... _Idx>
struct TMakeScalarVectorAssignable_<T, _Dim, _Expr, std::index_sequence<_Idx...>> {
    using type = TScalarVectorAssignable<T, _Dim, _Expr, _Idx...>;
};
template <typename T, size_t _Dim, typename _Expr>
using TMakeScalarVectorAssignable = typename TMakeScalarVectorAssignable_<T, _Dim, _Expr, std::make_index_sequence<_Dim>>::type;
} //!details
//----------------------------------------------------------------------------
// TScalarVectorSwizzle<T, N, Indices...>
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Dim, size_t... _Swizzle>
struct TScalarVectorSwizzle : TMakeScalarVectorAssignable<T, sizeof...(_Swizzle), TScalarVectorSwizzle<T, _Dim, _Swizzle...>> {
    using component_type = T;

    static CONSTEXPR size_t indices[sizeof...(_Swizzle)] = { _Swizzle... };

    T data[_Dim];

    template <size_t _Idx>
    NODISCARD CONSTEXPR component_type& get() {
        static_assert(indices[_Idx] < _Dim, "out-of-bounds");
        return data[indices[_Idx]];
    }
    template <size_t _Idx>
    NODISCARD CONSTEXPR const component_type& get() const {
        static_assert(indices[_Idx] < _Dim, "out-of-bounds");
        return data[indices[_Idx]];
    }

    using parent_type = TMakeScalarVectorAssignable<T, sizeof...(_Swizzle), TScalarVectorSwizzle<T, _Dim, _Swizzle...>>;
    using parent_type::parent_type;
    using parent_type::operator =;
    using parent_type::operator [];
};
} //!details
//----------------------------------------------------------------------------
// TScalarVector<T, N>
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TScalarVector : details::TMakeScalarVectorAssignable<T, _Dim, TScalarVector<T, _Dim>> {
    using component_type = T;

    union {
        T data[_Dim];
    };

    CONSTEXPR TScalarVector() = default;

    CONSTEXPR explicit TScalarVector(const component_type& broadcast) {
        Broadcast(broadcast);
    }

    template <size_t _Index>
    CONSTEXPR component_type& get() {
        static_assert(_Index < _Dim, "out-of-bounds");
        return data[_Index];
    }
    template <size_t _Index>
    CONSTEXPR const component_type& get() const {
        static_assert(_Index < _Dim, "out-of-bounds");
        return data[_Index];
    }

    using parent_type = details::TMakeScalarVectorAssignable<T, _Dim, TScalarVector<T, _Dim>>;
    using parent_type::parent_type;
    using parent_type::operator =;
    using parent_type::operator [];
};
//----------------------------------------------------------------------------
// TScalarVector<T, 1>
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVector<T, 1> : details::TMakeScalarVectorAssignable<T, 1, TScalarVector<T, 1>> {
    using component_type = T;

    union {
        T data[1];
        struct {
            T x;
        };

        //const details::TScalarVectorSwizzle<T, 1, 0, 0> xx;
        //const details::TScalarVectorSwizzle<T, 1, 0, 0, 0> xxx;
        //const details::TScalarVectorSwizzle<T, 1, 0, 0, 0, 0> xxxx;
    };

    CONSTEXPR TScalarVector() = default;

    CONSTEXPR explicit TScalarVector(Meta::FForceInit) : TScalarVector(Meta::DefaultValue<T>()) {}
    CONSTEXPR explicit TScalarVector(const component_type& broadcast) : data{
        broadcast } {}

    using parent_type = details::TMakeScalarVectorAssignable<T, 1, TScalarVector<T, 1>>;
    using parent_type::operator [];

    template <size_t _Index>
    NODISCARD CONSTEXPR component_type& get() {
        static_assert(_Index < 1, "out-of-bounds");
        return data[_Index];
    }
    template <size_t _Index>
    NODISCARD CONSTEXPR const component_type& get() const {
        static_assert(_Index < 1, "out-of-bounds");
        return data[_Index];
    }

    template <typename _Other>
    CONSTEXPR TScalarVector(const TScalarVectorExpr<T, 1, _Other>& src) {
        parent_type::Assign(src);
    }
    template <typename _Other>
    CONSTEXPR TScalarVector& operator =(const TScalarVectorExpr<T, 1, _Other>& src) {
        return parent_type::Assign(src);
    }

    template <typename _Src, typename _Other, typename = typename parent_type::template enable_if_assignable_t<_Src, _Other> >
    CONSTEXPR explicit TScalarVector(const TScalarVectorExpr<_Src, 1, _Other>& src) {
        parent_type::Assign(src);
    }
};
//----------------------------------------------------------------------------
// TScalarVector<T, 2>
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVector<T, 2> : details::TMakeScalarVectorAssignable<T, 2, TScalarVector<T, 2>> {
    using component_type = T;

    union {
        T data[2];
        struct {
            T x;
            T y;
        };

        //const details::TScalarVectorSwizzle<T, 2, 0, 0> xx;
        //const details::TScalarVectorSwizzle<T, 2, 1, 1> yy;

        //const details::TScalarVectorSwizzle<T, 2, 0, 0, 0> xxx;
        //const details::TScalarVectorSwizzle<T, 2, 1, 1, 1> yyy;

        //const details::TScalarVectorSwizzle<T, 2, 0, 0, 0, 0> xxxx;
        //const details::TScalarVectorSwizzle<T, 2, 1, 1, 1, 1> yyyy;

        //const details::TScalarVectorSwizzle<T, 2, 0, 1, 0, 1> xyxy;
        //const details::TScalarVectorSwizzle<T, 2, 1, 0, 1, 0> yxyx;
    };

    CONSTEXPR TScalarVector() = default;

    CONSTEXPR explicit TScalarVector(Meta::FForceInit) : TScalarVector(Meta::DefaultValue<T>()) {}
    CONSTEXPR explicit TScalarVector(const component_type& broadcast) : data{
        broadcast,
        broadcast } {}

    CONSTEXPR TScalarVector(
        const component_type& x_,
        const component_type& y_ ) : data{
        x_, y_ } {}

    template <size_t _Index>
    NODISCARD CONSTEXPR component_type& get() {
        static_assert(_Index < 2, "out-of-bounds");
        return data[_Index];
    }
    template <size_t _Index>
    NODISCARD CONSTEXPR const component_type& get() const {
        static_assert(_Index < 2, "out-of-bounds");
        return data[_Index];
    }

    using parent_type = details::TMakeScalarVectorAssignable<T, 2, TScalarVector<T, 2>>;
    using parent_type::operator [];

    template <typename _Other>
    CONSTEXPR TScalarVector(const TScalarVectorExpr<T, 2, _Other>& src) {
        parent_type::Assign(src);
    }
    template <typename _Other>
    CONSTEXPR TScalarVector& operator =(const TScalarVectorExpr<T, 2, _Other>& src) {
        return parent_type::Assign(src);
    }

    template <typename _Src, typename _Other, typename = typename parent_type::template enable_if_assignable_t<_Src, _Other> >
    CONSTEXPR explicit TScalarVector(const TScalarVectorExpr<_Src, 2, _Other>& src) {
        parent_type::Assign(src);
    }

    // static CONSTEXPR const details::TScalarVectorAxis<T, 2, 0> X{};
    // static CONSTEXPR const details::TScalarVectorAxis<T, 2, 1> Y{};
};
//----------------------------------------------------------------------------
// TScalarVector<T, 3>
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVector<T, 3> : details::TMakeScalarVectorAssignable<T, 3, TScalarVector<T, 3>> {
    using component_type = T;

    union {
        T data[3];
        struct {
            T x;
            T y;
            T z;
        };

        details::TScalarVectorSwizzle<T, 3, 0, 1> xy;
        details::TScalarVectorSwizzle<T, 3, 0, 2> xz;
        details::TScalarVectorSwizzle<T, 3, 1, 0> yx;
        details::TScalarVectorSwizzle<T, 3, 1, 2> yz;
        details::TScalarVectorSwizzle<T, 3, 2, 0> zx;
        details::TScalarVectorSwizzle<T, 3, 2, 1> zy;

        details::TScalarVectorSwizzle<T, 3, 0, 1, 2> xyz;
        details::TScalarVectorSwizzle<T, 3, 0, 2, 1> xzy;
        details::TScalarVectorSwizzle<T, 3, 1, 0, 2> yxz;
        details::TScalarVectorSwizzle<T, 3, 1, 2, 0> yzx;
        details::TScalarVectorSwizzle<T, 3, 2, 0, 1> zxy;
        details::TScalarVectorSwizzle<T, 3, 2, 1, 0> zyx;

        //const details::TScalarVectorSwizzle<T, 3, 0, 0> xx;
        //const details::TScalarVectorSwizzle<T, 3, 1, 1> yy;
        //const details::TScalarVectorSwizzle<T, 3, 2, 2> zz;

        //const details::TScalarVectorSwizzle<T, 3, 0, 0, 0> xxx;
        //const details::TScalarVectorSwizzle<T, 3, 1, 1, 1> yyy;
        //const details::TScalarVectorSwizzle<T, 3, 2, 2, 2> zzz;

        //const details::TScalarVectorSwizzle<T, 3, 0, 0, 0, 0> xxxx;
        //const details::TScalarVectorSwizzle<T, 3, 1, 1, 1, 1> yyyy;
        //const details::TScalarVectorSwizzle<T, 3, 2, 2, 2, 2> zzzz;

        //const details::TScalarVectorSwizzle<T, 3, 0, 1, 0, 1> xyxy;
        //const details::TScalarVectorSwizzle<T, 3, 1, 0, 1, 0> yxyx;
        //const details::TScalarVectorSwizzle<T, 3, 0, 2, 0, 2> xzxz;
        //const details::TScalarVectorSwizzle<T, 3, 1, 2, 1, 2> yzyz;
    };

    CONSTEXPR TScalarVector() = default;

    CONSTEXPR explicit TScalarVector(Meta::FForceInit) : TScalarVector(Meta::DefaultValue<T>()) {}
    CONSTEXPR explicit TScalarVector(const component_type& broadcast) : data{
        broadcast,
        broadcast,
        broadcast } {}

    CONSTEXPR TScalarVector(
        const component_type& x_,
        const component_type& y_,
        const component_type& z_ ) : data{
        x_, y_, z_ } {}

    template <typename _Expr>
    CONSTEXPR TScalarVector(
        const TScalarVectorExpr<T, 2, _Expr>& xy,
        const component_type& z_) :data{
        xy.template get<0>(),
        xy.template get<1>(),
        z_ } {}

    template <typename _Expr>
    CONSTEXPR TScalarVector(
        const component_type& x_,
        const TScalarVectorExpr<T, 2, _Expr>& yz) : data{
        x_,
        yz.template get<0>(),
        yz.template get<1>() } {}

    template <size_t _Index>
    NODISCARD CONSTEXPR component_type& get() {
        static_assert(_Index < 3, "out-of-bounds");
        return data[_Index];
    }
    template <size_t _Index>
    NODISCARD CONSTEXPR const component_type& get() const {
        static_assert(_Index < 3, "out-of-bounds");
        return data[_Index];
    }

    using parent_type = details::TMakeScalarVectorAssignable<T, 3, TScalarVector<T, 3>>;
    using parent_type::operator [];

    template <typename _Other>
    CONSTEXPR TScalarVector(const TScalarVectorExpr<T, 3, _Other>& src) {
        parent_type::Assign(src);
    }
    template <typename _Other>
    CONSTEXPR TScalarVector& operator =(const TScalarVectorExpr<T, 3, _Other>& src) {
        return parent_type::Assign(src);
    }

    template <typename _Src, typename _Other, typename = typename parent_type::template enable_if_assignable_t<_Src, _Other> >
    CONSTEXPR explicit TScalarVector(const TScalarVectorExpr<_Src, 3, _Other>& src) {
        parent_type::Assign(src);
    }

    CONSTEXPR const auto& Shift() const { return xy; }

    // static CONSTEXPR const details::TScalarVectorAxis<T, 3, 0> X{};
    // static CONSTEXPR const details::TScalarVectorAxis<T, 3, 1> Y{};
    // static CONSTEXPR const details::TScalarVectorAxis<T, 3, 2> Z{};
};
//----------------------------------------------------------------------------
// TScalarVector<T, 4>
//----------------------------------------------------------------------------
template <typename T>
struct TScalarVector<T, 4> : details::TMakeScalarVectorAssignable<T, 4, TScalarVector<T, 4>> {
    using component_type = T;

    union {
        T data[4];
        struct {
            T x;
            T y;
            T z;
            T w;
        };

        details::TScalarVectorSwizzle<T, 4, 0, 1> xy;
        details::TScalarVectorSwizzle<T, 4, 0, 2> xz;
        details::TScalarVectorSwizzle<T, 4, 0, 3> xw;
        details::TScalarVectorSwizzle<T, 4, 1, 0> yx;
        details::TScalarVectorSwizzle<T, 4, 1, 2> yz;
        details::TScalarVectorSwizzle<T, 4, 1, 3> yw;
        details::TScalarVectorSwizzle<T, 4, 2, 0> zx;
        details::TScalarVectorSwizzle<T, 4, 2, 1> zy;
        details::TScalarVectorSwizzle<T, 4, 2, 3> zw;
        details::TScalarVectorSwizzle<T, 4, 3, 0> wx;
        details::TScalarVectorSwizzle<T, 4, 3, 1> wy;
        details::TScalarVectorSwizzle<T, 4, 3, 2> wz;

        details::TScalarVectorSwizzle<T, 4, 0, 1, 2> xyz;
        details::TScalarVectorSwizzle<T, 4, 0, 2, 1> xzy;
        details::TScalarVectorSwizzle<T, 4, 0, 1, 3> xyw;
        details::TScalarVectorSwizzle<T, 4, 0, 3, 1> xwy;
        details::TScalarVectorSwizzle<T, 4, 0, 2, 3> xzw;
        details::TScalarVectorSwizzle<T, 4, 0, 3, 2> xwz;
        details::TScalarVectorSwizzle<T, 4, 1, 0, 2> yxz;
        details::TScalarVectorSwizzle<T, 4, 1, 2, 0> yzx;
        details::TScalarVectorSwizzle<T, 4, 1, 0, 3> yxw;
        details::TScalarVectorSwizzle<T, 4, 1, 3, 0> ywx;
        details::TScalarVectorSwizzle<T, 4, 1, 2, 3> yzw;
        details::TScalarVectorSwizzle<T, 4, 1, 3, 2> ywz;
        details::TScalarVectorSwizzle<T, 4, 2, 0, 1> zxy;
        details::TScalarVectorSwizzle<T, 4, 2, 1, 0> zyx;
        details::TScalarVectorSwizzle<T, 4, 2, 0, 3> zxw;
        details::TScalarVectorSwizzle<T, 4, 2, 3, 0> zwx;
        details::TScalarVectorSwizzle<T, 4, 2, 1, 3> zyw;
        details::TScalarVectorSwizzle<T, 4, 2, 3, 1> zwy;
        details::TScalarVectorSwizzle<T, 4, 3, 0, 1> wxy;
        details::TScalarVectorSwizzle<T, 4, 3, 1, 0> wyx;
        details::TScalarVectorSwizzle<T, 4, 3, 0, 2> wxz;
        details::TScalarVectorSwizzle<T, 4, 3, 2, 0> wzx;
        details::TScalarVectorSwizzle<T, 4, 3, 1, 2> wyz;
        details::TScalarVectorSwizzle<T, 4, 3, 2, 1> wzy;

        details::TScalarVectorSwizzle<T, 4, 0, 1, 2, 3> xyzw;
        details::TScalarVectorSwizzle<T, 4, 0, 1, 3, 2> xywz;
        details::TScalarVectorSwizzle<T, 4, 0, 2, 1, 3> xzyw;
        details::TScalarVectorSwizzle<T, 4, 0, 2, 3, 1> xzwy;
        details::TScalarVectorSwizzle<T, 4, 0, 3, 1, 2> xwyz;
        details::TScalarVectorSwizzle<T, 4, 0, 3, 2, 1> xwzy;
        details::TScalarVectorSwizzle<T, 4, 1, 0, 2, 3> yxzw;
        details::TScalarVectorSwizzle<T, 4, 1, 0, 3, 2> yxwz;
        details::TScalarVectorSwizzle<T, 4, 1, 2, 0, 3> yzxw;
        details::TScalarVectorSwizzle<T, 4, 1, 2, 3, 0> yzwx;
        details::TScalarVectorSwizzle<T, 4, 1, 3, 0, 2> ywxz;
        details::TScalarVectorSwizzle<T, 4, 1, 3, 2, 0> ywzx;
        details::TScalarVectorSwizzle<T, 4, 2, 0, 1, 3> zxyw;
        details::TScalarVectorSwizzle<T, 4, 2, 0, 3, 1> zxwy;
        details::TScalarVectorSwizzle<T, 4, 2, 1, 0, 3> zyxw;
        details::TScalarVectorSwizzle<T, 4, 2, 1, 3, 0> zywx;
        details::TScalarVectorSwizzle<T, 4, 2, 3, 0, 1> zwxy;
        details::TScalarVectorSwizzle<T, 4, 2, 3, 1, 0> zwyx;
        details::TScalarVectorSwizzle<T, 4, 3, 0, 1, 2> wxyz;
        details::TScalarVectorSwizzle<T, 4, 3, 0, 2, 1> wxzy;
        details::TScalarVectorSwizzle<T, 4, 3, 1, 0, 2> wyxz;
        details::TScalarVectorSwizzle<T, 4, 3, 1, 2, 0> wyzx;
        details::TScalarVectorSwizzle<T, 4, 3, 2, 0, 1> wzxy;
        details::TScalarVectorSwizzle<T, 4, 3, 2, 1, 0> wzyx;

        //const details::TScalarVectorSwizzle<T, 4, 0, 0> xx;
        //const details::TScalarVectorSwizzle<T, 4, 1, 1> yy;
        //const details::TScalarVectorSwizzle<T, 4, 2, 2> zz;
        //const details::TScalarVectorSwizzle<T, 4, 3, 3> ww;

        //const details::TScalarVectorSwizzle<T, 4, 0, 0, 0> xxx;
        //const details::TScalarVectorSwizzle<T, 4, 1, 1, 1> yyy;
        //const details::TScalarVectorSwizzle<T, 4, 2, 2, 2> zzz;
        //const details::TScalarVectorSwizzle<T, 4, 3, 3, 3> www;

        //const details::TScalarVectorSwizzle<T, 4, 0, 0, 0, 0> xxxx;
        //const details::TScalarVectorSwizzle<T, 4, 1, 1, 1, 1> yyyy;
        //const details::TScalarVectorSwizzle<T, 4, 2, 2, 2, 2> zzzz;
        //const details::TScalarVectorSwizzle<T, 4, 3, 3, 3, 3> wwww;

        //const details::TScalarVectorSwizzle<T, 4, 0, 1, 0, 1> xyxy;
        //const details::TScalarVectorSwizzle<T, 4, 1, 0, 1, 0> yxyx;
        //const details::TScalarVectorSwizzle<T, 4, 0, 2, 0, 2> xzxz;
        //const details::TScalarVectorSwizzle<T, 4, 1, 2, 1, 2> yzyz;
        //const details::TScalarVectorSwizzle<T, 4, 0, 3, 0, 3> xwxw;
        //const details::TScalarVectorSwizzle<T, 4, 1, 3, 1, 3> ywyw;
        //const details::TScalarVectorSwizzle<T, 4, 2, 3, 2, 3> zwzw;

    };

    CONSTEXPR TScalarVector() = default;

    CONSTEXPR explicit TScalarVector(Meta::FForceInit) : TScalarVector(Meta::DefaultValue<T>()) {}
    CONSTEXPR explicit TScalarVector(const component_type& broadcast) : data{
        broadcast,
        broadcast,
        broadcast,
        broadcast } {}

    CONSTEXPR TScalarVector(
        const component_type& x_,
        const component_type& y_,
        const component_type& z_,
        const component_type& w_ ) : data{
        x_, y_, z_, w_ } {}

    template <typename _Expr>
    CONSTEXPR TScalarVector(
        const TScalarVectorExpr<T, 2, _Expr>& xy,
        const component_type& z_,
        const component_type& w_) : data{
        xy.template get<0>(),
        xy.template get<1>(),
        z_,
        w_ } {}

    template <typename _Expr>
    CONSTEXPR TScalarVector(
        const TScalarVectorExpr<T, 3, _Expr>& xyz,
        const component_type& w_) : data{
        xyz.template get<0>(),
        xyz.template get<1>(),
        xyz.template get<2>(),
        w_ } {}

    template <typename _Expr>
    CONSTEXPR TScalarVector(
        const component_type& x_,
        const TScalarVectorExpr<T, 3, _Expr>& yzw) : data{
        x_,
        yzw.template get<0>(),
        yzw.template get<1>(),
        yzw.template get<2>() }{}

    template <typename _Expr>
    CONSTEXPR TScalarVector(
        const component_type& x_,
        const TScalarVectorExpr<T, 2, _Expr>& yz,
        const component_type& w_) : data{
        x_,
        yz.template get<0>(),
        yz.template get<1>(),
        w_ } {}

    template <typename _Expr>
    CONSTEXPR TScalarVector(
        const TScalarVectorExpr<T, 2, _Expr>& xy,
        const TScalarVectorExpr<T, 2, _Expr>& zw) : data{
        xy.template get<0>(),
        xy.template get<1>(),
        zw.template get<0>(),
        zw.template get<1>() }{}

    template <size_t _Index>
    NODISCARD CONSTEXPR component_type& get() {
        static_assert(_Index < 4, "out-of-bounds");
        return data[_Index];
    }
    template <size_t _Index>
    NODISCARD CONSTEXPR const component_type& get() const {
        static_assert(_Index < 4, "out-of-bounds");
        return data[_Index];
    }

    using parent_type = details::TMakeScalarVectorAssignable<T, 4, TScalarVector<T, 4>>;
    using parent_type::operator [];

    template <typename _Other>
    CONSTEXPR TScalarVector(const TScalarVectorExpr<T, 4, _Other>& src) {
        parent_type::Assign(src);
    }
    template <typename _Other>
    CONSTEXPR TScalarVector& operator =(const TScalarVectorExpr<T, 4, _Other>& src) {
        return parent_type::Assign(src);
    }

    template <typename _Src, typename _Other, typename = typename parent_type::template enable_if_assignable_t<_Src, _Other> >
    CONSTEXPR explicit TScalarVector(const TScalarVectorExpr<_Src, 4, _Other>& src) {
        parent_type::Assign(src);
    }

    CONSTEXPR const auto& Shift() const { return xyz; }

    // static CONSTEXPR const details::TScalarVectorAxis<T, 4, 0> X{};
    // static CONSTEXPR const details::TScalarVectorAxis<T, 4, 1> Y{};
    // static CONSTEXPR const details::TScalarVectorAxis<T, 4, 2> Z{};
    // static CONSTEXPR const details::TScalarVectorAxis<T, 4, 3> W{};
};
//----------------------------------------------------------------------------
// operator ==/!=
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR bool operator ==(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return ((lhs.template get<idx>() == rhs.template get<idx>()) && ...);
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Lhs, typename _Rhs>
NODISCARD CONSTEXPR auto operator !=(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) {
    return not operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
// TScalarVectorUnaryExpr
//----------------------------------------------------------------------------
namespace details {
template <typename _Dst, typename _Src, size_t _Dim, typename _Op, typename _Expr>
struct TScalarVectorUnaryExpr : TScalarVectorExpr<_Dst, _Dim,
    TScalarVectorUnaryExpr<_Dst, _Src, _Dim, _Op, _Expr>> {
    using component_type = _Dst;

    const TScalarVectorExpr<_Src, _Dim, _Expr>& Expr;

    CONSTEXPR explicit TScalarVectorUnaryExpr(
        const TScalarVectorExpr<_Src, _Dim, _Expr>& expr)
    :   Expr(expr)
    {}

    template <size_t _Index>
    NODISCARD CONSTEXPR auto get() const {
        return _Op{}( Expr.template get<_Index>() );
    }
};
template <typename _Op, typename T, size_t _Dim, typename _Expr>
CONSTEXPR auto MakeScalarVectorUnaryExpr(const TScalarVectorExpr<T, _Dim, _Expr>& expr) {
    using return_type = std::decay_t<decltype( (std::declval<_Op>())(
        std::declval<const TScalarVectorExpr<T, _Dim, _Expr>&>().template get<0>() ))>;
    return TScalarVectorUnaryExpr<return_type, T, _Dim, _Op, _Expr>(expr);
}
} //!details
//----------------------------------------------------------------------------
// TScalarVectorBinaryExpr
//----------------------------------------------------------------------------
namespace details {
template <typename _Dst, typename _Src, size_t _Dim, typename _Op, typename _Lhs, typename _Rhs>
struct TScalarVectorBinaryExpr : TScalarVectorExpr<_Dst, _Dim,
    TScalarVectorBinaryExpr<_Dst, _Src, _Dim, _Op, _Lhs, _Rhs>> {
    using component_type = _Dst;

    const TScalarVectorExpr<_Src, _Dim, _Lhs>& Lhs;
    const TScalarVectorExpr<_Src, _Dim, _Rhs>& Rhs;

    CONSTEXPR explicit TScalarVectorBinaryExpr(
        const TScalarVectorExpr<_Src, _Dim, _Lhs>& lhs,
        const TScalarVectorExpr<_Src, _Dim, _Rhs>& rhs )
    :   Lhs(lhs)
    ,   Rhs(rhs)
    {}

    template <size_t _Index>
    NODISCARD CONSTEXPR auto get() const {
        return _Op{}(Lhs.template get<_Index>(), Rhs.template get<_Index>());
    }
};
template <typename _Op, typename T, size_t _Dim, typename _Lhs, typename _Rhs>
CONSTEXPR auto MakeScalarVectorBinaryExpr(
    const TScalarVectorExpr<T, _Dim, _Lhs>& lhs,
    const TScalarVectorExpr<T, _Dim, _Rhs>& rhs ) {
    using return_type = std::decay_t<decltype( (std::declval<_Op>())(
        std::declval<const TScalarVectorExpr<T, _Dim, _Lhs>&>().template get<0>(),
        std::declval<const TScalarVectorExpr<T, _Dim, _Rhs>&>().template get<0>() ))>;
    return TScalarVectorBinaryExpr<return_type, T, _Dim, _Op, _Lhs, _Rhs>(lhs, rhs);
}
} //!details
//----------------------------------------------------------------------------
// TScalarVectorTernaryExpr
//----------------------------------------------------------------------------
namespace details {
template <typename _Dst, typename _Src, size_t _Dim, typename _Op, typename _A, typename _B, typename _C>
struct TScalarVectorTernaryExpr : TScalarVectorExpr<_Dst, _Dim,
    TScalarVectorTernaryExpr<_Dst, _Src, _Dim, _Op, _A, _B, _C>> {
    using component_type = _Dst;

    const TScalarVectorExpr<_Src, _Dim, _A>& A;
    const TScalarVectorExpr<_Src, _Dim, _B>& B;
    const TScalarVectorExpr<_Src, _Dim, _C>& C;

    CONSTEXPR explicit TScalarVectorTernaryExpr(
        const TScalarVectorExpr<_Src, _Dim, _A>& a,
        const TScalarVectorExpr<_Src, _Dim, _B>& b,
        const TScalarVectorExpr<_Src, _Dim, _C>& c )
    :   A(a)
    ,   B(b)
    ,   C(c)
    {}

    template <size_t _Index>
    NODISCARD CONSTEXPR auto get() const {
        return _Op{}(A.template get<_Index>(), B.template get<_Index>(), C.template get<_Index>());
    }
};
template <typename _Op, typename T, size_t _Dim, typename _A, typename _B, typename _C>
CONSTEXPR auto MakeScalarVectorTernaryExpr(
    const TScalarVectorExpr<T, _Dim, _A>& a,
    const TScalarVectorExpr<T, _Dim, _B>& b,
    const TScalarVectorExpr<T, _Dim, _C>& c ) {
    using return_type = std::decay_t<decltype( (std::declval<_Op>())(
        std::declval<const TScalarVectorExpr<T, _Dim, _A>&>().template get<0>(),
        std::declval<const TScalarVectorExpr<T, _Dim, _B>&>().template get<0>(),
        std::declval<const TScalarVectorExpr<T, _Dim, _C>&>().template get<0>() ))>;
    return TScalarVectorTernaryExpr<return_type, T, _Dim, _Op, _A, _B, _C>(a, b, c);
}
} //!details
//----------------------------------------------------------------------------
// TScalarVectorExpr<> helpers
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
CONSTEXPR auto TScalarVectorExpr<T, _Dim, _Expr>::HSum() const {
    return Meta::static_for<_Dim>([this](auto... idx) {
        return (this->template get<idx>() + ...);
    });
}
template <typename T, size_t _Dim, typename _Expr>
CONSTEXPR auto TScalarVectorExpr<T, _Dim, _Expr>::MaxComponent() const {
    return Meta::static_for<_Dim>([this](auto... idx) {
        return std::max({ this->template get<idx>()... });
    });
}
template <typename T, size_t _Dim, typename _Expr>
CONSTEXPR auto TScalarVectorExpr<T, _Dim, _Expr>::MinComponent() const {
    return Meta::static_for<_Dim>([this](auto... idx) {
        return std::min({ this->template get<idx>()... });
    });
}
template <typename T, size_t _Dim, typename _Expr>
CONSTEXPR TScalarVector<T, _Dim> TScalarVectorExpr<T, _Dim, _Expr>::Expand() const {
    return Meta::static_for<_Dim>([this](auto... idx) {
        return details::MakeScalarVector( this->template get<idx>()... );
    });
}
template <typename T, size_t _Dim, typename _Expr>
CONSTEXPR auto TScalarVectorExpr<T, _Dim, _Expr>::Shift() const {
    return Meta::static_for<_Dim - 1 ? _Dim - 1 : 1>([this](auto... idx) {
        return details::MakeScalarVector( this->template get<idx>()... );
    });
}
template <typename T, size_t _Dim, typename _Expr>
template <size_t... _Idx>
CONSTEXPR TScalarVector<T, sizeof...(_Idx)> TScalarVectorExpr<T, _Dim, _Expr>::Shuffle() const {
    return Meta::static_for<sizeof...(_Idx)>([this](auto... idx) {
        CONSTEXPR size_t shuffled[sizeof...(_Idx)] = { _Idx... };
        return details::MakeScalarVector( this->template get<shuffled[idx]>()... );
    });
}
template <typename T, size_t _Dim>
template <typename _Transform, typename A>
CONSTEXPR auto TScalarVectorExpr<T, _Dim, void>::Map(_Transform callable, const TScalarVectorExpr<T, _Dim, A>& a) {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return details::MakeScalarVector( callable(a.template get<idx>())... );
    });
}
template <typename T, size_t _Dim>
template <typename _Transform, typename A, typename B>
CONSTEXPR auto TScalarVectorExpr<T, _Dim, void>::Map(_Transform callable, const TScalarVectorExpr<T, _Dim, A>& a, const TScalarVectorExpr<T, _Dim, B>& b) {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return details::MakeScalarVector( callable(a.template get<idx>(), b.template get<idx>())... );
    });
}
template <typename T, size_t _Dim>
template <typename _Transform, typename A, typename B, typename C>
CONSTEXPR auto TScalarVectorExpr<T, _Dim, void>::Map(_Transform callable, const TScalarVectorExpr<T, _Dim, A>& a, const TScalarVectorExpr<T, _Dim, B>& b, const TScalarVectorExpr<T, _Dim, C>& c) {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return details::MakeScalarVector( callable(a.template get<idx>(), b.template get<idx>(), c.template get<idx>())... );
    });
}
//----------------------------------------------------------------------------
// TScalarVectorAssignable<> helpers
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Dim, typename _Expr, size_t... _Idx>
template <typename _Src, typename _Other, typename >
CONSTEXPR auto TScalarVectorAssignable<T, _Dim, _Expr, _Idx...>::Assign(const TScalarVectorExpr<_Src, _Dim, _Other>& src) NOEXCEPT -> _Expr& {
    return Set(static_cast<T>(src.template get<_Idx>())...);
}
template <typename T, size_t _Dim, typename _Expr, size_t... _Idx>
CONSTEXPR auto TScalarVectorAssignable<T, _Dim, _Expr, _Idx...>::Broadcast(const component_type& value) NOEXCEPT -> _Expr& {
    FOLD_EXPR( this->template get<_Idx>() = value );
    return parent_type::ref();
}
template <typename T, size_t _Dim, typename _Expr, size_t... _Idx>
CONSTEXPR auto TScalarVectorAssignable<T, _Dim, _Expr, _Idx...>::Set(const get_component_t<_Idx>&... in) NOEXCEPT -> _Expr& {
    FOLD_EXPR( this->template get<_Idx>() = in );
    return parent_type::ref();
}
template <typename T, size_t _Dim, typename _Expr, size_t... _Idx>
CONSTEXPR auto TScalarVectorAssignable<T, _Dim, _Expr, _Idx...>::MakeView() NOEXCEPT -> TMemoryView<component_type> {
    return PPE::MakeView(this->data());
}
template <typename T, size_t _Dim, typename _Expr, size_t... _Idx>
CONSTEXPR auto TScalarVectorAssignable<T, _Dim, _Expr, _Idx...>::MakeView() const -> TMemoryView<const component_type> {
    return PPE::MakeView(this->data());
}
template <typename T, size_t _Dim, typename _Expr, size_t... _Idx>
CONSTEXPR size_t TScalarVectorAssignable<T, _Dim, _Expr, _Idx...>::MaxComponentIndex() const {
    return Meta::static_for<_Dim-1>([this](auto... idx) {
        size_t result = 0;
        FOLD_EXPR( result = ((*this)[result] < (*this)[1+idx] ? 1+idx : result) );
        return result;
    });
}
template <typename T, size_t _Dim, typename _Expr, size_t... _Idx>
CONSTEXPR size_t TScalarVectorAssignable<T, _Dim, _Expr, _Idx...>::MinComponentIndex() const {
    return Meta::static_for<_Dim-1>([this](auto... idx) {
        size_t result = 0;
        FOLD_EXPR( result = ((*this)[result] < (*this)[1+idx] ? result : 1+idx) );
        return result;
    });
}
} //!details
//----------------------------------------------------------------------------
// unary operator -, ~
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
CONSTEXPR auto operator -(const TScalarVectorExpr<T, _Dim, _Expr>& e) {
    return details::MakeScalarVectorUnaryExpr<Meta::TUnaryMinus<T>>(e);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
CONSTEXPR auto operator ~(const TScalarVectorExpr<T, _Dim, _Expr>& e) {
    return details::MakeScalarVectorUnaryExpr<Meta::TUnaryComplement<T>>(e);
}
//----------------------------------------------------------------------------
template <typename _Dst, typename T, size_t _Dim, typename _Expr>
CONSTEXPR auto bit_cast(const TScalarVectorExpr<T, _Dim, _Expr>& e) {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return details::MakeScalarVector(bit_cast<_Dst>(e.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <typename _Dst, typename T, size_t _Dim, typename _Expr>
CONSTEXPR auto checked_cast(const TScalarVectorExpr<T, _Dim, _Expr>& e) {
    return Meta::static_for<_Dim>([&](auto... idx) {
        return details::MakeScalarVector(checked_cast<_Dst>(e.template get<idx>())...);
    });
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Expr>
hash_t hash_value(const TScalarVectorExpr<T, _Dim, _Expr>& v) NOEXCEPT {
    return Meta::static_for<_Dim>([&v](auto... idx) {
        return hash_tuple(v.template get<idx>()...);
    });
}
//----------------------------------------------------------------------------
// binary operator +, -, *, /, &, |, ^
//----------------------------------------------------------------------------
#define PPE_SCALARVECTOR_BINARY_OP(_Func, _Pred) \
    template <typename T, size_t _Dim, typename _Lhs, typename _Rhs> \
    NODISCARD CONSTEXPR auto _Func(const TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) { \
        return details::MakeScalarVectorBinaryExpr<_Pred>(lhs, rhs); \
    } \
    template <typename T, size_t _Dim, typename _Expr> \
    NODISCARD CONSTEXPR auto _Func(const TScalarVectorExpr<T, _Dim, _Expr>& lhs, const T& rhs) { \
        return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR { \
            return details::MakeScalarVector(_Pred{}(lhs.template get<idx>(), rhs)...); \
        }); \
    } \
    template <typename T, size_t _Dim, typename _Expr> \
    NODISCARD CONSTEXPR auto _Func(const T& lhs, const TScalarVectorExpr<T, _Dim, _Expr>& rhs) { \
        return Meta::static_for<_Dim>([&](auto... idx) CONSTEXPR { \
            return details::MakeScalarVector(_Pred{}(lhs, rhs.template get<idx>())...); \
        }); \
    } \
    template <typename T, size_t _Dim, typename _Lhs, typename _Rhs> \
    CONSTEXPR auto& _Func##=(TScalarVectorExpr<T, _Dim, _Lhs>& lhs, const TScalarVectorExpr<T, _Dim, _Rhs>& rhs) { \
        lhs = TScalarVector<T, _Dim>{ _Func(lhs, rhs) }; \
        return lhs; \
    } \
    template <typename T, size_t _Dim, typename _Expr> \
    CONSTEXPR auto& _Func##=(TScalarVectorExpr<T, _Dim, _Expr>& lhs, const T& rhs) { \
        lhs = TScalarVector<T, _Dim>{ _Func(lhs, rhs) }; \
        return lhs; \
    }
//----------------------------------------------------------------------------
PPE_SCALARVECTOR_BINARY_OP(operator +, Meta::TPlus<T>)
PPE_SCALARVECTOR_BINARY_OP(operator -, Meta::TMinus<T>)
PPE_SCALARVECTOR_BINARY_OP(operator *, Meta::TMultiplies<T>)
PPE_SCALARVECTOR_BINARY_OP(operator /, Meta::TDivides<T>)
PPE_SCALARVECTOR_BINARY_OP(operator &, Meta::TBitAnd<T>)
PPE_SCALARVECTOR_BINARY_OP(operator |, Meta::TBitOr<T>)
PPE_SCALARVECTOR_BINARY_OP(operator ^, Meta::TBitXor<T>)
#undef PPE_SCALARVECTOR_BINARY_OP
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// All scalar vectors are considered as pods
PPE_ASSUME_TEMPLATE_AS_POD(TScalarVector<T COMMA _Dim>, typename T, size_t _Dim)
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

#ifndef EXPORT_PPE_RUNTIME_CORE_SCALARVECTOR
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<bool, 1>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<bool, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<bool, 3>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<bool, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<int, 1>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<int, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<int, 3>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<unsigned int, 1>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<unsigned int, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<unsigned int, 3>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<unsigned int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<float, 1>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<float, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<float, 3>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<float, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<double, 1>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<double, 2>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<double, 3>;
EXTERN_TEMPLATE_STRUCT_DECL(PPE_CORE_API) TScalarVector<double, 4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace
#endif //!EXPORT_PPE_RUNTIME_CORE_SCALARVECTOR

PRAGMA_RESTORE_RUNTIMECHECKS
