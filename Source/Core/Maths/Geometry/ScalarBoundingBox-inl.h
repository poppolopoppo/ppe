#pragma once

#include "Core/Maths/Geometry/ScalarBoundingBox.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarBoundingBox<T, _Dim>::ScalarBoundingBox()
:   ScalarBoundingBox(ScalarBoundingBox::DefaultValue()) {}
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
template <size_t _Dim2>
void ScalarBoundingBox<T, _Dim>::GetCorners(vector_type (&points)[_Dim2]) const {
    return GetCorners(MakeView(points));
}
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Dim>
struct GetCornersAABB_;
template <typename T>
struct GetCornersAABB_<T, 1> {
    void operator ()(const MemoryView<ScalarVector<T, 1>>& points, const ScalarVector<T, 1>& min, const ScalarVector<T, 1>& max) const {
        Assert(points.size() == 2);
        points[0].x() = min.x();
        points[1].x() = max.x();
    }
};
template <typename T>
struct GetCornersAABB_<T, 2> {
    void operator ()(const MemoryView<ScalarVector<T, 2>>& points, const ScalarVector<T, 2>& min, const ScalarVector<T, 2>& max) const {
        Assert(points.size() == 4);
        points[0].x() = min.x();
        points[0].y() = min.y();

        points[1].x() = max.x();
        points[1].y() = min.y();

        points[2].x() = max.x();
        points[2].y() = max.y();

        points[3].x() = min.x();
        points[3].y() = max.y();
    }
};
template <typename T>
struct GetCornersAABB_<T, 3> {
    void operator ()(const MemoryView<ScalarVector<T, 3>>& points, const ScalarVector<T, 3>& min, const ScalarVector<T, 3>& max) const {
        Assert(points.size() == 8);
        points[0].x() = min.x();
        points[0].y() = min.y();
        points[0].z() = min.z();

        points[1].x() = min.x();
        points[1].y() = min.y();
        points[1].z() = max.z();

        points[2].x() = min.x();
        points[2].y() = max.y();
        points[2].z() = max.z();

        points[3].x() = min.x();
        points[3].y() = max.y();
        points[3].z() = min.z();

        points[4].x() = max.x();
        points[4].y() = min.y();
        points[4].z() = max.z();

        points[5].x() = max.x();
        points[5].y() = max.y();
        points[5].z() = max.z();

        points[6].x() = max.x();
        points[6].y() = max.y();
        points[6].z() = min.z();

        points[7].x() = max.x();
        points[7].y() = max.y();
        points[7].z() = max.z();
    }
};
} //!details

template <typename T, size_t _Dim>
void ScalarBoundingBox<T, _Dim>::GetCorners(const MemoryView<vector_type>& points) const {
    details::GetCornersAABB_<T, _Dim>()(points, _min, _max);
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
