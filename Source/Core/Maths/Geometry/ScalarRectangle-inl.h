#pragma once

#include "Core/Maths/Geometry/ScalarRectangle.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarRectangle<T, _Dim>::ScalarRectangle() {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarRectangle<T, _Dim>::ScalarRectangle(T left, T top, T width, T height)
:   aabb_type(  ScalarVector<T, 2>(left, top),
                ScalarVector<T, 2>(left + width, top + height) ) {
    STATIC_ASSERT(_Dim == 2);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarRectangle<T, _Dim>::ScalarRectangle(T left, T top, T width, T height, T znear, T zfar)
:   aabb_type(  ScalarVector<T, 3>(left, top, znear),
                ScalarVector<T, 3>(left + width, top + height, zfar) ) {
    STATIC_ASSERT(_Dim == 3);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarRectangle<T, _Dim>::ScalarRectangle(const aabb_type& aabb)
:   aabb_type(aabb) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarRectangle<T, _Dim>::~ScalarRectangle() {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarRectangle<T, _Dim>::ScalarRectangle(const ScalarRectangle& other)
:   aabb_type(other) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto ScalarRectangle<T, _Dim>::operator =(const ScalarRectangle& other) -> ScalarRectangle& {
    aabb_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
ScalarRectangle<T, _Dim>::ScalarRectangle(const ScalarRectangle<U, _Dim>& other)
:   aabb_type(other) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto ScalarRectangle<T, _Dim>::operator =(const ScalarRectangle<U, _Dim>& other) -> ScalarRectangle& {
    aabb_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
