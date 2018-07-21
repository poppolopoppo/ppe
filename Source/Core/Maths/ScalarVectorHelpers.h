#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/MathHelpers.h"
#include "Core/Maths/PackingHelpers.h"

#include "Core/HAL/PlatformEndian.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Dot(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T Dot2(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T Dot3(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T Dot4(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T LengthSq(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T LengthSq2(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T LengthSq3(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T LengthSq4(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Length(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Length2(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Length3(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Length4(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float DistanceSq(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq2(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq3(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq4(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Distance(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance2(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance3(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance4(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Normalize(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE TScalarVector<T, _Dim> Normalize2(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE TScalarVector<T, _Dim> Normalize3(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE TScalarVector<T, _Dim> Normalize4(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Min(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Max(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Min(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Max(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void MinMax(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs,
            TScalarVector<T, _Dim>& min, TScalarVector<T, _Dim>& max);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Det(const TScalarVector<T, 2>& lhs, const TScalarVector<T, 2>& rhs);
//----------------------------------------------------------------------------
template <typename T>
float Cross(const TScalarVector<T, 2>& o, const TScalarVector<T, 2>& a, const TScalarVector<T, 2>& b);
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> Cross(const TScalarVector<T, 3>& lhs, const TScalarVector<T, 3>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Reflect(const TScalarVector<T, _Dim>& incident, const TScalarVector<T, _Dim>& normal);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Refract(const TScalarVector<T, _Dim>& incident, const TScalarVector<T, _Dim>& normal, T refractionIndex);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Abs(const TScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> BarycentricLerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, const TScalarVector<T, _Dim>& v2, U f0, U f1, U f2);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> BarycentricLerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, const TScalarVector<T, _Dim>& v2, const TScalarVector<U, 3>& uvw);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Ceil(const TScalarVector<T, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Clamp(const TScalarVector<T, _Dim>& value, T vmin, T vmax);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Clamp(const TScalarVector<T, _Dim>& value, const TScalarVector<T, _Dim>& vmin, const TScalarVector<T, _Dim>& vmax);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Floor(const TScalarVector<T, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> FMod(const TScalarVector<T, _Dim>& f, float m);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> FMod(const TScalarVector<T, _Dim>& f, const TScalarVector<T, _Dim>& m);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Frac(const TScalarVector<T, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> Lerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, U f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> Lerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, const TScalarVector<U, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<float, _Dim> LinearStep(const TScalarVector<T, _Dim>& value, const TScalarVector<T, _Dim>& vmin, const TScalarVector<T, _Dim>& vmax);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<float, _Dim> Pow(const TScalarVector<T, _Dim>& value, T n);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<float, _Dim> Pow(const TScalarVector<T, _Dim>& value, const TScalarVector<T, _Dim>& n);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Rcp(const TScalarVector<T, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Round(const TScalarVector<T, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> RSqrt(const TScalarVector<T, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Saturate(const TScalarVector<T, _Dim>& value) { return Clamp(value, T(0), T(1)); }
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> SLerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, U f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> SLerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, const TScalarVector<U, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Step(const TScalarVector<T, _Dim>& y, const TScalarVector<T, _Dim>& x);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<U, _Dim> Smoothstep(const TScalarVector<T, _Dim>& vmin, const TScalarVector<T, _Dim>& vmax, U f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<U, _Dim> Smoothstep(const TScalarVector<T, _Dim>& vmin, const TScalarVector<T, _Dim>& vmax, const TScalarVector<U, _Dim>& f);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> GridSnap(const TScalarVector<T, _Dim>& location, T grid);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> GridSnap(const TScalarVector<T, _Dim>& location, const TScalarVector<T, _Dim>& grid);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNormalized(const TScalarVector<T, _Dim>& v, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool NearlyEquals(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b, float maxRelDiff = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 2> SinCos(T angle);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
size_t BiggestComponent2(const TScalarVector<T, 2>& v);
//----------------------------------------------------------------------------
template <typename T>
size_t BiggestComponent3(const TScalarVector<T, 3>& v);
//----------------------------------------------------------------------------
template <typename T>
size_t BiggestComponent4(const TScalarVector<T, 4>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u32, _Dim> FalseMask();
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u32, _Dim> TrueMask();
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u32, _Dim> BitAndMask(const TScalarVector<u32, _Dim>& lhs, const TScalarVector<u32, _Dim>& rhs);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u32, _Dim> BitOrMask(const TScalarVector<u32, _Dim>& lhs, const TScalarVector<u32, _Dim>& rhs);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u32, _Dim> BitXorMask(const TScalarVector<u32, _Dim>& lhs, const TScalarVector<u32, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> EqualMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> NotEqualMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> LessMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> LessEqualMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> GreaterMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> GreaterEqualMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> SelectMask(const TScalarVector<u32, _Dim>& mask, const TScalarVector<T, _Dim>& ifTrue, const TScalarVector<T, _Dim>& ifFalse);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsINF(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNAN(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNANorINF(const TScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte0255_to_Float01(const TScalarVector<u8, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte0255_to_FloatM11(const TScalarVector<u8, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u8, _Dim> Float01_to_UByte0255(const TScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u8, _Dim> FloatM11_to_UByte0255(const TScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UShort065535_to_Float01(const TScalarVector<u16, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte065535_to_FloatM11(const TScalarVector<u16, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u16, _Dim> Float01_to_UShort065535(const TScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u16, _Dim> FloatM11_to_UShort065535(const TScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
FORCE_INLINE TScalarVector<u8, 4> Float3M11_to_UByte4N(const TScalarVector<float, 3>& value);
//----------------------------------------------------------------------------
FORCE_INLINE TScalarVector<float, 3> UByte4N_to_Float3M11(const TScalarVector<u8, 4>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> Float01_to_FloatM11(const TScalarVector<float, _Dim>& v_01);
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> FloatM11_to_Float01(const TScalarVector<float, _Dim>& v_M11);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void SwapEndiannessInPlace(TScalarVector<T, _Dim>* value) {
    forrange(i, 0, _Dim)
        FPlatformEndian::SwapInPlace(&value->_data[i]);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/ScalarVectorHelpers-inl.h"
