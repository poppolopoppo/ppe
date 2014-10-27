#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"

#include "Core/Container/Hash.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Meta/NumericLimits.h"

#include <algorithm>
#include <iosfwd>
#include <initializer_list>
#include <limits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarVector {
public:
    template <typename U, size_t _Dim2>
    friend class ScalarVector;
    template <typename U, size_t _Width, size_t _Height>
    friend class ScalarMatrix;

    ScalarVector();
    ~ScalarVector();

    ScalarVector(T broadcast);
    template <typename U>
    ScalarVector(U broadcast) : ScalarVector(checked_cast<T>(broadcast)) {}
    ScalarVector(std::initializer_list<T> values);

    FORCE_INLINE ScalarVector(T v0, T v1);
    FORCE_INLINE ScalarVector(T v0, T v1, T v2);
    FORCE_INLINE ScalarVector(T v0, T v1, T v2, T v3);

    ScalarVector(T extend, const ScalarVector<T, _Dim - 1>& other);
    ScalarVector(const ScalarVector<T, _Dim - 1>& other, T extend);

    ScalarVector(const ScalarVector& other);
    ScalarVector& operator =(const ScalarVector& other);

    template <typename U>
    ScalarVector(const ScalarVector<U, _Dim>& other);
    template <typename U>
    ScalarVector& operator =(const ScalarVector<U, _Dim>& other);

    template <size_t _Idx>
    FORCE_INLINE const T& get() const { STATIC_ASSERT(_Idx < _Dim); return _data[_Idx]; }
    template <size_t _Idx>
    FORCE_INLINE T& get() { STATIC_ASSERT(_Idx < _Dim); return _data[_Idx]; }

    FORCE_INLINE const T& x() const { return get<0>(); };
    FORCE_INLINE const T& y() const { return get<1>(); };
    FORCE_INLINE const T& z() const { return get<2>(); };
    FORCE_INLINE const T& w() const { return get<3>(); };

    FORCE_INLINE T& x() { return get<0>(); };
    FORCE_INLINE T& y() { return get<1>(); };
    FORCE_INLINE T& z() { return get<2>(); };
    FORCE_INLINE T& w() { return get<3>(); };

    FORCE_INLINE const T& operator [](size_t i) const;
    FORCE_INLINE T& operator [](size_t i);

#define DECL_SCALARVECTOR_OP_SELF_LHS(_Op) \
    ScalarVector&   operator _Op (T scalar); \
    ScalarVector&   operator _Op (const ScalarVector& other); \
    template <typename U> \
    ScalarVector&   operator _Op (const ScalarVector<U, _Dim>& other);

#define DECL_SCALARVECTOR_OP_LHS(_Op) \
    ScalarVector    operator _Op (T scalar) const; \
    ScalarVector    operator _Op (const ScalarVector& other) const; \
    template <typename U> \
    ScalarVector    operator _Op (const ScalarVector<U, _Dim>& other) const;

    DECL_SCALARVECTOR_OP_SELF_LHS(+=)
    DECL_SCALARVECTOR_OP_SELF_LHS(-=)
    DECL_SCALARVECTOR_OP_SELF_LHS(*=)
    DECL_SCALARVECTOR_OP_SELF_LHS(/=)
    DECL_SCALARVECTOR_OP_SELF_LHS(^=)
    DECL_SCALARVECTOR_OP_SELF_LHS(%=)

    DECL_SCALARVECTOR_OP_LHS(+)
    DECL_SCALARVECTOR_OP_LHS(-)
    DECL_SCALARVECTOR_OP_LHS(*)
    DECL_SCALARVECTOR_OP_LHS(/)
    DECL_SCALARVECTOR_OP_LHS(^)
    DECL_SCALARVECTOR_OP_LHS(%)

    ScalarVector    operator -() const;

#undef DECL_SCALARVECTOR_OP_SELF_LHS
#undef DECL_SCALARVECTOR_OP_LHS

    bool operator ==(const ScalarVector& other) const;
    bool operator !=(const ScalarVector& other) const { return !operator ==(other); }

    void Broadcast(T scalar);

    void Swap(ScalarVector& other);
    FORCE_INLINE size_t HashValue() const { return hash_value(_data); }

    MemoryView<T> MakeView() { return Core::MakeView(_data); }
    MemoryView<const T> MakeView() const { return Core::MakeView(_data); }

    ScalarVector<T, _Dim - 1> Dehomogenize() const;
    ScalarVector<T, _Dim + 1> Extend(T value) const;

    FORCE_INLINE ScalarVector<T, _Dim + 1> OneExtend() const { return Extend(T(1)); }
    FORCE_INLINE ScalarVector<T, _Dim + 1> ZeroExtend() const { return Extend(T(0)); }

    bool AllLessThan(const ScalarVector& other) const;
    bool AllLessOrEqual(const ScalarVector& other) const;
    bool AllGreaterThan(const ScalarVector& other) const;
    bool AllGreaterOrEqual(const ScalarVector& other) const;

    template <typename U>
    ScalarVector<U, _Dim> Cast() const;

    static ScalarVector MinusOne() { return ScalarVector(T(-1)); }
    static ScalarVector One() { return ScalarVector(T(1)); }
    static ScalarVector Zero() { return ScalarVector(T(0)); }

    static ScalarVector MaxValue() { return ScalarVector(NumericLimits<T>::MaxValue); }
    static ScalarVector MinValue() { return ScalarVector(NumericLimits<T>::MinValue); }

    static ScalarVector Homogeneous() { ScalarVector r(0); r._data[_Dim - 1] = 1; return r; }

    static ScalarVector X() { ScalarVector r(0); r.get<0>() = 1; return r; }
    static ScalarVector Y() { ScalarVector r(0); r.get<1>() = 1; return r; }
    static ScalarVector Z() { ScalarVector r(0); r.get<2>() = 1; return r; }
    static ScalarVector W() { ScalarVector r(0); r.get<3>() = 1; return r; }

    template <size_t _0, size_t _1>
    ScalarVector<T, 2> Shuffle2() const { return ScalarVector<T, 2>(get<_0>(), get<_1>()); }
    template <size_t _0, size_t _1, size_t _2>
    ScalarVector<T, 3> Shuffle3() const { return ScalarVector<T, 3>(get<_0>(), get<_1>(), get<_2>()); }
    template <size_t _0, size_t _1, size_t _2, size_t _3>
    ScalarVector<T, 4> Shuffle4() const { return ScalarVector<T, 4>(get<_0>(), get<_1>(), get<_2>(), get<_3>()); }

public:
    STATIC_CONST_INTEGRAL(size_t, Dim, _Dim);
    STATIC_ASSERT(0 < _Dim);
    T _data[_Dim];

public:
    // All shuffle specializations :

#define DEF_SCALARVECTOR_SHUFFLE2(_Name, _0, _1) \
    ScalarVector<T, 2> _Name() const { return Shuffle2<_0, _1>(); }
#define DEF_SCALARVECTOR_SHUFFLE3(_Name, _0, _1, _2) \
    ScalarVector<T, 3> _Name() const { return Shuffle3<_0, _1, _2>(); }
#define DEF_SCALARVECTOR_SHUFFLE4(_Name, _0, _1, _2, _3) \
    ScalarVector<T, 4> _Name() const { return Shuffle4<_0, _1, _2, _3>(); }

#   include "Core/Maths/Geometry/ScalarVector.Shuffle-inl.h"

#undef DEF_SCALARVECTOR_SHUFFLE2
#undef DEF_SCALARVECTOR_SHUFFLE3
#undef DEF_SCALARVECTOR_SHUFFLE4
};
//----------------------------------------------------------------------------
#define DECL_SCALARVECTOR_OP_RHS(_Op) \
    template <typename T, size_t _Dim> \
    ScalarVector<T, _Dim> operator _Op(T lhs, const ScalarVector<T, _Dim>& rhs); \
    template <typename U, typename T, size_t _Dim> \
    ScalarVector<T, _Dim> operator _Op(U lhs, const ScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
DECL_SCALARVECTOR_OP_RHS(+)
DECL_SCALARVECTOR_OP_RHS(-)
DECL_SCALARVECTOR_OP_RHS(*)
DECL_SCALARVECTOR_OP_RHS(/)
DECL_SCALARVECTOR_OP_RHS(^)
DECL_SCALARVECTOR_OP_RHS(%)
//----------------------------------------------------------------------------
#undef DECL_SCALARVECTOR_OP_RHS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T, size_t _Dim >
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const ScalarVector<T, _Dim>& v) {
    for (const auto& s : v.MakeView())
        oss << ' ' << s << ',';
    return oss;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
size_t hash_value(const ScalarVector<T, _Dim>& v) {
    return v.HashValue();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void swap(ScalarVector<T, _Dim>& lhs, ScalarVector<T, _Dim>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct NumericLimits< ScalarVector<T, _Dim> > {
    static const ScalarVector<T, _Dim> Epsilon;
    static const ScalarVector<T, _Dim> Inf;
    static const ScalarVector<T, _Dim> MaxValue;
    static const ScalarVector<T, _Dim> MinValue;
    static const ScalarVector<T, _Dim> Nan;
    static const ScalarVector<T, _Dim> Default;
};
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::Epsilon(NumericLimits<T>::Epsilon);
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::Inf(NumericLimits<T>::Inf);
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::MaxValue(NumericLimits<T>::MaxValue);
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::MinValue(NumericLimits<T>::MinValue);
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::Nan(NumericLimits<T>::Nan);
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::Default(NumericLimits<T>::Default);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Geometry/ScalarVector-inl.h"
