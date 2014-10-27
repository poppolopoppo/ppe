#pragma once

#include "Core/Maths/Geometry/ScalarBoundingBox.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarBoundingBox<T, _Dim>::ScalarBoundingBox()
:   ScalarBoundingBox(vector_type::MaxValue(), vector_type::MinValue()) {}
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
void ScalarBoundingBox<T, _Dim>::Add(const vector_type& v) const {
    _min = ::Min(_min, v);
    _max = ::Max(_max, v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void ScalarBoundingBox<T, _Dim>::Add(const ScalarBoundingBox& other) const {
    _min = ::Min(_min, other._min);
    _max = ::Max(_max, other._max);
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
    return _min.AllLessOrEqual(other._min) && _max.AllLessOrEqual(other._max);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::ContainsStrict(const ScalarBoundingBox& other) const {
    return _min.AllLessThan(other._min) && _max.AllLessThan(other._max);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool ScalarBoundingBox<T, _Dim>::ContainsMaxStrict(const ScalarBoundingBox& other) const {
    return _min.AllLessOrEqual(other._min) && _max.AllLessThan(other._max);
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
template <size_t _Dim2>
void ScalarBoundingBox<T, _Dim>::GetCorners(vector_type (&points)[_Dim2]) const {
    return GetCorners(MakeView(points));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void ScalarBoundingBox<T, _Dim>::GetCorners(const MemoryView<vector_type>& points) const {
    if (_Dim == 1) {
        Assert(points.size() == 2);
        points[0].x() = _min.x();
        points[1].x() = _max.x();
    }
    else if (_Dim == 2) {
        Assert(points.size() == 2);
        points[0].x() = _min.x();
        points[0].y() = _min.y();
        points[1].x() = _max.x();
        points[1].y() = _min.y();
        points[2].x() = _max.x();
        points[2].y() = _max.y();
        points[3].x() = _min.x();
        points[3].y() = _max.y();
    }
    else if (_Dim == 3) {
        Assert(points.size() == 8);
        points[0].x() = _min.x();
        points[0].y() = _min.y();
        points[0].z() = _min.z();
        points[1].x() = _min.x();
        points[1].y() = _min.y();
        points[1].z() = _max.z();
        points[2].x() = _min.x();
        points[2].y() = _max.y();
        points[2].z() = _max.z();
        points[3].x() = _min.x();
        points[3].y() = _max.y();
        points[3].z() = _min.z();
        points[4].x() = _max.x();
        points[4].y() = _min.y();
        points[4].z() = _max.z();
        points[5].x() = _max.x();
        points[5].y() = _max.y();
        points[5].z() = _max.z();
        points[6].x() = _max.x();
        points[6].y() = _max.y();
        points[6].z() = _min.z();
        points[7].x() = _max.x();
        points[7].y() = _max.y();
        points[7].z() = _max.z();
    }
    else {
        AssertNotImplemented();
    }
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
