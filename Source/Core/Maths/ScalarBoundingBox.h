#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarBoundingBox_fwd.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

#include <iosfwd>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class TScalarBoundingBox {
public:
    typedef TScalarVector<T, _Dim> vector_type;

    template <typename U, size_t _Dim2>
    friend class TScalarBoundingBox;

    TScalarBoundingBox();
    FORCE_INLINE explicit TScalarBoundingBox(Meta::FNoInit) {}
    TScalarBoundingBox(const vector_type& min, const vector_type& max);

    TScalarBoundingBox(const TScalarBoundingBox& other);
    TScalarBoundingBox& operator =(const TScalarBoundingBox& other);

    TScalarBoundingBox(std::initializer_list<vector_type> points);
    TScalarBoundingBox& operator =(std::initializer_list<vector_type> points);

    template <typename U>
    TScalarBoundingBox(const TScalarBoundingBox<U, _Dim>& other);
    template <typename U>
    TScalarBoundingBox& operator =(const TScalarBoundingBox<U, _Dim>& other);

    const vector_type& Min() const { return _min; }
    const vector_type& Max() const { return _max; }

    void SetMinMax(const vector_type& vmin, const vector_type& vmax) {
        Assert(vmin.AllLessOrEqual(vmax));
        _min = vmin;
        _max = vmax;
    }

    vector_type Center() const;
    vector_type Extents() const;
    vector_type HalfExtents() const;

    bool HasPositiveExtents() const;
    bool HasPositiveExtentsStrict() const;

    void Add(const vector_type& v);
    void Add(const TScalarBoundingBox& other);

    template <typename _It>
    void AddRange(_It&& begin, _It&& end) {
        for (; begin != end; ++begin)
            Add(*begin);
    }

    bool Contains(const vector_type& v) const;
    bool ContainsStrict(const vector_type& v) const;
    bool ContainsMaxStrict(const vector_type& v) const;

    bool Contains(const TScalarBoundingBox& other) const;
    bool ContainsStrict(const TScalarBoundingBox& other) const;
    bool ContainsMaxStrict(const TScalarBoundingBox& other) const;

    bool Intersects(const TScalarBoundingBox& other, bool *inside) const;

    template <size_t _Dim2>
    void GetCorners(vector_type (&points)[_Dim2]) const;
    void GetCorners(const TMemoryView<vector_type>& points) const;

    template <typename U>
    vector_type Lerp(U f) const;
    template <typename U>
    vector_type Lerp(const TScalarVector<U, _Dim>& f) const;

    template <typename U>
    vector_type SLerp(U f) const;
    template <typename U>
    vector_type SLerp(const TScalarVector<U, _Dim>& f) const;

    TScalarBoundingBox ClipAbove(size_t axis, T value) const;
    TScalarBoundingBox ClipBelow(size_t axis, T value) const;

    void Swap(TScalarBoundingBox& other);

    friend hash_t hash_value(const TScalarBoundingBox& b) { return hash_tuple(b._min, b._max); }

    template <typename U>
    TScalarBoundingBox<U, _Dim> Cast() const;

    static TScalarBoundingBox<T, _Dim> MaxMinValue() { return TScalarBoundingBox(vector_type::MaxValue(), vector_type::MinValue()); }
    static TScalarBoundingBox<T, _Dim> MinMaxValue() { return TScalarBoundingBox(vector_type::MinValue(), vector_type::MaxValue()); }
    static TScalarBoundingBox<T, _Dim> MinusOneOneValue() { return TScalarBoundingBox(vector_type::MinusOne(), vector_type::One()); }
    static TScalarBoundingBox<T, _Dim> ZeroOneValue() { return TScalarBoundingBox(vector_type::Zero(), vector_type::One()); }

    static TScalarBoundingBox<T, _Dim> DefaultValue() { return MaxMinValue(); }

    template <size_t _0, size_t _1>
    TScalarBoundingBox<T, 2> Shuffle2() const { return TScalarBoundingBox<T, 2>(_min.Shuffle2<_0, _1>(), _max.Shuffle2<_0, _1>()); }
    template <size_t _0, size_t _1, size_t _2>
    TScalarBoundingBox<T, 3> Shuffle3() const { return TScalarBoundingBox<T, 3>(_min.Shuffle3<_0, _1, _2>(), _max.Shuffle3<_0, _1, _2>()); }
    template <size_t _0, size_t _1, size_t _2, size_t _3>
    TScalarBoundingBox<T, 4> Shuffle4() const { return TScalarBoundingBox<T, 4>(_min.Shuffle3<_0, _1, _2, _3>(), _max.Shuffle3<_0, _1, _2, _3>()); }

private:
    vector_type _min;
    vector_type _max;

public:
    // All shuffle specializations :

#define DEF_SCALARBOUNDINGBOX_SHUFFLE2(_Name, _0, _1) \
    FORCE_INLINE TScalarBoundingBox<T, 2> _Name() const { return Shuffle2<_0, _1>(); }
#define DEF_SCALARBOUNDINGBOX_SHUFFLE3(_Name, _0, _1, _2) \
    FORCE_INLINE TScalarBoundingBox<T, 3> _Name() const { return Shuffle3<_0, _1, _2>(); }
#define DEF_SCALARBOUNDINGBOX_SHUFFLE4(_Name, _0, _1, _2, _3) \
    FORCE_INLINE TScalarBoundingBox<T, 4> _Name() const { return Shuffle4<_0, _1, _2, _3>(); }

#   include "Core/Maths/ScalarBoundingBox.Shuffle-inl.h"

#undef DEF_SCALARBOUNDINGBOX_SHUFFLE2
#undef DEF_SCALARBOUNDINGBOX_SHUFFLE3
#undef DEF_SCALARBOUNDINGBOX_SHUFFLE4
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class TScalarBoxWExtent {
public:
    typedef TScalarVector<T, _Dim> vector_type;

    TScalarBoxWExtent() : _center(T(0)), _halfExtents(T(0)) {}

    TScalarBoxWExtent(const vector_type& center, const vector_type& halfExtents)
        : _center(center), _halfExtents(halfExtents) {
        Assert(_halfExtents.AllGreaterOrEqual(vector_type(T(0))));
    }

    TScalarBoxWExtent(const TScalarBoundingBox<T, _Dim>& aabb)
        : _center(aabb.Center()), _halfExtents(aabb.Extents() / 2) {}

    vector_type& Center() { return _center; }
    const vector_type& Center() const { return _center; }

    vector_type& HalfExtents() { return _halfExtents; }
    const vector_type& HalfExtents() const { return _halfExtents; }

    vector_type Max() const { return (_center + _halfExtents); }
    vector_type Min() const { return (_center - _halfExtents); }

    bool HasPositiveExtents() const { return (_halfExtents.AllGreaterOrEqual(T(0))); }
    bool HasPositiveExtentsStrict() const { return (_halfExtents.AllGreaterThan(T(0))); }

    bool Contains(const vector_type& p) const {
        return (Abs(p - _center).AllGreaterOrEqual(_halfExtents));
    }

    bool Contains(const TScalarBoxWExtent& other) const {
        return (Min().AllLessOrEqual(other.Min()) &&
                Max().AllGreaterOrEqual(other.Max()) );
    }

    TScalarBoundingBox<T, _Dim> ToBoundingBox() const {
        return TScalarBoundingBox<T, _Dim>(Min(), Max());
    }

private:
    vector_type _center;
    vector_type _halfExtents;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename _Traits, typename T, size_t _Dim >
std::basic_ostream<_Char, _Traits>& operator <<(
    std::basic_ostream<_Char, _Traits>& oss,
    const TScalarBoundingBox<T, _Dim>& v) {
    return oss << v.Min() << v.Max();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void swap(TScalarBoundingBox<T, _Dim>& lhs, TScalarBoundingBox<T, _Dim>& rhs) {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// All scalar bounding boxes are considered as pods
//----------------------------------------------------------------------------
CORE_ASSUME_TYPE_AS_POD(TScalarBoundingBox<T COMMA _Dim>, typename T, size_t _Dim)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/ScalarBoundingBox-inl.h"
