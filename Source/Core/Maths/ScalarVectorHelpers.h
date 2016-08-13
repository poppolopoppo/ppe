#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/MathHelpers.h"
#include "Core/Maths/PackingHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Dot(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T Dot2(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T Dot3(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T Dot4(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T LengthSq(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T LengthSq2(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T LengthSq3(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE T LengthSq4(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Length(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Length2(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Length3(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Length4(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float DistanceSq(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq2(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq3(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq4(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Distance(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance2(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance3(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance4(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Normalize(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE ScalarVector<T, _Dim> Normalize2(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE ScalarVector<T, _Dim> Normalize3(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE ScalarVector<T, _Dim> Normalize4(const ScalarVector<T, _Dim>& v);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Min(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Max(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void MinMax(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs,
            ScalarVector<T, _Dim>& min, ScalarVector<T, _Dim>& max);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Det(const ScalarVector<T, 2>& lhs, const ScalarVector<T, 2>& rhs);
//----------------------------------------------------------------------------
template <typename T>
float Cross(const ScalarVector<T, 2>& o, const ScalarVector<T, 2>& a, const ScalarVector<T, 2>& b);
//----------------------------------------------------------------------------
template <typename T>
ScalarVector<T, 3> Cross(const ScalarVector<T, 3>& lhs, const ScalarVector<T, 3>& rhs);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Reflect(const ScalarVector<T, _Dim>& incident, const ScalarVector<T, _Dim>& normal);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Refract(const ScalarVector<T, _Dim>& incident, const ScalarVector<T, _Dim>& normal, T refractionIndex);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Abs(const ScalarVector<T, _Dim>& value);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<T, _Dim> BarycentricLerp(const ScalarVector<T, _Dim>& v0, const ScalarVector<T, _Dim>& v1, const ScalarVector<T, _Dim>& v2, U f0, U f1, U f2);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<T, _Dim> BarycentricLerp(const ScalarVector<T, _Dim>& v0, const ScalarVector<T, _Dim>& v1, const ScalarVector<T, _Dim>& v2, const ScalarVector<U, 3>& uvw);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Clamp(const ScalarVector<T, _Dim>& value, T vmin, T vmax);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Clamp(const ScalarVector<T, _Dim>& value, const ScalarVector<T, _Dim>& vmin, const ScalarVector<T, _Dim>& vmax);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Frac(const ScalarVector<T, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<T, _Dim> Lerp(const ScalarVector<T, _Dim>& v0, const ScalarVector<T, _Dim>& v1, U f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<T, _Dim> Lerp(const ScalarVector<T, _Dim>& v0, const ScalarVector<T, _Dim>& v1, const ScalarVector<U, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<float, _Dim> LinearStep(const ScalarVector<T, _Dim>& value, const ScalarVector<T, _Dim>& vmin, const ScalarVector<T, _Dim>& vmax);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<float, _Dim> Pow(const ScalarVector<T, _Dim>& value, T n);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<float, _Dim> Pow(const ScalarVector<T, _Dim>& value, const ScalarVector<T, _Dim>& n);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Rcp(const ScalarVector<T, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> RSqrt(const ScalarVector<T, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Saturate(const ScalarVector<T, _Dim>& value) { return Clamp(value, T(0), T(1)); }
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<T, _Dim> SLerp(const ScalarVector<T, _Dim>& v0, const ScalarVector<T, _Dim>& v1, U f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<T, _Dim> SLerp(const ScalarVector<T, _Dim>& v0, const ScalarVector<T, _Dim>& v1, const ScalarVector<U, _Dim>& f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Step(const ScalarVector<T, _Dim>& y, const ScalarVector<T, _Dim>& x);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<U, _Dim> Smoothstep(const ScalarVector<T, _Dim>& vmin, const ScalarVector<T, _Dim>& vmax, U f);
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<U, _Dim> Smoothstep(const ScalarVector<T, _Dim>& vmin, const ScalarVector<T, _Dim>& vmax, const ScalarVector<U, _Dim>& f);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNormalized(const ScalarVector<T, _Dim>& v, float epsilon = F_Epsilon);
//----------------------------------------------------------------------------
template <typename T>
ScalarVector<T, 2> SinCos(T angle);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> UByte0255_to_Float01(const ScalarVector<u8, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> UByte0255_to_FloatM11(const ScalarVector<u8, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u8, _Dim> Float01_to_UByte0255(const ScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u8, _Dim> FloatM11_to_UByte0255(const ScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> UShort065535_to_Float01(const ScalarVector<u16, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> UByte065535_to_FloatM11(const ScalarVector<u16, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u16, _Dim> Float01_to_UShort065535(const ScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u16, _Dim> FloatM11_to_UShort065535(const ScalarVector<float, _Dim>& value);
//----------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE ScalarVector<u8, 4> Float3M11_to_UByte4N(const ScalarVector<float, 3>& value);
//----------------------------------------------------------------------------
FORCE_INLINE ScalarVector<float, 3> UByte4N_to_Float3M11(const ScalarVector<u8, 4>& value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/ScalarVectorHelpers-inl.h"
