#pragma once

#include "Maths/ScalarRectangle.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarRectangle<T, _Dim>::TScalarRectangle(T left, T top, T width, T height)
:   aabb_type(  TScalarVector<T, 2>(left, top),
                TScalarVector<T, 2>(left + width, top + height) ) {
    STATIC_ASSERT(_Dim == 2);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarRectangle<T, _Dim>::TScalarRectangle(T left, T top, T width, T height, T znear, T zfar)
:   aabb_type(  TScalarVector<T, 3>(left, top, znear),
                TScalarVector<T, 3>(left + width, top + height, zfar) ) {
    STATIC_ASSERT(_Dim == 3);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarRectangle<T, _Dim>::TScalarRectangle(const vector_type& vmin, const vector_type& vmax)
:   aabb_type(vmin, vmax) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarRectangle<T, _Dim>::TScalarRectangle(const aabb_type& aabb)
:   aabb_type(aabb) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarRectangle<T, _Dim>::TScalarRectangle(const vector_type& extent)
:   aabb_type(vector_type::Zero, extent) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarRectangle<T, _Dim>::~TScalarRectangle() = default;
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarRectangle<T, _Dim>::TScalarRectangle(const TScalarRectangle& other)
:   aabb_type(other) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
auto TScalarRectangle<T, _Dim>::operator =(const TScalarRectangle& other) -> TScalarRectangle& {
    aabb_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
TScalarRectangle<T, _Dim>::TScalarRectangle(const TScalarRectangle<U, _Dim>& other)
:   aabb_type(other) {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
template <typename U>
auto TScalarRectangle<T, _Dim>::operator =(const TScalarRectangle<U, _Dim>& other) -> TScalarRectangle& {
    aabb_type::operator =(other);
    return *this;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
