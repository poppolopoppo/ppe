#pragma once

#include "Core/Maths/Geometry/ScalarBoundingBox.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
template <size_t _0, size_t _1>
ScalarBoundingBox<T, 2> ScalarBoundingBoxBase<T, 1, _Impl>::Shuffle2() const {
    const auto* const pself = static_cast<const _Impl*>(this);
    return ScalarBoundingBox<T, 2>(pself->_min.Shuffle2<_0, _1>(), pself->_max.Shuffle2<_0, _1>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
template <size_t _0, size_t _1, size_t _2>
ScalarBoundingBox<T, 3> ScalarBoundingBoxBase<T, 1, _Impl>::Shuffle3() const {
    const auto* const pself = static_cast<const _Impl*>(this);
    return ScalarBoundingBox<T, 3>(pself->_min.Shuffle3<_0, _1, _2>(), pself->_max.Shuffle3<_0, _1, _2>());
}
//----------------------------------------------------------------------------
template <typename T, typename _Impl>
template <size_t _0, size_t _1, size_t _2, size_t _3>
ScalarBoundingBox<T, 4> ScalarBoundingBoxBase<T, 1, _Impl>::Shuffle4() const {
    const auto* const pself = static_cast<const _Impl*>(this);
    return ScalarBoundingBox<T, 4>(pself->_min.Shuffle4<_0, _1, _2, _3>(), pself->_max.Shuffle4<_0, _1, _2, _3>());
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
void ScalarBoundingBoxCorners<T, 1>::GetCorners(ScalarVector<T, 1> (&points)[2]) const {
    const auto* const pself = static_cast<const ScalarBoundingBox<T, 1>*>(this);

    points[0].x() = pself->_min.x();
    points[1].x() = pself->_max.x();
}
template <typename T>
void ScalarBoundingBoxCorners<T, 2>::GetCorners(ScalarVector<T, 2> (&points)[4]) const {
    const auto* const pself = static_cast<const ScalarBoundingBox<T, 2>*>(this);

    points[0].x() = pself->_min.x();
    points[0].y() = pself->_min.y();

    points[1].x() = pself->_max.x();
    points[1].y() = pself->_min.y();

    points[2].x() = pself->_max.x();
    points[2].y() = pself->_max.y();

    points[3].x() = pself->_min.x();
    points[3].y() = pself->_max.y();
}
template <typename T>
void ScalarBoundingBoxCorners<T, 3>::GetCorners(ScalarVector<T, 3> (&points)[8]) const {
    const auto* const pself = static_cast<const ScalarBoundingBox<T, 3>*>(this);

    points[0].x() = pself->_min.x();
    points[0].y() = pself->_min.y();
    points[0].z() = pself->_min.z();

    points[1].x() = pself->_min.x();
    points[1].y() = pself->_min.y();
    points[1].z() = pself->_max.z();

    points[2].x() = pself->_min.x();
    points[2].y() = pself->_max.y();
    points[2].z() = pself->_max.z();

    points[3].x() = pself->_min.x();
    points[3].y() = pself->_max.y();
    points[3].z() = pself->_min.z();

    points[4].x() = pself->_max.x();
    points[4].y() = pself->_min.y();
    points[4].z() = pself->_max.z();

    points[5].x() = pself->_max.x();
    points[5].y() = pself->_max.y();
    points[5].z() = pself->_max.z();

    points[6].x() = pself->_max.x();
    points[6].y() = pself->_max.y();
    points[6].z() = pself->_min.z();

    points[7].x() = pself->_max.x();
    points[7].y() = pself->_max.y();
    points[7].z() = pself->_max.z();
}
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarBoundingBox<T, _Dim>::ScalarBoundingBox()
:   ScalarBoundingBox(ScalarBoundingBox::DefaultValue()) {
    STATIC_ASSERT(sizeof(*this) == _Dim*sizeof(T)*2);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarBoundingBox<T, _Dim>::ScalarBoundingBox(Meta::noinit_tag) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarBoundingBox<T, _Dim>::ScalarBoundingBox(const vector_type& min, const vector_type& max)
:   _min(min), _max(max) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarBoundingBox<T, _Dim>::~ScalarBoundingBox() {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarBoundingBox<T, _Dim>::ScalarBoundingBox(const ScalarBoundingBox& other)
:   _min(other._min), _max(other._max) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto ScalarBoundingBox<T, _Dim>::operator =(const ScalarBoundingBox& other) -> ScalarBoundingBox& {
    _min = other._min;
    _max = other._max;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
ScalarBoundingBox<T, _Dim>::ScalarBoundingBox(const ScalarBoundingBox<U, _Dim>& other)
:   _min(other._min), _max(other._max) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto ScalarBoundingBox<T, _Dim>::operator =(const ScalarBoundingBox<U, _Dim>& other) -> ScalarBoundingBox& {
    _min = other._min;
    _max = other._max;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto ScalarBoundingBox<T, _Dim>::Center() const -> vector_type {
    return (_max + _min) / 2;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto ScalarBoundingBox<T, _Dim>::Extents() const -> vector_type {
    return _max - _min;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::HasPositiveExtents() const {
    for (size_t i = 0; i < _Dim; ++i)
        if (_min[i] > _max[i])
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::HasPositiveExtentsStrict() const {
    for (size_t i = 0; i < _Dim; ++i)
        if (_min[i] >= _max[i])
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void ScalarBoundingBox<T, _Dim>::Add(const vector_type& v) {
    _min = Core::Min(_min, v);
    _max = Core::Max(_max, v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void ScalarBoundingBox<T, _Dim>::Add(const ScalarBoundingBox& other) {
    _min = Core::Min(_min, other._min);
    _max = Core::Max(_max, other._max);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::Contains(const vector_type& v) const {
    return _min.AllLessOrEqual(v) && _max.AllGreaterOrEqual(v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::ContainsStrict(const vector_type& v) const {
    return _min.AllLessThan(v) && _max.AllGreaterThan(v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::ContainsMaxStrict(const vector_type& v) const {
    return _min.AllLessOrEqual(v) && _max.AllGreaterThan(v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::Contains(const ScalarBoundingBox& other) const {
    return _min.AllLessOrEqual(other._min) && _max.AllGreaterOrEqual(other._max);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::ContainsStrict(const ScalarBoundingBox& other) const {
    return _min.AllLessThan(other._min) && _max.AllGreaterThan(other._max);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::ContainsMaxStrict(const ScalarBoundingBox& other) const {
    return _min.AllLessOrEqual(other._min) && _max.AllGreaterThan(other._max);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::Intersects(const ScalarBoundingBox& other, bool *inside) const {
    const vector_type c0 = Center();
    const vector_type c1 = other.Center();

    const vector_type h0 = Extents() / 2;
    const vector_type h1 = other.Extents() / 2;

    const bool intersects = (c0 - c1).AllLessThan(h0 + h1);

    if (intersects && inside) {
        if (Contains(other._min))
            *inside = Contains(other._max);
        else if (other.Contains(_min))
            *inside = other.Contains(_max);
        else
            *inside = false;
    }

    return intersects;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto ScalarBoundingBox<T, _Dim>::Lerp(U f) const -> vector_type {
    return Core::Lerp(_min, _max, f);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto ScalarBoundingBox<T, _Dim>::Lerp(const ScalarVector<U, _Dim>& f) const -> vector_type {
    return Core::Lerp(_min, _max, f);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto ScalarBoundingBox<T, _Dim>::SLerp(U f) const -> vector_type {
    return Core::SLerp(_min, _max, f);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto ScalarBoundingBox<T, _Dim>::SLerp(const ScalarVector<U, _Dim>& f) const -> vector_type {
    return Core::SLerp(_min, _max, f);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto ScalarBoundingBox<T, _Dim>::ClipAbove(size_t axis, T value) const -> ScalarBoundingBox {
    Assert(_min[axis] <= value && _max[axis] >= value);
    ScalarBoundingBox result = *this;
    result._min[axis] = value;
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto ScalarBoundingBox<T, _Dim>::ClipBelow(size_t axis, T value) const -> ScalarBoundingBox {
    Assert(_min[axis] <= value && _max[axis] >= value);
    ScalarBoundingBox result = *this;
    result._max[axis] = value;
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void ScalarBoundingBox<T, _Dim>::Swap(ScalarBoundingBox& other) {
    _min.Swap(other._min);
    _max.Swap(other._max);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
ScalarBoundingBox<U, _Dim> ScalarBoundingBox<T, _Dim>::Cast() const {
    return ScalarBoundingBox<U, _Dim>(_min.Cast<U>(), _max.Cast<U>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
