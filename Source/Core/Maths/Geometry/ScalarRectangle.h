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

    T Left() const { return aabb_type::Min().get<0>(); }
    T Right() const { return aabb_type::Max().get<0>(); }

    T Top() const { return aabb_type::Min().get<1>(); }
    T Bottom() const { return aabb_type::Max().get<1>(); }

    T Near() const { return aabb_type::Min().get<2>(); }
    T Far() const { return aabb_type::Max().get<2>(); }

    T Width() const { return Right() - Left(); }
    T Height() const { return Bottom() - Top(); }
    T Depth() const { return Far() - Near(); }

    float AspectRatio() const { return float(Width()) / Height(); }

    void SetLeft(T value) { aabb_type::Min().get<0>() = value; }
    void SetRight(T value) { aabb_type::Max().get<0>() = value; }

    void SetTop(T value) { aabb_type::Min().get<1>() = value; }
    void SetBottom(T value) { aabb_type::Max().get<1>() = value; }

    void SetNear(T value) { aabb_type::Min().get<2>() = value; }
    void SetFar(T value) { aabb_type::Max().get<2>() = value; }

    void SetWidth(T value) { Assert(value >= 0); aabb_type::Max().get<0>() = aabb_type::Min().get<0>() + value; }
    void SetHeight(T value) { Assert(value >= 0); aabb_type::Max().get<1>() = aabb_type::Min().get<1>() + value; }
    void SetDepth(T value) { Assert(value >= 0); aabb_type::Max().get<2>() = aabb_type::Min().get<2>() + value; }
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
