#pragma once

#include "Core/Maths/Geometry/ScalarVector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
ScalarVectorBase<T, 1>::ScalarVectorBase(T x) {
    static_cast<ScalarVector<T, 1>*>(this)->_data[0] = x;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE const T& ScalarVectorBase<T, 1>::x() const {
    return static_cast<const ScalarVector<T, 1>*>(this)->_data[0];
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T& ScalarVectorBase<T, 1>::x() {
    return static_cast<ScalarVector<T, 1>*>(this)->_data[0];
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
ScalarVectorBase<T, 2>::ScalarVectorBase(T x, T y) {
    auto* const pself = static_cast<ScalarVector<T, 2>*>(this);
    pself->_data[0] = x;
    pself->_data[1] = y;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE const T& ScalarVectorBase<T, 2>::y() const {
    return static_cast<const ScalarVector<T, 2>*>(this)->_data[1];
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T& ScalarVectorBase<T, 2>::y() {
    return static_cast<ScalarVector<T, 2>*>(this)->_data[1];
}
//----------------------------------------------------------------------------
template <typename T>
template <size_t _0, size_t _1>
ScalarVector<T, 2> ScalarVectorBase<T, 2>::Shuffle2() const {
    auto* const pself = static_cast<ScalarVector<T, 2>*>(this);
    return ScalarVector<T, 2>(pself->get<_0>(), pself->get<_1>());
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
ScalarVectorBase<T, 3>::ScalarVectorBase(T x, T y, T z) {
    auto* const pself = static_cast<ScalarVector<T, 3>*>(this);
    pself->_data[0] = x;
    pself->_data[1] = y;
    pself->_data[2] = z;
}
//----------------------------------------------------------------------------
template <typename T>
ScalarVectorBase<T, 3>::ScalarVectorBase(const ScalarVector<T, 2>& xy, T z) {
    auto* const pself = static_cast<ScalarVector<T, 3>*>(this);
    pself->_data[0] = xy._data[0];
    pself->_data[1] = xy._data[1];
    pself->_data[2] = z;
}
//----------------------------------------------------------------------------
template <typename T>
ScalarVectorBase<T, 3>::ScalarVectorBase(T x, const ScalarVector<T, 2>& yz) {
    auto* const pself = static_cast<ScalarVector<T, 3>*>(this);
    pself->_data[0] = x;
    pself->_data[1] = yz._data[0];
    pself->_data[2] = yz._data[1];
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE const T& ScalarVectorBase<T, 3>::z() const {
    return static_cast<const ScalarVector<T, 3>*>(this)->_data[2];
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T& ScalarVectorBase<T, 3>::z() {
    return static_cast<ScalarVector<T, 3>*>(this)->_data[2];
}
//----------------------------------------------------------------------------
template <typename T>
template <size_t _0, size_t _1, size_t _2>
ScalarVector<T, 3> ScalarVectorBase<T, 3>::Shuffle3() const {
    auto* const pself = static_cast<const ScalarVector<T, 3>*>(this);
    return ScalarVector<T, 3>(pself->get<_0>(), pself->get<_1>(), pself->get<_2>());
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
ScalarVectorBase<T, 4>::ScalarVectorBase(T x, T y, T z, T w) {
    auto* const pself = static_cast<ScalarVector<T, 4>*>(this);
    pself->_data[0] = x;
    pself->_data[1] = y;
    pself->_data[2] = z;
    pself->_data[3] = w;
}
//----------------------------------------------------------------------------
template <typename T>
ScalarVectorBase<T, 4>::ScalarVectorBase(const ScalarVector<T, 3>& xyz, T w) {
    auto* const pself = static_cast<ScalarVector<T, 4>*>(this);
    pself->_data[0] = xyz._data[0];
    pself->_data[1] = xyz._data[1];
    pself->_data[2] = xyz._data[2];
    pself->_data[3] = w;
}
//----------------------------------------------------------------------------
template <typename T>
ScalarVectorBase<T, 4>::ScalarVectorBase(T x, const ScalarVector<T, 3>& yzw) {
    auto* const pself = static_cast<ScalarVector<T, 4>*>(this);
    pself->_data[0] = x;
    pself->_data[1] = yzw._data[0];
    pself->_data[2] = yzw._data[1];
    pself->_data[3] = yzw._data[2];
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE const T& ScalarVectorBase<T, 4>::w() const {
    return static_cast<const ScalarVector<T, 4>*>(this)->_data[3];
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T& ScalarVectorBase<T, 4>::w() {
    return static_cast<ScalarVector<T, 4>*>(this)->_data[3];
}
//----------------------------------------------------------------------------
template <typename T>
ScalarVector<T, 3> ScalarVectorBase<T, 4>::Dehomogenize() const {
    const auto* const pself = static_cast<const ScalarVector<T, 4>*>(this);
    ScalarVector<T, 3> result(Meta::noinit_tag{});
    result._data[0] = pself->_data[0] / pself->_data[3];
    result._data[1] = pself->_data[1] / pself->_data[3];
    result._data[2] = pself->_data[2] / pself->_data[3];
    return result;
}
//----------------------------------------------------------------------------
template <typename T>
template <size_t _0, size_t _1, size_t _2, size_t _3>
ScalarVector<T, 4> ScalarVectorBase<T, 4>::Shuffle4() const {
    auto* const pself = static_cast<const ScalarVector<T, 4>*>(this);
    return ScalarVector<T, 4>(pself->get<_0>(), pself->get<_1>(), pself->get<_2>(), pself->get<_3>());
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim>::ScalarVector() : ScalarVector(T(0)) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim>::ScalarVector(Meta::noinit_tag) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim>::~ScalarVector() {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim>::ScalarVector(T broadcast) {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = broadcast;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim>::ScalarVector(std::initializer_list<T> values) {
    Assert(_Dim == values.size());
    std::copy(std::begin(values), std::end(values), _data);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim>::ScalarVector(const MemoryView<const T>& data) {
    Assert(data.size() == _Dim);
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = data[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim>::ScalarVector(const ScalarVector& other) {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = other._data[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto ScalarVector<T, _Dim>::operator =(const ScalarVector& other) -> ScalarVector& {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = other._data[i];
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
ScalarVector<T, _Dim>::ScalarVector(const ScalarVector<U, _Dim>& other) {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = static_cast<T>(other._data[i]);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto ScalarVector<T, _Dim>::operator =(const ScalarVector<U, _Dim>& other) -> ScalarVector& {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = static_cast<T>(other._data[i]);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
const T& ScalarVector<T, _Dim>::operator [](size_t i) const {
    Assert(i < _Dim);
    return _data[i];
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T& ScalarVector<T, _Dim>::operator [](size_t i) {
    Assert(i < _Dim);
    return _data[i];
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_SELFSCALAR(_SelfOp) \
    template <typename T, size_t _Dim> \
    auto ScalarVector<T, _Dim>::operator _SelfOp (T scalar) -> ScalarVector& { \
        for (size_t i = 0; i < _Dim; ++i) \
            _data[i] _SelfOp scalar; \
        return *this; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_SCALAR(_Op) \
    template <typename T, size_t _Dim> \
    auto ScalarVector<T, _Dim>::operator _Op (T scalar) const -> ScalarVector { \
        ScalarVector result(Meta::noinit_tag{}); \
        for (size_t i = 0; i < _Dim; ++i) \
            result._data[i] = _data[i] _Op scalar; \
        return result; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_SELFOTHER(_SelfOp) \
    template <typename T, size_t _Dim> \
    auto ScalarVector<T, _Dim>::operator _SelfOp (const ScalarVector& other) -> ScalarVector& { \
        for (size_t i = 0; i < _Dim; ++i) \
            _data[i] _SelfOp other._data[i]; \
        return *this; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_OTHER(_Op) \
    template <typename T, size_t _Dim> \
    auto ScalarVector<T, _Dim>::operator _Op (const ScalarVector& other) const -> ScalarVector { \
        ScalarVector result(Meta::noinit_tag{}); \
        for (size_t i = 0; i < _Dim; ++i) \
            result._data[i] = _data[i] _Op other._data[i]; \
        return result; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_SELFOTHERU(_SelfOp) \
    template <typename T, size_t _Dim> \
    template <typename U> \
    auto ScalarVector<T, _Dim>::operator _SelfOp (const ScalarVector<U, _Dim>& other) -> ScalarVector& { \
        for (size_t i = 0; i < _Dim; ++i) \
            _data[i] _SelfOp static_cast<T>(other._data[i]); \
        return *this; \
    }
//----------------------------------------------------------------------------
#define DEF_SCALARVECTOR_OP_OTHERU(_Op) \
    template <typename T, size_t _Dim> \
    template <typename U> \
    auto ScalarVector<T, _Dim>::operator _Op (const ScalarVector<U, _Dim>& other) const -> ScalarVector { \
        ScalarVector result(Meta::noinit_tag{}); \
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

DEF_SCALARVECTOR_OP_LHS(+)
DEF_SCALARVECTOR_OP_LHS(-)
DEF_SCALARVECTOR_OP_LHS(*)
DEF_SCALARVECTOR_OP_LHS(/)
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
    ScalarVector<T, _Dim> operator _Op(T lhs, const ScalarVector<T, _Dim>& rhs) { \
        ScalarVector<T, _Dim> result(Meta::noinit_tag{}); \
        for (size_t i = 0; i < _Dim; ++i) \
            result[i] = lhs _Op rhs[i]; \
        return result; \
    } \
    template <typename U, typename T, size_t _Dim> \
    ScalarVector<T, _Dim> operator _Op(U lhs, const ScalarVector<T, _Dim>& rhs) { \
        ScalarVector<T, _Dim> result(Meta::noinit_tag{}); \
        for (size_t i = 0; i < _Dim; ++i) \
            result[i] = static_cast<T>(lhs _Op rhs[i]); \
        return result; \
    }
//----------------------------------------------------------------------------
DEF_SCALARVECTOR_OP_RHS(+)
DEF_SCALARVECTOR_OP_RHS(-)
DEF_SCALARVECTOR_OP_RHS(*)
DEF_SCALARVECTOR_OP_RHS(/)
//----------------------------------------------------------------------------
#undef DEF_SCALARVECTOR_OP_RHS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto ScalarVector<T, _Dim>::operator -() const -> ScalarVector {
    ScalarVector result(Meta::noinit_tag{});
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = -_data[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarVector<T, _Dim>::operator ==(const ScalarVector& other) const {
    for (size_t i = 0; i < _Dim; ++i)
        if (_data[i] != other._data[i])
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void ScalarVector<T, _Dim>::Broadcast(T scalar) {
    for (size_t i = 0; i < _Dim; ++i)
        _data[i] = scalar;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void ScalarVector<T, _Dim>::Swap(ScalarVector& other) {
    using std::swap;
    using Core::swap;
    for (size_t i = 0; i < _Dim; ++i)
        swap(other._data[i], _data[i]);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim + 1> ScalarVector<T, _Dim>::Extend(T value) const {
    ScalarVector<T, _Dim + 1> result(Meta::noinit_tag{});
    for (size_t i = 0; i < _Dim; ++i)
        result[i] = _data[i];
    result[_Dim] = value;
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarVector<T, _Dim>::AllLessThan(const ScalarVector& other) const {
    for (size_t i = 0; i < _Dim; ++i)
        if (!(_data[i] < other._data[i]))
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarVector<T, _Dim>::AllLessOrEqual(const ScalarVector& other) const {
    for (size_t i = 0; i < _Dim; ++i)
        if (!(_data[i] <= other._data[i]))
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarVector<T, _Dim>::AllGreaterThan(const ScalarVector& other) const {
    for (size_t i = 0; i < _Dim; ++i)
        if (!(_data[i] > other._data[i]))
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarVector<T, _Dim>::AllGreaterOrEqual(const ScalarVector& other) const {
    for (size_t i = 0; i < _Dim; ++i)
        if (!(_data[i] >= other._data[i]))
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
ScalarVector<U, _Dim> ScalarVector<T, _Dim>::Cast() const {
    return ScalarVector<U, _Dim>(*this);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
