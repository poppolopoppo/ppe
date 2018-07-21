#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarVector_fwd.h"

#include "Core/Container/Hash.h"
#include "Core/IO/TextWriter_fwd.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Meta/NumericLimits.h"

#include <algorithm>
#include <initializer_list>
#include <limits>
#include <type_traits>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class TScalarVector {
public:
    template <typename U, size_t _Dim2>
    friend class TScalarVector;
    template <typename U, size_t _Width, size_t _Height>
    friend class TScalarMatrix;

    FORCE_INLINE TScalarVector() {}
    explicit TScalarVector(Meta::FForceInit);

    explicit TScalarVector(T broadcast);
    template <typename U>
    explicit TScalarVector(U broadcast) : TScalarVector(checked_cast<T>(broadcast)) {}
    TScalarVector(std::initializer_list<T> values);

    FORCE_INLINE TScalarVector(T v0, T v1);
    FORCE_INLINE TScalarVector(T v0, T v1, T v2);
    FORCE_INLINE TScalarVector(T v0, T v1, T v2, T v3);

    TScalarVector(T extend, const TScalarVector<T, _Dim - 1>& other);
    TScalarVector(const TScalarVector<T, _Dim - 1>& other, T extend);

    TScalarVector(const TMemoryView<const T>& data);

    TScalarVector(const TScalarVector& other);
    TScalarVector& operator =(const TScalarVector& other);

    template <typename U>
    TScalarVector(const TScalarVector<U, _Dim>& other);
    template <typename U>
    TScalarVector& operator =(const TScalarVector<U, _Dim>& other);

    template <size_t _Idx>
    FORCE_INLINE Meta::TEnableIf<_Idx < _Dim, T&> get() { return _data[_Idx]; }
    template <size_t _Idx>
    FORCE_INLINE Meta::TEnableIf<_Idx < _Dim, T> get() const { return _data[_Idx]; }

    FORCE_INLINE T x() const { return get<0>(); };
    FORCE_INLINE T y() const { return get<1>(); };
    FORCE_INLINE T z() const { return get<2>(); };
    FORCE_INLINE T w() const { return get<3>(); };

    FORCE_INLINE T& x() { return get<0>(); };
    FORCE_INLINE T& y() { return get<1>(); };
    FORCE_INLINE T& z() { return get<2>(); };
    FORCE_INLINE T& w() { return get<3>(); };

    FORCE_INLINE T& operator [](size_t i);
    FORCE_INLINE T operator [](size_t i) const;

#define DECL_SCALARVECTOR_OP_SELF_LHS(_Op) \
    TScalarVector&   operator _Op (T scalar); \
    TScalarVector&   operator _Op (const TScalarVector& other); \
    template <typename U> \
    TScalarVector&   operator _Op (const TScalarVector<U, _Dim>& other);

#define DECL_SCALARVECTOR_OP_LHS(_Op) \
    TScalarVector    operator _Op (T scalar) const; \
    TScalarVector    operator _Op (const TScalarVector& other) const; \
    template <typename U> \
    TScalarVector    operator _Op (const TScalarVector<U, _Dim>& other) const;

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

    TScalarVector    operator -() const;

#undef DECL_SCALARVECTOR_OP_SELF_LHS
#undef DECL_SCALARVECTOR_OP_LHS

    bool operator ==(const TScalarVector& other) const;
    bool operator !=(const TScalarVector& other) const { return !operator ==(other); }

    void Broadcast(T scalar);

    void Swap(TScalarVector& other);

    TMemoryView<T> MakeView() { return Core::MakeView(_data); }
    TMemoryView<const T> MakeView() const { return Core::MakeView(_data); }

    friend hash_t hash_value(const TScalarVector& v) { return hash_as_pod_array(v._data); }

    TScalarVector<T, _Dim - 1> Dehomogenize() const;
    TScalarVector<T, _Dim + 1> Extend(T value) const;

    FORCE_INLINE TScalarVector<T, _Dim + 1> OneExtend() const { return Extend(T(1)); }
    FORCE_INLINE TScalarVector<T, _Dim + 1> ZeroExtend() const { return Extend(T(0)); }

    bool AllLessThan(const TScalarVector& other) const;
    bool AllLessOrEqual(const TScalarVector& other) const;
    bool AllGreaterThan(const TScalarVector& other) const;
    bool AllGreaterOrEqual(const TScalarVector& other) const;

    template <typename U>
    TScalarVector<U, _Dim> Cast() const;

    static TScalarVector MinusOne() { return TScalarVector(T(-1)); }
    static TScalarVector One() { return TScalarVector(T(1)); }
    static TScalarVector Zero() { return TScalarVector(T(0)); }

    static TScalarVector MaxValue() { return TScalarVector(TNumericLimits<T>::MaxValue()); }
    static TScalarVector MinValue() { return TScalarVector(TNumericLimits<T>::MinValue()); }

    static TScalarVector Homogeneous() { TScalarVector r(0); r._data[_Dim - 1] = 1; return r; }

    static TScalarVector X() { TScalarVector r(0); r.get<0>() = 1; return r; }
    static TScalarVector Y() { TScalarVector r(0); r.get<1>() = 1; return r; }
    static TScalarVector Z() { TScalarVector r(0); r.get<2>() = 1; return r; }
    static TScalarVector W() { TScalarVector r(0); r.get<3>() = 1; return r; }

    static TScalarVector Left() { STATIC_ASSERT(3 == _Dim); return TScalarVector(-1,0,0); }
    static TScalarVector Right() { STATIC_ASSERT(3 == _Dim); return TScalarVector(1,0,0); }

    static TScalarVector Up() { STATIC_ASSERT(3 == _Dim); return TScalarVector(0,1,0); }
    static TScalarVector Down() { STATIC_ASSERT(3 == _Dim); return TScalarVector(0,-1,0); }

    static TScalarVector Forward() { STATIC_ASSERT(3 == _Dim); return TScalarVector(0,0,1); }
    static TScalarVector Backward() { STATIC_ASSERT(3 == _Dim); return TScalarVector(0,0,-1); }

    template <size_t _Offset, size_t _N>
    Meta::TEnableIf<_Offset + _N <= _Dim, TScalarVector<T, _N>&> Shuffle0() { return *reinterpret_cast< TScalarVector<T, _N>* >(&_data[_Offset]); }
    template <size_t _Offset, size_t _N>
    Meta::TEnableIf<_Offset + _N <= _Dim, const TScalarVector<T,_N>&> Shuffle0() const { return *reinterpret_cast< const TScalarVector<T,_N>* >(&_data[_Offset]); }
    template <size_t _0, size_t _1>
    Meta::TEnableIf<_0 < _Dim && _1 < _Dim, TScalarVector<T, 2>> Shuffle2() const { return TScalarVector<T, 2>(get<_0>(), get<_1>()); }
    template <size_t _0, size_t _1, size_t _2>
    Meta::TEnableIf<_0 < _Dim && _1 < _Dim && _2 < _Dim, TScalarVector<T, 3>> Shuffle3() const { return TScalarVector<T, 3>(get<_0>(), get<_1>(), get<_2>()); }
    template <size_t _0, size_t _1, size_t _2, size_t _3>
    Meta::TEnableIf<_0 < _Dim && _1 < _Dim && _2 < _Dim && _3 < _Dim, TScalarVector<T, 4>> Shuffle4() const { return TScalarVector<T, 4>(get<_0>(), get<_1>(), get<_2>(), get<_3>()); }

public:
    STATIC_CONST_INTEGRAL(size_t, Dim, _Dim);
    STATIC_ASSERT(0 < _Dim);
    T _data[_Dim];

public:
    // All shuffle specializations :

#define DEF_SCALARVECTOR_SHUFFLE0(_Name, _Offset, _N) \
    FORCE_INLINE TScalarVector<T,_N>& _Name() { return Shuffle0<_Offset,_N>(); } \
    FORCE_INLINE const TScalarVector<T,_N>& _Name() const { return Shuffle0<_Offset,_N>(); }
#define DEF_SCALARVECTOR_SHUFFLE2(_Name, _0, _1) \
    FORCE_INLINE TScalarVector<T, 2> _Name() const { return Shuffle2<_0, _1>(); }
#define DEF_SCALARVECTOR_SHUFFLE3(_Name, _0, _1, _2) \
    FORCE_INLINE TScalarVector<T, 3> _Name() const { return Shuffle3<_0, _1, _2>(); }
#define DEF_SCALARVECTOR_SHUFFLE4(_Name, _0, _1, _2, _3) \
    FORCE_INLINE TScalarVector<T, 4> _Name() const { return Shuffle4<_0, _1, _2, _3>(); }

#   include "Core/Maths/ScalarVector.Shuffle-inl.h"

#undef DEF_SCALARVECTOR_SHUFFLE0
#undef DEF_SCALARVECTOR_SHUFFLE2
#undef DEF_SCALARVECTOR_SHUFFLE3
#undef DEF_SCALARVECTOR_SHUFFLE4
};
//----------------------------------------------------------------------------
#define DECL_SCALARVECTOR_OP_RHS(_Op) \
    template <typename T, size_t _Dim> \
    TScalarVector<T, _Dim> operator _Op(T lhs, const TScalarVector<T, _Dim>& rhs); \
    template <typename U, typename T, size_t _Dim> \
    TScalarVector<T, _Dim> operator _Op(U lhs, const TScalarVector<T, _Dim>& rhs);
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
template <typename T, size_t _Dim>
void swap(TScalarVector<T, _Dim>& lhs, TScalarVector<T, _Dim>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim >
FTextWriter& operator <<(FTextWriter& oss, const TScalarVector<T, _Dim>& v) {
    oss << '[' << v._data[0];
    forrange(i, 1, _Dim)
        oss << ", " << v._data[i];
    oss << ']';
    return oss;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim >
FWTextWriter& operator <<(FWTextWriter& oss, const TScalarVector<T, _Dim>& v) {
    oss << L'[' << v._data[0];
    forrange(i, 1, _Dim)
        oss << L", " << v._data[i];
    oss << L']';
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

    static constexpr value_type DefaultValue() { return value_type( scalar_type::DefaultValue() ); }
    static constexpr value_type Epsilon() { return value_type( scalar_type::Epsilon() ); }
    static constexpr value_type Inf() { return value_type( scalar_type::Inf() ); }
    static constexpr value_type MaxValue() { return value_type( scalar_type::MaxValue() ); }
    static constexpr value_type MinValue() { return value_type( scalar_type::MinValue() ); }
    static constexpr value_type Nan() { return value_type( scalar_type::Nan() ); }
    static constexpr value_type Zero() { return value_type( scalar_type::Zero() ); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// All scalar vectors are considered as pods
//----------------------------------------------------------------------------
CORE_ASSUME_TYPE_AS_POD(TScalarVector<T COMMA _Dim>, typename T, size_t _Dim)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/ScalarVector-inl.h"
