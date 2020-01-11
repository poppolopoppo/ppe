#pragma once

#include "Maths/ScalarBoundingBox.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarBoundingBox<T, _Dim>::TScalarBoundingBox()
:   TScalarBoundingBox(TScalarBoundingBox::DefaultValue()) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarBoundingBox<T, _Dim>::TScalarBoundingBox(const vector_type& min, const vector_type& max)
:   _min(min), _max(max) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarBoundingBox<T, _Dim>::TScalarBoundingBox(const TScalarBoundingBox& other)
:   _min(other._min), _max(other._max) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto TScalarBoundingBox<T, _Dim>::operator =(const TScalarBoundingBox& other) -> TScalarBoundingBox& {
    _min = other._min;
    _max = other._max;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarBoundingBox<T, _Dim>::TScalarBoundingBox(std::initializer_list<vector_type> points)
    : TScalarBoundingBox() {
    AddRange(points.begin(), points.end());
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto TScalarBoundingBox<T, _Dim>::operator =(std::initializer_list<vector_type> points) -> TScalarBoundingBox& {
    *this = TScalarBoundingBox::DefaultValue();
    AddRange(points.begin(), points.end());
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
TScalarBoundingBox<T, _Dim>::TScalarBoundingBox(const TScalarBoundingBox<U, _Dim>& other)
:   _min(other._min), _max(other._max) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto TScalarBoundingBox<T, _Dim>::operator =(const TScalarBoundingBox<U, _Dim>& other) -> TScalarBoundingBox& {
    _min = other._min;
    _max = other._max;
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto TScalarBoundingBox<T, _Dim>::Center() const -> vector_type {
    return ((_max + _min) / 2);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto TScalarBoundingBox<T, _Dim>::Extents() const -> vector_type {
    return (_max - _min);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto TScalarBoundingBox<T, _Dim>::HalfExtents() const -> vector_type {
    return ((_max - _min) / 2);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarBoundingBox<T, _Dim>::HasPositiveExtents() const {
    for (size_t i = 0; i < _Dim; ++i)
        if (_min[i] > _max[i])
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarBoundingBox<T, _Dim>::HasPositiveExtentsStrict() const {
    for (size_t i = 0; i < _Dim; ++i)
        if (_min[i] >= _max[i])
            return false;
    return true;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void TScalarBoundingBox<T, _Dim>::Add(const vector_type& v) {
    _min = PPE::Min(_min, v);
    _max = PPE::Max(_max, v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void TScalarBoundingBox<T, _Dim>::Add(const TScalarBoundingBox& other) {
    _min = PPE::Min(_min, other._min);
    _max = PPE::Max(_max, other._max);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarBoundingBox<T, _Dim>::Contains(const vector_type& v) const {
    return (AllLessEqual(_min, v) &&
            AllGreaterEqual(_max, v) );
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarBoundingBox<T, _Dim>::ContainsStrict(const vector_type& v) const {
    return (AllLess(_min, v) &&
            AllGreater(_max, v) );
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarBoundingBox<T, _Dim>::ContainsMaxStrict(const vector_type& v) const {
    return (AllLessEqual(_min, v) &&
            AllGreater(_max, v) );
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarBoundingBox<T, _Dim>::Contains(const TScalarBoundingBox& other) const {
    return (AllLessEqual(_min, other._min) &&
            AllGreaterEqual(_max, other._max) );
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarBoundingBox<T, _Dim>::ContainsStrict(const TScalarBoundingBox& other) const {
    return (AllLess(_min, other._min) &&
            AllGreater(_max, other._max) );
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarBoundingBox<T, _Dim>::ContainsMaxStrict(const TScalarBoundingBox& other) const {
    return (AllLessEqual(_min, other._min) &&
            AllGreater(_max, other._max) );
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarBoundingBox<T, _Dim>::Intersects(const TScalarBoundingBox& other) const {
#if 0
    return (Abs(Center() - other.Center()) * 2).AllLessThan(Extents() + other.Extents());
#else
    for (size_t i = 0; i < _Dim; ++i) {
        const T m = PPE::Min(_min.data[i], other._min.data[i]);
        const T M = PPE::Max(_max.data[i], other._max.data[i]);
        const T e = (_max.data[i] - _min.data[i]) +
            (other._max.data[i] - other._min.data[i]);
        if (M - m >= e)
            return false;
    }
    return true;
#endif
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool TScalarBoundingBox<T, _Dim>::Intersects(const TScalarBoundingBox& other, bool* inside) const {
    Assert(inside);

    if (Intersects(other)) {
        if (Contains(other._min))
            *inside = Contains(other._max);
        else if (other.Contains(_min))
            *inside = other.Contains(_max);
        else
            *inside = false;

        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <size_t _Dim2>
void TScalarBoundingBox<T, _Dim>::GetCorners(vector_type (&points)[_Dim2]) const {
    return GetCorners(MakeView(points));
}
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Dim>
struct TGetCornersAABB_;
template <typename T>
struct TGetCornersAABB_<T, 1> {
    void operator ()(const TMemoryView<TScalarVector<T, 1>>& points, const TScalarVector<T, 1>& min, const TScalarVector<T, 1>& max) const {
        Assert(points.size() == 2);
        points[0].x = min.x;
        points[1].x = max.x;
    }
};
template <typename T>
struct TGetCornersAABB_<T, 2> {
    void operator ()(const TMemoryView<TScalarVector<T, 2>>& points, const TScalarVector<T, 2>& min, const TScalarVector<T, 2>& max) const {
        Assert(points.size() == 4);
        points[0].x = min.x;
        points[0].y = min.y;

        points[1].x = max.x;
        points[1].y = min.y;

        points[2].x = max.x;
        points[2].y = max.y;

        points[3].x = min.x;
        points[3].y = max.y;
    }
};
template <typename T>
struct TGetCornersAABB_<T, 3> {
    void operator ()(const TMemoryView<TScalarVector<T, 3>>& points, const TScalarVector<T, 3>& min, const TScalarVector<T, 3>& max) const {
        Assert(points.size() == 8);
        points[0].x = min.x;
        points[0].y = min.y;
        points[0].z = min.z;

        points[1].x = min.x;
        points[1].y = min.y;
        points[1].z = max.z;

        points[2].x = min.x;
        points[2].y = max.y;
        points[2].z = max.z;

        points[3].x = min.x;
        points[3].y = max.y;
        points[3].z = min.z;

        points[4].x = max.x;
        points[4].y = min.y;
        points[4].z = max.z;

        points[5].x = max.x;
        points[5].y = max.y;
        points[5].z = max.z;

        points[6].x = max.x;
        points[6].y = max.y;
        points[6].z = min.z;

        points[7].x = max.x;
        points[7].y = max.y;
        points[7].z = max.z;
    }
};
} //!details

template <typename T, size_t _Dim>
void TScalarBoundingBox<T, _Dim>::GetCorners(const TMemoryView<vector_type>& points) const {
    details::TGetCornersAABB_<T, _Dim>()(points, _min, _max);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto TScalarBoundingBox<T, _Dim>::Lerp(U f) const -> vector_type {
    return PPE::Lerp(_min, _max, f);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto TScalarBoundingBox<T, _Dim>::Lerp(const TScalarVector<U, _Dim>& f) const -> vector_type {
    return PPE::Lerp(_min, _max, f);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto TScalarBoundingBox<T, _Dim>::SLerp(U f) const -> vector_type {
    return PPE::SLerp(_min, _max, f);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto TScalarBoundingBox<T, _Dim>::SLerp(const TScalarVector<U, _Dim>& f) const -> vector_type {
    return PPE::SLerp(_min, _max, f);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto TScalarBoundingBox<T, _Dim>::ClipAbove(size_t axis, T value) const -> TScalarBoundingBox {
    Assert(_min[axis] <= value && _max[axis] >= value);
    TScalarBoundingBox result = *this;
    result._min[axis] = value;
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto TScalarBoundingBox<T, _Dim>::ClipBelow(size_t axis, T value) const -> TScalarBoundingBox {
    Assert(_min[axis] <= value && _max[axis] >= value);
    TScalarBoundingBox result = *this;
    result._max[axis] = value;
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void TScalarBoundingBox<T, _Dim>::Swap(TScalarBoundingBox& other) {
    swap(_min, other._min);
    swap(_max, other._max);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
TScalarBoundingBox<U, _Dim> TScalarBoundingBox<T, _Dim>::Cast() const {
    return TScalarBoundingBox<U, _Dim>(_min.template Cast<U>(), _max.template Cast<U>());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
