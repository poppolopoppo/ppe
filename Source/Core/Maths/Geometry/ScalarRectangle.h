#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarBoundingBox.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
template <typename T, size_t _Dim>
class ScalarRectangleBase {};
template <typename T>
class ScalarRectangleBase<T, 1> {
public:
    ScalarRectangleBase() = default;
    T Left() const;
    T Right() const;
    void SetLeft(T value);
    void SetRight(T value);
    T Width() const { return Right() - Left(); }
    void SetWidth(T value);
};
template <typename T>
class ScalarRectangleBase<T, 2> : public ScalarRectangleBase<T, 1> {
public:
    ScalarRectangleBase() = default;
    ScalarRectangleBase(T left, T top, T width, T height);
    T Top() const;
    T Bottom() const;
    void SetTop(T value);
    void SetBottom(T value);
    T Height() const { return Bottom() - Top(); }
    void SetHeight(T value);
    float AspectRatio() const { return float(Width()) / Height(); }
};
template <typename T>
class ScalarRectangleBase<T, 3> : public ScalarRectangleBase<T, 2> {
public:
    ScalarRectangleBase() = default;
    ScalarRectangleBase(T left, T top, T width, T height, T near, T far);
    T Near() const;
    T Far() const;
    void SetNear(T value);
    void SetFar(T value);
    T Depth() const { return Far() - Near(); }
    void SetDepth(T value);
};
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarRectangle
:   public ScalarBoundingBox<T, _Dim>
,   public details::ScalarRectangleBase<T, _Dim> {
public:
    template <typename T2, size_t _Dim2>
    friend class details::ScalarRectangleBase;
    using details::ScalarRectangleBase<T, _Dim>::ScalarRectangleBase;

    typedef ScalarBoundingBox<T, _Dim> aabb_type;

    ScalarRectangle();
    explicit ScalarRectangle(Meta::noinit_tag);
    explicit ScalarRectangle(const aabb_type& aabb);
    ~ScalarRectangle();

    ScalarRectangle(const ScalarRectangle& other);
    ScalarRectangle& operator =(const ScalarRectangle& other);

    template <typename U>
    ScalarRectangle(const ScalarRectangle<U, _Dim>& other);
    template <typename U>
    ScalarRectangle& operator =(const ScalarRectangle<U, _Dim>& other);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Geometry/ScalarRectangle-inl.h"
