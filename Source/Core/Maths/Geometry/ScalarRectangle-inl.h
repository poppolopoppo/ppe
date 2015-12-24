#pragma once

#include "Core/Maths/Geometry/ScalarRectangle.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T ScalarRectangleBase<T, 1>::Left() const {
    return static_cast<const ScalarRectangle<T, 1>*>(this)->_min.x();
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T ScalarRectangleBase<T, 1>::Right() const {
    return static_cast<const ScalarRectangle<T, 1>*>(this)->_max.x();
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void ScalarRectangleBase<T, 1>::SetLeft(T value) {
    static_cast<ScalarRectangle<T, 1>*>(this)->_min.x() = value;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void ScalarRectangleBase<T, 1>::SetRight(T value) {
    static_cast<ScalarRectangle<T, 1>*>(this)->_max.x() = value;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void ScalarRectangleBase<T, 1>::SetWidth(T value) {
    auto* const pself = static_cast<ScalarRectangle<T, 1>*>(this);
    pself->_max.x() = pself->_min.x() + value;
}
//----------------------------------------------------------------------------
} //! details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
ScalarRectangleBase<T, 2>::ScalarRectangleBase(T left, T top, T width, T height) {
    auto* const pself = static_cast<ScalarRectangle<T, 2>*>(this);
    pself->_min.x() = left;
    pself->_min.y() = top;
    pself->_max.x() = left + width;
    pself->_max.y() = top + height;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T ScalarRectangleBase<T, 2>::Top() const {
    return static_cast<const ScalarRectangle<T, 2>*>(this)->_min.y();
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T ScalarRectangleBase<T, 2>::Bottom() const {
    return static_cast<const ScalarRectangle<T, 2>*>(this)->_max.y();
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void ScalarRectangleBase<T, 2>::SetTop(T value) {
    static_cast<ScalarRectangle<T, 2>*>(this)->_min.y() = value;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void ScalarRectangleBase<T, 2>::SetBottom(T value) {
    static_cast<ScalarRectangle<T, 2>*>(this)->_max.y() = value;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void ScalarRectangleBase<T, 2>::SetHeight(T value) {
    auto* const pself = static_cast<ScalarRectangle<T, 2>*>(this);
    pself->_max.y() = pself->_min.y() + value;
}
//----------------------------------------------------------------------------
} //! details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T>
ScalarRectangleBase<T, 3>::ScalarRectangleBase(T left, T top, T width, T height, T near, T far) {
    auto* const pself = static_cast<ScalarRectangle<T, 3>*>(this);
    pself->_min.x() = left;
    pself->_min.y() = top;
    pself->_min.z() = near;
    pself->_max.x() = left + width;
    pself->_max.y() = top + height;
    pself->_max.z() = far;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T ScalarRectangleBase<T, 3>::Near() const {
    return static_cast<const ScalarRectangle<T, 3>*>(this)->_min.z();
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE T ScalarRectangleBase<T, 3>::Far() const {
    return static_cast<const ScalarRectangle<T, 3>*>(this)->_max.z();
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void ScalarRectangleBase<T, 3>::SetNear(T value) {
    static_cast<ScalarRectangle<T, 3>*>(this)->_min.z() = value;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void ScalarRectangleBase<T, 3>::SetFar(T value) {
    static_cast<ScalarRectangle<T, 3>*>(this)->_max.z() = value;
}
//----------------------------------------------------------------------------
template <typename T>
FORCE_INLINE void ScalarRectangleBase<T, 3>::SetDepth(T value) {
    auto* const pself = static_cast<ScalarRectangle<T, 3>*>(this);
    pself->_max.z() = pself->_min.z() + value;
}
//----------------------------------------------------------------------------
} //! details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarRectangle<T, _Dim>::ScalarRectangle() {}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarRectangle<T, _Dim>::ScalarRectangle(Meta::noinit_tag tag) : aabb_type(tag) {}
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
