#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Maths/ScalarRectangle_fwd.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class TScalarRectangle : public TScalarBoundingBox<T, _Dim> {
public:
    typedef TScalarBoundingBox<T, _Dim> aabb_type;

    TScalarRectangle();
    TScalarRectangle(T left, T top, T width, T height);
    TScalarRectangle(T left, T top, T width, T height, T znear, T zfar);
    explicit TScalarRectangle(const aabb_type& aabb);
    ~TScalarRectangle();

    TScalarRectangle(const TScalarRectangle& other);
    TScalarRectangle& operator =(const TScalarRectangle& other);

    template <typename U>
    TScalarRectangle(const TScalarRectangle<U, _Dim>& other);
    template <typename U>
    TScalarRectangle& operator =(const TScalarRectangle<U, _Dim>& other);

    T Left()    const { return aabb_type::Min().x(); }
    T Right()   const { return aabb_type::Max().x(); }

    T Top()     const { return aabb_type::Min().y(); }
    T Bottom()  const { return aabb_type::Max().y(); }

    T Near()    const { return aabb_type::Min().z(); }
    T Far()     const { return aabb_type::Max().z(); }

    T Width()   const { return Right() - Left(); }
    T Height()  const { return Bottom() - Top(); }
    T Depth()   const { return Far() - Near(); }

    float AspectRatio() const { return float(Width()) / Height(); }

    void SetLeft(T value) { aabb_type::Min().x() = value; }
    void SetRight(T value) { aabb_type::Max().x() = value; }

    void SetTop(T value) { aabb_type::Min().y() = value; }
    void SetBottom(T value) { aabb_type::Max().y() = value; }

    void SetNear(T value) { aabb_type::Min().z() = value; }
    void SetFar(T value) { aabb_type::Max().z() = value; }

    void SetWidth(T value) { Assert(value >= 0); aabb_type::Max().x() = aabb_type::Min().x() + value; }
    void SetHeight(T value) { Assert(value >= 0); aabb_type::Max().y() = aabb_type::Min().y() + value; }
    void SetDepth(T value) { Assert(value >= 0); aabb_type::Max().z() = aabb_type::Min().z() + value; }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// All rectangles considered as pods
//----------------------------------------------------------------------------
CORE_ASSUME_TYPE_AS_POD(TScalarRectangle<T COMMA _Dim>, typename T, size_t _Dim)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/ScalarRectangle-inl.h"
