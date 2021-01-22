#pragma once

#include "Core.h"

#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarRectangle_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class TScalarRectangle : public TScalarBoundingBox<T, _Dim> {
public:
    using aabb_type = TScalarBoundingBox<T, _Dim>;
    using vector_type = typename aabb_type::vector_type;

    TScalarRectangle() = default;
    TScalarRectangle(T left, T top, T width, T height);
    TScalarRectangle(T left, T top, T width, T height, T znear, T zfar);
    TScalarRectangle(const vector_type& vmin, const vector_type& vmax);
    explicit TScalarRectangle(const aabb_type& aabb);
    explicit TScalarRectangle(const vector_type& extent);
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
PPE_ASSUME_TEMPLATE_AS_POD(TScalarRectangle<T COMMA _Dim>, typename T, size_t _Dim)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/ScalarRectangle-inl.h"
