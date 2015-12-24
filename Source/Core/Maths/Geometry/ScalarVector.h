#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"
#include "Core/Maths/Geometry/ScalarVector.Shuffle-inl.h"

#include "Core/Container/Hash.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Meta/NumericLimits.h"

#include <algorithm>
#include <iosfwd>
#include <initializer_list>
#include <limits>
#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Dim>
class ScalarVectorBase {};
template <typename T>
class ScalarVectorBase<T, 1> {
public:
    ScalarVectorBase() = default;
    ScalarVectorBase(T x);
    const T& x() const;
    T& x();
};
template <typename T>
class ScalarVectorBase<T, 2> : public ScalarVectorBase<T, 1> {
public:
    ScalarVectorBase() = default;
    ScalarVectorBase(T x, T y);
    const T& y() const;
    T& y();
    template <size_t _0, size_t _1>
    ScalarVector<T, 2> Shuffle2() const;
#define DECL_SCALARVECTOR_SHUFFLE2(_Name, _0, _1, _Unused) \
    ScalarVector<T, 2> _Name() const { return Shuffle2<_0, _1>(); }
    FOREACH_CORE_SCALARVECTOR_SHUFFLE2(DECL_SCALARVECTOR_SHUFFLE2)
#undef DECL_SCALARVECTOR_SHUFFLE2
};
template <typename T>
class ScalarVectorBase<T, 3> : public ScalarVectorBase<T, 2> {
public:
    ScalarVectorBase() = default;
    ScalarVectorBase(T x, T y, T z);
    ScalarVectorBase(const ScalarVector<T, 2>& xy, T z);
    ScalarVectorBase(T x, const ScalarVector<T, 2>& yz);
    const T& z() const;
    T& z();
    template <size_t _0, size_t _1, size_t _2>
    ScalarVector<T, 3> Shuffle3() const;
#define DECL_SCALARVECTOR_SHUFFLE3(_Name, _0, _1, _2, _Unused) \
    ScalarVector<T, 3> _Name() const { return Shuffle3<_0, _1, _2>(); }
    FOREACH_CORE_SCALARVECTOR_SHUFFLE3(DECL_SCALARVECTOR_SHUFFLE3)
#undef DECL_SCALARVECTOR_SHUFFLE3
};
template <typename T>
class ScalarVectorBase<T, 4> : public ScalarVectorBase<T, 3> {
public:
    ScalarVectorBase() = default;
    ScalarVectorBase(T x, T y, T z, T w);
    ScalarVectorBase(const ScalarVector<T, 3>& xyz, T w);
    ScalarVectorBase(T x, const ScalarVector<T, 3>& yzw);
    const T& w() const;
    T& w();
    ScalarVector<T, 3> Dehomogenize() const;
    template <size_t _0, size_t _1, size_t _2, size_t _3>
    ScalarVector<T, 4> Shuffle4() const;
#define DECL_SCALARVECTOR_SHUFFLE4(_Name, _0, _1, _2, _3, _Unused) \
    ScalarVector<T, 4> _Name() const { return Shuffle4<_0, _1, _2, _3>(); }
    FOREACH_CORE_SCALARVECTOR_SHUFFLE4(DECL_SCALARVECTOR_SHUFFLE4)
#undef DECL_SCALARVECTOR_SHUFFLE4
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarVectorConstants {};
template <typename T>
class ScalarVectorConstants<T, 2> {
public:
    static ScalarVector<T, 2> UnitX()   { return ScalarVector<T, 2>(1,0); }
    static ScalarVector<T, 2> UnitY()   { return ScalarVector<T, 2>(0,1); }
    static ScalarVector<T, 2> Left()    { return ScalarVector<T, 2>(-1,0); }
    static ScalarVector<T, 2> Right()   { return ScalarVector<T, 2>(1,0); }
    static ScalarVector<T, 2> Up()      { return ScalarVector<T, 2>(0,1); }
    static ScalarVector<T, 2> Down()    { return ScalarVector<T, 2>(0,-1); }
};
template <typename T>
class ScalarVectorConstants<T, 3> {
public:
    static ScalarVector<T, 3> UnitX()   { return ScalarVector<T, 3>(1,0,0); }
    static ScalarVector<T, 3> UnitY()   { return ScalarVector<T, 3>(0,1,0); }
    static ScalarVector<T, 3> UnitZ()   { return ScalarVector<T, 3>(0,0,1); }
    static ScalarVector<T, 3> Left()    { return ScalarVector<T, 3>(-1,0,0); }
    static ScalarVector<T, 3> Right()   { return ScalarVector<T, 3>(1,0,0); }
    static ScalarVector<T, 3> Up()      { return ScalarVector<T, 3>(0,1,0); }
    static ScalarVector<T, 3> Down()    { return ScalarVector<T, 3>(0,-1,0); }
    static ScalarVector<T, 3> Forward() { return ScalarVector<T, 3>(0,0,1); }
    static ScalarVector<T, 3> Backward(){ return ScalarVector<T, 3>(0,0,-1); }
};
template <typename T>
class ScalarVectorConstants<T, 4> {
public:
    static ScalarVector<T, 4> UnitX()   { return ScalarVector<T, 4>(1,0,0,0); }
    static ScalarVector<T, 4> UnitY()   { return ScalarVector<T, 4>(0,1,0,0); }
    static ScalarVector<T, 4> UnitZ()   { return ScalarVector<T, 4>(0,0,1,0); }
    static ScalarVector<T, 4> UnitW()   { return ScalarVector<T, 4>(0,0,0,1); }
    static ScalarVector<T, 4> Left()    { return ScalarVector<T, 4>(-1,0,0,0); }
    static ScalarVector<T, 4> Right()   { return ScalarVector<T, 4>(1,0,0,0); }
    static ScalarVector<T, 4> Up()      { return ScalarVector<T, 4>(0,1,0,0); }
    static ScalarVector<T, 4> Down()    { return ScalarVector<T, 4>(0,-1,0,0); }
    static ScalarVector<T, 4> Forward() { return ScalarVector<T, 4>(0,0,1,0); }
    static ScalarVector<T, 4> Backward(){ return ScalarVector<T, 4>(0,0,-1,0); }
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarVector
:   public details::ScalarVectorBase<T, _Dim>
,   public details::ScalarVectorConstants<T, _Dim> {
public:
    template <typename U, size_t _Dim2>
    friend class ScalarVectorBase;
    template <typename U, size_t _Dim2>
    friend class ScalarVector;
    template <typename U, size_t _Width, size_t _Height>
    friend class ScalarMatrix;

    using details::ScalarVectorBase<T, _Dim>::ScalarVectorBase;

    ScalarVector();
    explicit ScalarVector(Meta::noinit_tag);
    ~ScalarVector();

    ScalarVector(T broadcast);
    template <typename U>
    ScalarVector(U broadcast) : ScalarVector(checked_cast<T>(broadcast)) {}
    ScalarVector(std::initializer_list<T> values);

    ScalarVector(const MemoryView<const T>& data);

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

    DECL_SCALARVECTOR_OP_LHS(+)
    DECL_SCALARVECTOR_OP_LHS(-)
    DECL_SCALARVECTOR_OP_LHS(*)
    DECL_SCALARVECTOR_OP_LHS(/)

    ScalarVector operator -() const;

#undef DECL_SCALARVECTOR_OP_SELF_LHS
#undef DECL_SCALARVECTOR_OP_LHS

    bool operator ==(const ScalarVector& other) const;
    bool operator !=(const ScalarVector& other) const { return !operator ==(other); }

    void Broadcast(T scalar);

    void Swap(ScalarVector& other);

    MemoryView<T> MakeView() { return Core::MakeView(_data); }
    MemoryView<const T> MakeView() const { return Core::MakeView(_data); }

    friend hash_t hash_value(const ScalarVector& v) { return hash_as_pod(v._data); }

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

public:
    STATIC_CONST_INTEGRAL(size_t, Dim, _Dim);
    STATIC_ASSERT(0 < _Dim);
    T _data[_Dim];
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
    static const ScalarVector<T, _Dim> Zero;
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::Epsilon( NumericLimits<T>::Epsilon );
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::Inf( NumericLimits<T>::Inf );
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::MaxValue( NumericLimits<T>::MaxValue );
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::MinValue( NumericLimits<T>::MinValue );
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::Nan( NumericLimits<T>::Nan );
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::Default( NumericLimits<T>::Default );
template <typename T, size_t _Dim>
const ScalarVector<T, _Dim> NumericLimits< ScalarVector<T, _Dim> >::Zero( NumericLimits<T>::Zero );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Geometry/ScalarVector-inl.h"

#undef FOREACH_CORE_SCALARVECTOR_SHUFFLE2
#undef FOREACH_CORE_SCALARVECTOR_SHUFFLE3
#undef FOREACH_CORE_SCALARVECTOR_SHUFFLE4
