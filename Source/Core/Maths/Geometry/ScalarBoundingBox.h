#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarBoundingBox_fwd.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Geometry/ScalarVectorHelpers.h"

#include "Core/Maths/Geometry/ScalarVector.Shuffle-inl.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace details {
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename _Impl>
class ScalarBoundingBoxBase {};
template <typename T, typename _Impl>
class ScalarBoundingBoxBase<T, 1, _Impl> {
public:
    template <size_t _0, size_t _1>
    ScalarBoundingBox<T, 2> Shuffle2() const;
    template <size_t _0, size_t _1, size_t _2>
    ScalarBoundingBox<T, 3> Shuffle3() const;
    template <size_t _0, size_t _1, size_t _2, size_t _3>
    ScalarBoundingBox<T, 4> Shuffle4() const;

#define DECL_SCALARBOUNDINGBOX_SHUFFLE1(_Dim, _Name, ...) \
    ScalarBoundingBox<T, _Dim> _Name() const { return Shuffle ## _Dim <__VA_ARGS__>(); }
    FOREACH_CORE_SCALARVECTOR_SHUFFLE1(DECL_SCALARBOUNDINGBOX_SHUFFLE1)
#undef DECL_SCALARBOUNDINGBOX_SHUFFLE1
};
template <typename T, typename _Impl>
class ScalarBoundingBoxBase<T, 2, _Impl> : public ScalarBoundingBoxBase<T, 1, _Impl> {
public:
#define DECL_SCALARBOUNDINGBOX_SHUFFLE2(_Dim, _Name, ...) \
    ScalarBoundingBox<T, _Dim> _Name() const { return Shuffle ## _Dim <__VA_ARGS__>(); }
    FOREACH_CORE_SCALARVECTOR_SHUFFLE2(DECL_SCALARBOUNDINGBOX_SHUFFLE2)
#undef DECL_SCALARBOUNDINGBOX_SHUFFLE2
};
template <typename T, typename _Impl>
class ScalarBoundingBoxBase<T, 3, _Impl> : public ScalarBoundingBoxBase<T, 2, _Impl> {
public:
#define DECL_SCALARBOUNDINGBOX_SHUFFLE3(_Dim, _Name, ...) \
    ScalarBoundingBox<T, _Dim> _Name() const { return Shuffle ## _Dim <__VA_ARGS__>(); }
    FOREACH_CORE_SCALARVECTOR_SHUFFLE3(DECL_SCALARBOUNDINGBOX_SHUFFLE3)
#undef DECL_SCALARBOUNDINGBOX_SHUFFLE3
};
template <typename T, typename _Impl>
class ScalarBoundingBoxBase<T, 4, _Impl> : public ScalarBoundingBoxBase<T, 3, _Impl> {
public:
#define DECL_SCALARBOUNDINGBOX_SHUFFLE4(_Dim, _Name, ...) \
    ScalarBoundingBox<T, _Dim> _Name() const { return Shuffle ## _Dim <__VA_ARGS__>(); }
    FOREACH_CORE_SCALARVECTOR_SHUFFLE4(DECL_SCALARBOUNDINGBOX_SHUFFLE4)
#undef DECL_SCALARBOUNDINGBOX_SHUFFLE4
};
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarBoundingBoxCorners {};
template <typename T>
class ScalarBoundingBoxCorners<T, 1> {
public:
    void GetCorners(ScalarVector<T, 1> (&points)[2]) const;
};
template <typename T>
class ScalarBoundingBoxCorners<T, 2> {
public:
    void GetCorners(ScalarVector<T, 2> (&points)[4]) const;
};
template <typename T>
class ScalarBoundingBoxCorners<T, 3> {
public:
    void GetCorners(ScalarVector<T, 3> (&points)[8]) const;
};
//----------------------------------------------------------------------------
} //!details
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarBoundingBox
:   public details::ScalarBoundingBoxBase<T, _Dim, ScalarBoundingBox<T, _Dim>>
,   public details::ScalarBoundingBoxCorners<T, _Dim> {
public:
    typedef ScalarVector<T, _Dim> vector_type;

    template <typename U, size_t _Dim2>
    friend class ScalarBoundingBox;
    template <typename U, size_t _Dim2, typename _Impl>
    friend class details::ScalarBoundingBoxBase;
    template <typename U, size_t _Dim2>
    friend class details::ScalarBoundingBoxCorners;

    ScalarBoundingBox();
    explicit ScalarBoundingBox(Meta::noinit_tag);
    ScalarBoundingBox(const vector_type& min, const vector_type& max);
    ~ScalarBoundingBox();

    ScalarBoundingBox(const ScalarBoundingBox& other);
    ScalarBoundingBox& operator =(const ScalarBoundingBox& other);

    template <typename U>
    ScalarBoundingBox(const ScalarBoundingBox<U, _Dim>& other);
    template <typename U>
    ScalarBoundingBox& operator =(const ScalarBoundingBox<U, _Dim>& other);

    const vector_type& Min() const { return _min; }
    const vector_type& Max() const { return _max; }

    vector_type Center() const;
    vector_type Extents() const;

    bool HasPositiveExtents() const;
    bool HasPositiveExtentsStrict() const;

    void Add(const vector_type& v);
    void Add(const ScalarBoundingBox& other);

    template <typename _It>
    void AddRange(_It&& begin, _It&& end) {
        for (; begin != end; ++begin)
            Add(*begin);
    }

    bool Contains(const vector_type& v) const;
    bool ContainsStrict(const vector_type& v) const;
    bool ContainsMaxStrict(const vector_type& v) const;

    bool Contains(const ScalarBoundingBox& other) const;
    bool ContainsStrict(const ScalarBoundingBox& other) const;
    bool ContainsMaxStrict(const ScalarBoundingBox& other) const;

    bool Intersects(const ScalarBoundingBox& other, bool *inside) const;

    template <typename U>
    vector_type Lerp(U f) const;
    template <typename U>
    vector_type Lerp(const ScalarVector<U, _Dim>& f) const;

    template <typename U>
    vector_type SLerp(U f) const;
    template <typename U>
    vector_type SLerp(const ScalarVector<U, _Dim>& f) const;

    ScalarBoundingBox ClipAbove(size_t axis, T value) const;
    ScalarBoundingBox ClipBelow(size_t axis, T value) const;

    void Swap(ScalarBoundingBox& other);

    friend hash_t hash_value(const ScalarBoundingBox& b) { return hash_tuple(b._min, b._max); }

    template <typename U>
    ScalarBoundingBox<U, _Dim> Cast() const;

    static ScalarBoundingBox<T, _Dim> MaxMinValue() { return ScalarBoundingBox(vector_type::MaxValue(), vector_type::MinValue()); }
    static ScalarBoundingBox<T, _Dim> MinMaxValue() { return ScalarBoundingBox(vector_type::MinValue(), vector_type::MaxValue()); }
    static ScalarBoundingBox<T, _Dim> MinusOneOneValue() { return ScalarBoundingBox(vector_type::MinusOne(), vector_type::One()); }
    static ScalarBoundingBox<T, _Dim> ZeroOneValue() { return ScalarBoundingBox(vector_type::Zero(), vector_type::One()); }

    static ScalarBoundingBox<T, _Dim> DefaultValue() { return MaxMinValue(); }

protected:
    vector_type _min;
    vector_type _max;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T, size_t _Dim >
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const ScalarBoundingBox<T, _Dim>& v) {
    return oss << v.Min() << v.Max();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void swap(ScalarBoundingBox<T, _Dim>& lhs, ScalarBoundingBox<T, _Dim>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Geometry/ScalarBoundingBox-inl.h"

#undef FOREACH_CORE_SCALARVECTOR_SHUFFLE2
#undef FOREACH_CORE_SCALARVECTOR_SHUFFLE3
#undef FOREACH_CORE_SCALARVECTOR_SHUFFLE4
