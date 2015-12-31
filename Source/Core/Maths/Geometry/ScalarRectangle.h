#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarBoundingBox.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarRectangle : public ScalarBoundingBox<T, _Dim> {
public:
    typedef ScalarBoundingBox<T, _Dim> aabb_type;

    ScalarRectangle();
    ScalarRectangle(T left, T top, T width, T height);
    ScalarRectangle(T left, T top, T width, T height, T znear, T zfar);
    explicit ScalarRectangle(const aabb_type& aabb);
    ~ScalarRectangle();

    ScalarRectangle(const ScalarRectangle& other);
    ScalarRectangle& operator =(const ScalarRectangle& other);

    template <typename U>
    ScalarRectangle(const ScalarRectangle<U, _Dim>& other);
    template <typename U>
    ScalarRectangle& operator =(const ScalarRectangle<U, _Dim>& other);

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
typedef ScalarRectangle<float, 2> RectangleF;
//----------------------------------------------------------------------------
typedef ScalarRectangle<float, 3> ViewportF;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Geometry/ScalarRectangle-inl.h"
