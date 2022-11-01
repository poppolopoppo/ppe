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
template <typename T, u32 _Dim>
class TScalarBoundingBox {
public:
    typedef TScalarVector<T, _Dim> vector_type;

    template <typename U, u32 _Dim2>
    friend class TScalarBoundingBox;

    TScalarBoundingBox();
    explicit TScalarBoundingBox(Meta::FNoInit) NOEXCEPT {}
    TScalarBoundingBox(const vector_type& min, const vector_type& max);

    TScalarBoundingBox(const TScalarBoundingBox& other);
    TScalarBoundingBox& operator =(const TScalarBoundingBox& other);

    TScalarBoundingBox(std::initializer_list<vector_type> points);
    TScalarBoundingBox& operator =(std::initializer_list<vector_type> points);

    template <typename U>
    explicit TScalarBoundingBox(const TScalarBoundingBox<U, _Dim>& other);
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

    vector_type Corner(u32 index) const;
    void MakeCorners(const TMemoryView<vector_type>& points) const;

    template <typename U>
    vector_type Lerp(U f) const;
    template <typename U>
    vector_type Lerp(const TScalarVector<U, _Dim>& f) const;

    template <typename U>
    vector_type SLerp(U f) const;
    template <typename U>
    vector_type SLerp(const TScalarVector<U, _Dim>& f) const;

    TScalarBoundingBox ClipAbove(u32 axis, T value) const;
    TScalarBoundingBox ClipBelow(u32 axis, T value) const;

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

    template <u32... _Indices>
    auto Shuffle() const {
        return TScalarBoundingBox<T, sizeof...(_Indices)>{
            _min.template Shuffle<_Indices...>(),
            _max.template Shuffle<_Indices...>() };
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

//#define DEF_SCALARBOUNDINGBOX_SHUFFLE2(_Name, _0, _1) \
//    FORCE_INLINE TScalarBoundingBox<T, 2> _Name() const { return Shuffle<_0, _1>(); }
//#define DEF_SCALARBOUNDINGBOX_SHUFFLE3(_Name, _0, _1, _2) \
//    FORCE_INLINE TScalarBoundingBox<T, 3> _Name() const { return Shuffle<_0, _1, _2>(); }
//#define DEF_SCALARBOUNDINGBOX_SHUFFLE4(_Name, _0, _1, _2, _3) \
//    FORCE_INLINE TScalarBoundingBox<T, 4> _Name() const { return Shuffle<_0, _1, _2, _3>(); }
//
//#   include "Maths/ScalarBoundingBox.Shuffle-inl.h"
//
//#undef DEF_SCALARBOUNDINGBOX_SHUFFLE2
//#undef DEF_SCALARBOUNDINGBOX_SHUFFLE3
//#undef DEF_SCALARBOUNDINGBOX_SHUFFLE4
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
class TScalarBoxWExtent {
public:
    typedef TScalarVector<T, _Dim> vector_type;

    TScalarBoxWExtent() : _center(vector_type::Zero), _halfExtents(vector_type::Zero) {}

    TScalarBoxWExtent(const vector_type& center, const vector_type& halfExtents)
        : _center(center), _halfExtents(halfExtents) {
        Assert_NoAssume(HasPositiveExtents());
    }

    TScalarBoxWExtent(const TScalarBoundingBox<T, _Dim>& aabb)
        : _center(aabb.Center()), _halfExtents(aabb.Extents() / static_cast<T>(2)) {}

    vector_type& Center() { return _center; }
    const vector_type& Center() const { return _center; }

    vector_type& HalfExtents() { return _halfExtents; }
    const vector_type& HalfExtents() const { return _halfExtents; }

    vector_type Max() const { return (_center + _halfExtents); }
    vector_type Min() const { return (_center - _halfExtents); }

    bool HasPositiveExtents() const { return AllGreaterEqual(_halfExtents, vector_type::Zero); }
    bool HasPositiveExtentsStrict() const { return AllGreater(_halfExtents, vector_type::Zero); }

    bool Contains(const vector_type& p) const {
        IF_CONSTEXPR(TNumericLimits<T>::is_integer && not TNumericLimits<T>::is_signed) {
            using signed_t = std::make_signed_t<T>;
            const auto dist = Abs(checked_cast<signed_t>(p) - checked_cast<signed_t>(_center));
            return AllGreaterEqual(checked_cast<T>(dist), _halfExtents);
        } else {
            return AllGreaterEqual(Abs(p - _center), _halfExtents);
        }
    }

    bool Contains(const TScalarBoxWExtent& other) const {
        return (AllLessEqual(Min(), other.Min()) &&
                AllGreaterEqual(Max(), other.Max()) );
    }

    TScalarBoundingBox<T, _Dim> ToBoundingBox() const {
        return TScalarBoundingBox<T, _Dim>(Min(), Max());
    }

private:
    vector_type _center;
    vector_type _halfExtents;
};
//----------------------------------------------------------------------------
template <typename T, u32 _Dim>
void swap(TScalarBoundingBox<T, _Dim>& lhs, TScalarBoundingBox<T, _Dim>& rhs) NOEXCEPT {
    lhs.Swap(rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Char, typename T, u32 _Dim>
TBasicTextWriter<_Char>& operator <<(TBasicTextWriter<_Char>& oss, const TScalarBoundingBox<T, _Dim>& v) {
    return oss << v.Min() << v.Max();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/ScalarBoundingBox-inl.h"

#ifndef EXPORT_PPE_RUNTIME_CORE_SCALARBOUNDINGBOX
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
#endif
