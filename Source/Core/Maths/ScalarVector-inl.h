#pragma once

#include "Core/Maths/ScalarVector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector() : TScalarVector(TNumericLimits<T>::DefaultValue()) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector(Meta::noinit_tag) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::~TScalarVector() {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector(T broadcast) {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = broadcast;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector(std::initializer_list<T> values) {
    Assert(_Dim == values.size());
    std::copy(std::begin(values), std::end(values), _data);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector(T v0, T v1) {
    STATIC_ASSERT(2 == _Dim);
    _data[0] = v0;
    _data[1] = v1;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector(T v0, T v1, T v2) {
    STATIC_ASSERT(3 == _Dim);
    _data[0] = v0;
    _data[1] = v1;
    _data[2] = v2;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector(T v0, T v1, T v2, T v3) {
    STATIC_ASSERT(4 == _Dim);
    _data[0] = v0;
    _data[1] = v1;
    _data[2] = v2;
    _data[3] = v3;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector(T extend, const TScalarVector<T, _Dim - 1>& other) {
    STATIC_ASSERT(1 < _Dim);
    _data[0] = extend;
    for (size_t i = 1; i < _Dim; ++i)
        _data[i] = other[i - 1];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector(const TScalarVector<T, _Dim - 1>& other, T extend) {
    STATIC_ASSERT(1 < _Dim);
    for (size_t i = 0; i < _Dim - 1; ++i)
        _data[i] = other[i];
    _data[_Dim - 1] = extend;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector(const TMemoryView<const T>& data) {
    Assert(data.size() == _Dim);
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = data[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim>::TScalarVector(const TScalarVector& other) {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = other._data[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto TScalarVector<T, _Dim>::operator =(const TScalarVector& other) -> TScalarVector& {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = other._data[i];
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
TScalarVector<T, _Dim>::TScalarVector(const TScalarVector<U, _Dim>& other) {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = static_cast<T>(other._data[i]);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto TScalarVector<T, _Dim>::operator =(const TScalarVector<U, _Dim>& other) -> TScalarVector& {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = static_cast<T>(other._data[i]);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
const T& TScalarVector<T, _Dim>::operator [](size_t i) const {
    Assert(i < _Dim);
    return _data[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T& TScalarVector<T, _Dim>::operator [](size_t i) {
    Assert(i < _Dim);
    return _data[i];
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_SELFSCALAR(_SelfOp) \
    template <typename T, size_t _Dim> \
    auto TScalarVector<T, _Dim>::operator _SelfOp (T scalar) -> TScalarVector& { \
        for (size_t i = 0; i < _Dim; ++i) \
            _data[i] _SelfOp scalar; \
        return *this; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_SCALAR(_Op) \
    template <typename T, size_t _Dim> \
    auto TScalarVector<T, _Dim>::operator _Op (T scalar) const -> TScalarVector { \
        TScalarVector result; \
        for (size_t i = 0; i < _Dim; ++i) \
            result._data[i] = _data[i] _Op scalar; \
        return result; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_SELFOTHER(_SelfOp) \
    template <typename T, size_t _Dim> \
    auto TScalarVector<T, _Dim>::operator _SelfOp (const TScalarVector& other) -> TScalarVector& { \
        for (size_t i = 0; i < _Dim; ++i) \
            _data[i] _SelfOp other._data[i]; \
        return *this; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_OTHER(_Op) \
    template <typename T, size_t _Dim> \
    auto TScalarVector<T, _Dim>::operator _Op (const TScalarVector& other) const -> TScalarVector { \
        TScalarVector result; \
        for (size_t i = 0; i < _Dim; ++i) \
            result._data[i] = _data[i] _Op other._data[i]; \
        return result; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_SELFOTHERU(_SelfOp) \
    template <typename T, size_t _Dim> \
    template <typename U> \
    auto TScalarVector<T, _Dim>::operator _SelfOp (const TScalarVector<U, _Dim>& other) -> TScalarVector& { \
        for (size_t i = 0; i < _Dim; ++i) \
            _data[i] _SelfOp static_cast<T>(other._data[i]); \
        return *this; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_OTHERU(_Op) \
    template <typename T, size_t _Dim> \
    template <typename U> \
    auto TScalarVector<T, _Dim>::operator _Op (const TScalarVector<U, _Dim>& other) const -> TScalarVector { \
        TScalarVector result; \
        for (size_t i = 0; i < _Dim; ++i) \
            result._data[i] = _data[i] _Op static_cast<T>(other._data[i]); \
        return result; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_SELF_LHS(_SelfOp) \
    DEF_SCALARVECTOR_OP_SELFSCALAR(_SelfOp) \
    DEF_SCALARVECTOR_OP_SELFOTHER(_SelfOp) \
    DEF_SCALARVECTOR_OP_SELFOTHERU(_SelfOp)
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_LHS(_Op) \
    DEF_SCALARVECTOR_OP_SCALAR(_Op) \
    DEF_SCALARVECTOR_OP_OTHER(_Op) \
    DEF_SCALARVECTOR_OP_OTHERU(_Op)
//----------------------------------------------------------------------------
DEF_SCALARVECTOR_OP_SELF_LHS(+=)
DEF_SCALARVECTOR_OP_SELF_LHS(-=)
DEF_SCALARVECTOR_OP_SELF_LHS(*=)
DEF_SCALARVECTOR_OP_SELF_LHS(/=)
DEF_SCALARVECTOR_OP_SELF_LHS(^=)
DEF_SCALARVECTOR_OP_SELF_LHS(%=)

DEF_SCALARVECTOR_OP_LHS(+)
DEF_SCALARVECTOR_OP_LHS(-)
DEF_SCALARVECTOR_OP_LHS(*)
DEF_SCALARVECTOR_OP_LHS(/)
DEF_SCALARVECTOR_OP_LHS(^)
DEF_SCALARVECTOR_OP_LHS(%)
//----------------------------------------------------------------------------
#undef DEF_SCALARVECTOR_OP_LHS
#undef DEF_SCALARVECTOR_OP_SELF_LHS
#undef DEF_SCALARVECTOR_OP_SELFSCALAR
#undef DEF_SCALARVECTOR_OP_SCALAR
#undef DEF_SCALARVECTOR_OP_SELFOTHER
#undef DEF_SCALARVECTOR_OP_OTHER
#undef DEF_SCALARVECTOR_OP_SELFOTHERU
#undef DEF_SCALARVECTOR_OP_OTHERU
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_RHS(_Op) \
    template <typename T, size_t _Dim> \
    TScalarVector<T, _Dim> operator _Op(T lhs, const TScalarVector<T, _Dim>& rhs) { \
        TScalarVector<T, _Dim> result; \
        for (size_t i = 0; i < _Dim; ++i) \
            result[i] = lhs _Op rhs[i]; \
        return result; \
    } \
    template <typename U, typename T, size_t _Dim> \
    TScalarVector<T, _Dim> operator _Op(U lhs, const TScalarVector<T, _Dim>& rhs) { \
        TScalarVector<T, _Dim> result; \
        for (size_t i = 0; i < _Dim; ++i) \
            result[i] = static_cast<T>(lhs _Op rhs[i]); \
        return result; \
    }
//----------------------------------------------------------------------------
DEF_SCALARVECTOR_OP_RHS(+)
DEF_SCALARVECTOR_OP_RHS(-)
DEF_SCALARVECTOR_OP_RHS(*)
DEF_SCALARVECTOR_OP_RHS(/)
DEF_SCALARVECTOR_OP_RHS(^)
DEF_SCALARVECTOR_OP_RHS(%)
//----------------------------------------------------------------------------
#undef DEF_SCALARVECTOR_OP_RHS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto TScalarVector<T, _Dim>::operator -() const -> TScalarVector {
    TScalarVector result;
    for (size_t i = 0; i < _Dim; ++i)
        result[i] = -_data[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarVector<T, _Dim>::operator ==(const TScalarVector& other) const {
    for (size_t i = 0; i < _Dim; ++i)
        if (_data[i] != other._data[i])
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void TScalarVector<T, _Dim>::Broadcast(T scalar) {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = scalar;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void TScalarVector<T, _Dim>::Swap(TScalarVector& other) {
    using std::swap;
    using Core::swap;
    for (size_t i = 0; i < _Dim; ++i)
        swap(other._data[i], _data[i]);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim - 1> TScalarVector<T, _Dim>::Dehomogenize() const {
    STATIC_ASSERT(1 < _Dim);
    TScalarVector<T, _Dim - 1> result;
    for (size_t i = 0; i < _Dim - 1; ++i)
        result[i] = _data[i] / _data[_Dim - 1];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim + 1> TScalarVector<T, _Dim>::Extend(T value) const {
    TScalarVector<T, _Dim + 1> result;
    for (size_t i = 0; i < _Dim; ++i)
        result[i] = _data[i];
    result[_Dim] = value;
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarVector<T, _Dim>::AllLessThan(const TScalarVector& other) const {
    for (size_t i = 0; i < _Dim; ++i)
        if (!(_data[i] < other._data[i]))
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarVector<T, _Dim>::AllLessOrEqual(const TScalarVector& other) const {
    for (size_t i = 0; i < _Dim; ++i)
        if (!(_data[i] <= other._data[i]))
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarVector<T, _Dim>::AllGreaterThan(const TScalarVector& other) const {
    for (size_t i = 0; i < _Dim; ++i)
        if (!(_data[i] > other._data[i]))
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarVector<T, _Dim>::AllGreaterOrEqual(const TScalarVector& other) const {
    for (size_t i = 0; i < _Dim; ++i)
        if (!(_data[i] >= other._data[i]))
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
TScalarVector<U, _Dim> TScalarVector<T, _Dim>::Cast() const {
    return TScalarVector<U, _Dim>(*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
