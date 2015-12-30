#pragma once

#include "Core/Maths/Geometry/ScalarVector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
ScalarVectorAccessor<T, 1, _Impl>::ScalarVectorAccessor(T x) {
    static_cast<_Impl*>(this)->_data[0] = x;
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
FORCE_INLINE const T& ScalarVectorAccessor<T, 1, _Impl>::x() const {
    return static_cast<const _Impl*>(this)->_data[0];
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
FORCE_INLINE T& ScalarVectorAccessor<T, 1, _Impl>::x() {
    return static_cast<_Impl*>(this)->_data[0];
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
template <size_t _0, size_t _1>
ScalarVector<T, 2> ScalarVectorAccessor<T, 1, _Impl>::Shuffle2() const {
    const auto* const pself = static_cast<const _Impl*>(this);
    return ScalarVector<T, 2>(pself->get<_0>(), pself->get<_1>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
template <size_t _0, size_t _1, size_t _2>
ScalarVector<T, 3> ScalarVectorAccessor<T, 1, _Impl>::Shuffle3() const {
    const auto* const pself = static_cast<const _Impl*>(this);
    return ScalarVector<T, 3>(pself->get<_0>(), pself->get<_1>(), pself->get<_2>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
template <size_t _0, size_t _1, size_t _2, size_t _3>
ScalarVector<T, 4> ScalarVectorAccessor<T, 1, _Impl>::Shuffle4() const {
    const auto* const pself = static_cast<const _Impl*>(this);
    return ScalarVector<T, 4>(pself->get<_0>(), pself->get<_1>(), pself->get<_2>(), pself->get<_3>());
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
ScalarVectorAccessor<T, 2, _Impl>::ScalarVectorAccessor(T x, T y) {
    auto* const pself = static_cast<_Impl*>(this);
    pself->_data[0] = x;
    pself->_data[1] = y;
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
FORCE_INLINE const T& ScalarVectorAccessor<T, 2, _Impl>::y() const {
    return static_cast<const _Impl*>(this)->_data[1];
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
FORCE_INLINE T& ScalarVectorAccessor<T, 2, _Impl>::y() {
    return static_cast<_Impl*>(this)->_data[1];
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
ScalarVectorAccessor<T, 3, _Impl>::ScalarVectorAccessor(T x, T y, T z) {
    auto* const pself = static_cast<_Impl*>(this);
    pself->_data[0] = x;
    pself->_data[1] = y;
    pself->_data[2] = z;
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
ScalarVectorAccessor<T, 3, _Impl>::ScalarVectorAccessor(const ScalarVector<T, 2>& xy, T z) {
    auto* const pself = static_cast<_Impl*>(this);
    pself->_data[0] = xy._data[0];
    pself->_data[1] = xy._data[1];
    pself->_data[2] = z;
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
ScalarVectorAccessor<T, 3, _Impl>::ScalarVectorAccessor(T x, const ScalarVector<T, 2>& yz) {
    auto* const pself = static_cast<_Impl*>(this);
    pself->_data[0] = x;
    pself->_data[1] = yz._data[0];
    pself->_data[2] = yz._data[1];
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
FORCE_INLINE const T& ScalarVectorAccessor<T, 3, _Impl>::z() const {
    return static_cast<const _Impl*>(this)->_data[2];
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
FORCE_INLINE T& ScalarVectorAccessor<T, 3, _Impl>::z() {
    return static_cast<_Impl*>(this)->_data[2];
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
ScalarVectorAccessor<T, 4, _Impl>::ScalarVectorAccessor(T x, T y, T z, T w) {
    auto* const pself = static_cast<_Impl*>(this);
    pself->_data[0] = x;
    pself->_data[1] = y;
    pself->_data[2] = z;
    pself->_data[3] = w;
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
ScalarVectorAccessor<T, 4, _Impl>::ScalarVectorAccessor(const ScalarVector<T, 3>& xyz, T w) {
    auto* const pself = static_cast<_Impl*>(this);
    pself->_data[0] = xyz._data[0];
    pself->_data[1] = xyz._data[1];
    pself->_data[2] = xyz._data[2];
    pself->_data[3] = w;
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
ScalarVectorAccessor<T, 4, _Impl>::ScalarVectorAccessor(T x, const ScalarVector<T, 3>& yzw) {
    auto* const pself = static_cast<_Impl*>(this);
    pself->_data[0] = x;
    pself->_data[1] = yzw._data[0];
    pself->_data[2] = yzw._data[1];
    pself->_data[3] = yzw._data[2];
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
FORCE_INLINE const T& ScalarVectorAccessor<T, 4, _Impl>::w() const {
    return static_cast<const _Impl*>(this)->_data[3];
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
FORCE_INLINE T& ScalarVectorAccessor<T, 4, _Impl>::w() {
    return static_cast<_Impl*>(this)->_data[3];
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
ScalarVector<T, 3> ScalarVectorAccessor<T, 4, _Impl>::Dehomogenize() const {
    const auto* const pself = static_cast<const _Impl*>(this);
    ScalarVector<T, 3> result(Meta::noinit_tag{});
    result._data[0] = pself->_data[0] / pself->_data[3];
    result._data[1] = pself->_data[1] / pself->_data[3];
    result._data[2] = pself->_data[2] / pself->_data[3];
    return result;
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim>::ScalarVector() : ScalarVector(T(0)) {
    STATIC_ASSERT(sizeof(_data) == _Dim*sizeof(T));
    STATIC_ASSERT(sizeof(*this) == _Dim*sizeof(T));
}
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
            result._data[i] = lhs _Op rhs[i]; \
        return result; \
    } \
    template <typename U, typename T, size_t _Dim> \
    ScalarVector<T, _Dim> operator _Op(U lhs, const ScalarVector<T, _Dim>& rhs) { \
        ScalarVector<T, _Dim> result(Meta::noinit_tag{}); \
        for (size_t i = 0; i < _Dim; ++i) \
            result._data[i] = static_cast<T>(lhs _Op rhs[i]); \
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
#pragma warning(push)
#pragma warning(disable: 4146) // unary minus operator applied to unsigned type, result still unsigned
template <typename T, size_t _Dim>
auto ScalarVector<T, _Dim>::operator -() const -> ScalarVector {
    ScalarVector result(Meta::noinit_tag{});
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = T(-_data[i]);
    return result;
}
#pragma warning(pop)
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
        result._data[i] = _data[i];
    result._data[_Dim] = value;
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
