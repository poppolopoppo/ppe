#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Geometry/ScalarVectorHelpers.h"
#include "Core/Maths/Packing/PackingHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct UX10Y10Z10W2N {
    u32 _data;

    UX10Y10Z10W2N() {}
    UX10Y10Z10W2N(u32 data) : _data(data) {}
    ~UX10Y10Z10W2N() {}

    FORCE_INLINE void Pack_Float01(const ScalarVector<float, 3>& xyz, u8 w);
    FORCE_INLINE void Unpack_Float01(ScalarVector<float, 3>& xyz) const;
    FORCE_INLINE void Unpack_Float01(ScalarVector<float, 3>& xyz, u8& w) const;

    FORCE_INLINE void Pack_FloatM11(const ScalarVector<float, 3>& xyz, u8 w);
    FORCE_INLINE void Unpack_FloatM11(ScalarVector<float, 3>& xyz) const;
    FORCE_INLINE void Unpack_FloatM11(ScalarVector<float, 3>& xyz, u8& w) const;

    FORCE_INLINE bool operator ==(const UX10Y10Z10W2N& other) const { return _data == other._data; }
    FORCE_INLINE bool operator !=(const UX10Y10Z10W2N& other) const { return !operator ==(other); }
};
//----------------------------------------------------------------------------
UX10Y10Z10W2N Float01_to_UX10Y10Z10W2N(const ScalarVector<float, 3>& xyz, u8 w);
UX10Y10Z10W2N Float01_to_UX10Y10Z10W2N(float x, float y, float z, u8 w);
//----------------------------------------------------------------------------
UX10Y10Z10W2N FloatM11_to_UX10Y10Z10W2N(const ScalarVector<float, 3>& xyz, u8 w);
UX10Y10Z10W2N FloatM11_to_UX10Y10Z10W2N(float x, float y, float z, u8 w);
//----------------------------------------------------------------------------
template <>
struct NumericLimits< UX10Y10Z10W2N > {
    static const UX10Y10Z10W2N Epsilon;
    static const UX10Y10Z10W2N Inf;
    static const UX10Y10Z10W2N MaxValue;
    static const UX10Y10Z10W2N MinValue;
    static const UX10Y10Z10W2N Nan;
    static const UX10Y10Z10W2N Default;
    static const UX10Y10Z10W2N Zero;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef ScalarVector<HalfFloat, 2> half2;
typedef ScalarVector<HalfFloat, 4> half4;
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<HalfFloat, _Dim> HalfPack(const ScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> HalfUnpack(const ScalarVector<HalfFloat, _Dim>& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef ScalarVector<SNorm<u8>, 2> byte2n;
typedef ScalarVector<SNorm<u8>, 4> byte4n;
typedef ScalarVector<UNorm<u8>, 2> ubyte2n;
typedef ScalarVector<UNorm<u8>, 4> ubyte4n;

typedef ScalarVector<SNorm<u16>, 2> short2n;
typedef ScalarVector<SNorm<u16>, 4> short4n;
typedef ScalarVector<UNorm<u16>, 2> ushort2n;
typedef ScalarVector<UNorm<u16>, 4> ushort4n;

typedef ScalarVector<SNorm<u32>, 2> word2n;
typedef ScalarVector<SNorm<u32>, 3> word3n;
typedef ScalarVector<SNorm<u32>, 4> word4n;
typedef ScalarVector<UNorm<u32>, 2> uword2n;
typedef ScalarVector<UNorm<u32>, 3> uword3n;
typedef ScalarVector<UNorm<u32>, 4> uword4n;
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<UNorm<T>, _Dim> UNormPack(const ScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<SNorm<T>, _Dim> SNormPack(const ScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <typename _Traits, typename T, size_t _Dim>
ScalarVector<float, _Dim> NormUnpack(const ScalarVector<BasicNorm<T, _Traits>, _Dim>& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/Packing/PackedVectors-inl.h"
