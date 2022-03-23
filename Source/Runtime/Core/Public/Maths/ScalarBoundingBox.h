#pragma once

#include "Core.h"

#include "Maths/ScalarBoundingBox_fwd.h"

#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

#include "IO/TextWriter_fwd.h"

namespace PPE {
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
    explicit TScalarBoundingBox(Meta::FNoInit) NOEXCEPT {}
    TScalarBoundingBox(const vector_type& min, const vector_type& max);

    TScalarBoundingBox(const TScalarBoundingBox& other);
    TScalarBoundingBox& operator =(const TScalarBoundingBox& other);

    TScalarBoundingBox(std::initializer_list<vector_type> points);
    TScalarBoundingBox& operator =(std::initializer_list<vector_type> points);

    template <typename U>
    TScalarBoundingBox(const TScalarBoundingBox<U, _Dim>& other);
    template <typename U>
    TScalarBoundingBox& operator =(const TScalarBoundingBox<U, _Dim>& other);

    vector_type& Min() { return _min; }
    vector_type& Max() { return _max; }

    const vector_type& Min() const { return _min; }
    const vector_type& Max() const { return _max; }

    void SetMinMax(const vector_type& vmin, const vector_type& vmax) {
        Assert(AllLessEqual(vmin, vmax));
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

    bool Intersects(const TScalarBoundingBox& other) const;
    bool Intersects(const TScalarBoundingBox& other, bool* inside) const;

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

    friend hash_t hash_value(const TScalarBoundingBox& b) NOEXCEPT { return hash_tuple(b._min, b._max); }

    template <typename U>
    TScalarBoundingBox<U, _Dim> Cast() const;

    static auto EmptyValue() { return TScalarBoundingBox(vector_type::MaxValue, vector_type::Lowest); }
    static auto MaxMinValue() { return TScalarBoundingBox(vector_type::MaxValue, vector_type::MinValue); }
    static auto MinMaxValue() { return TScalarBoundingBox(vector_type::MinValue, vector_type::MaxValue); }
    static auto MinusOneOneValue() { return TScalarBoundingBox(vector_type::MinusOne, vector_type::One); }
    static auto ZeroOneValue() { return TScalarBoundingBox(vector_type::Zero, vector_type::One); }

    static TScalarBoundingBox<T, _Dim> DefaultValue() { return EmptyValue(); }

    template <size_t... _Indices>
    auto Shuffle() const {
        STATIC_CONST_INTEGRAL(size_t, ShuffleDim, sizeof...(_Indices));
        return TScalarBoundingBox<T, ShuffleDim>(PPE::Shuffle<_Indices...>(_min), PPE::Shuffle<_Indices...>(_max));
    }

    TScalarBoundingBox& operator +=(const vector_type& v) { _min += v; _max += v; return (*this); }
    TScalarBoundingBox& operator -=(const vector_type& v) { _min -= v; _max -= v; return (*this); }

    TScalarBoundingBox operator +(const vector_type& v) const { return { _min + v, _max + v }; }
    TScalarBoundingBox operator -(const vector_type& v) const { return { _min - v, _max - v }; }

private:
    vector_type _min;
    vector_type _max;

public:
    // All shuffle specializations :

#define DEF_SCALARBOUNDINGBOX_SHUFFLE2(_Name, _0, _1) \
    FORCE_INLINE TScalarBoundingBox<T, 2> _Name() const { return Shuffle<_0, _1>(); }
#define DEF_SCALARBOUNDINGBOX_SHUFFLE3(_Name, _0, _1, _2) \
    FORCE_INLINE TScalarBoundingBox<T, 3> _Name() const { return Shuffle<_0, _1, _2>(); }
#define DEF_SCALARBOUNDINGBOX_SHUFFLE4(_Name, _0, _1, _2, _3) \
    FORCE_INLINE TScalarBoundingBox<T, 4> _Name() const { return Shuffle<_0, _1, _2, _3>(); }

#   include "Maths/ScalarBoundingBox.Shuffle-inl.h"

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
template <typename T, size_t _Dim>
void swap(TScalarBoundingBox<T, _Dim>& lhs, TScalarBoundingBox<T, _Dim>& rhs) NOEXCEPT {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename T, size_t _Dim>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TScalarBoundingBox<T, _Dim>& v) {
    return oss << v.Min() << v.Max();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/ScalarBoundingBox-inl.h"

#if 0
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<int, 1>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<int, 2>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<int, 3>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<unsigned int, 1>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<unsigned int, 2>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<unsigned int, 3>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<unsigned int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<float, 1>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<float, 2>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<float, 3>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<float, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<double, 1>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<double, 2>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<double, 3>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoundingBox<double, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<int, 1>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<int, 2>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<int, 3>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<unsigned int, 1>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<unsigned int, 2>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<unsigned int, 3>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<unsigned int, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<float, 1>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<float, 2>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<float, 3>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<float, 4>;
//----------------------------------------------------------------------------
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<double, 1>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<double, 2>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<double, 3>;
EXTERN_TEMPLATE_CLASS_DECL(PPE_CORE_API) TScalarBoxWExtent<double, 4>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
#endif //!#if 0
