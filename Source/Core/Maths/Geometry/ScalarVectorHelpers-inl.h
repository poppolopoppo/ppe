#pragma once

#include "Core/Maths/Geometry/ScalarVectorHelpers.h"

#pragma warning(push)
#pragma warning(disable: 6201) // L'index 'XXX' est en dehors de la plage d'index valide 'XXX' à 'XXX' pour la mémoire tampon 'XXX' allouée sans doute par la pile.
#pragma warning(disable: 6294) // Boucle mal définie : la condition initiale ne satisfait pas les tests. Le corps de la boucle n'est pas exécuté.


namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Dot(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs) {
    T result(0);
    for (size_t i = 0; i < _Dim; ++i)
        result += lhs._data[i] * rhs._data[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Dot2(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs) {
    STATIC_ASSERT(2 <= _Dim);
    return lhs.x()*rhs.x() + lhs.y()*rhs.y();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Dot3(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs) {
    STATIC_ASSERT(3 <= _Dim);
    return lhs.x()*rhs.x() + lhs.y()*rhs.y() + lhs.z()*rhs.z();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Dot4(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs) {
    STATIC_ASSERT(4 <= _Dim);
    return lhs.x()*rhs.x() + lhs.y()*rhs.y() + lhs.z()*rhs.z() + lhs.w()*rhs.w();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T LengthSq(const ScalarVector<T, _Dim>& v) {
    return Dot(v, v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T LengthSq2(const ScalarVector<T, _Dim>& v) {
    return Dot2(v, v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T LengthSq3(const ScalarVector<T, _Dim>& v) {
    return Dot3(v, v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T LengthSq4(const ScalarVector<T, _Dim>& v) {
    return Dot4(v, v);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Length(const ScalarVector<T, _Dim>& v) {
    return std::sqrt(static_cast<float>(LengthSq(v)));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Length2(const ScalarVector<T, _Dim>& v) {
    return std::sqrt(static_cast<float>(LengthSq2(v)));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Length3(const ScalarVector<T, _Dim>& v) {
    return std::sqrt(static_cast<float>(LengthSq3(v)));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Length4(const ScalarVector<T, _Dim>& v) {
    return std::sqrt(static_cast<float>(LengthSq(v)));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float DistanceSq(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b) {
    return LengthSq(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq2(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b) {
    return LengthSq2(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq3(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b) {
    return LengthSq3(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq4(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b) {
    return LengthSq4(a - b);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Distance(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b) {
    return Length(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance2(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b) {
    return Length2(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance3(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b) {
    return Length3(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance4(const ScalarVector<T, _Dim>& a, const ScalarVector<T, _Dim>& b) {
    return Length4(a - b);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Normalize(const ScalarVector<T, _Dim>& v) {
    const float length = Length(v);
    Assert(length > 0); // no zero length normalize
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = static_cast<T>(v._data[i] / length);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Normalize2(const ScalarVector<T, _Dim>& v) {
    const float length = Length2(v);
    Assert(length > 0); // no zero length normalize
    ScalarVector<T, _Dim> result;
    result.x() = static_cast<T>(v.x() / length);
    result.y() = static_cast<T>(v.y() / length);
    for (size_t i = 2; i < _Dim; ++i)
        result._data[i] = v._data[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Normalize3(const ScalarVector<T, _Dim>& v) {
    const float length = Length3(v);
    Assert(length > 0); // no zero length normalize
    ScalarVector<T, _Dim> result;
    result.x() = static_cast<T>(v.x() / length);
    result.y() = static_cast<T>(v.y() / length);
    result.z() = static_cast<T>(v.z() / length);
    for (size_t i = 3; i < _Dim; ++i)
        result._data[i] = v._data[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Normalize4(const ScalarVector<T, _Dim>& v) {
    const float length = Length4(v);
    Assert(length > 0); // no zero length normalize
    ScalarVector<T, _Dim> result;
    result.x() = static_cast<T>(v.x() / length);
    result.y() = static_cast<T>(v.y() / length);
    result.z() = static_cast<T>(v.z() / length);
    result.w() = static_cast<T>(v.w() / length);
    for (size_t i = 4; i < _Dim; ++i)
        result._data[i] = v._data[i];
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Min(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = std::min(lhs._data[i], rhs._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Max(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = std::max(lhs._data[i], rhs._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void MinMax(const ScalarVector<T, _Dim>& lhs, const ScalarVector<T, _Dim>& rhs,
            ScalarVector<T, _Dim>& min, ScalarVector<T, _Dim>& max) {
    for (size_t i = 0; i < _Dim; ++i) {
        T mi = lhs._data[i];
        T ma = rhs._data[i];
        if (ma < mi)
            std::swap(mi, ma);

        min._data[i] = mi;
        max._data[i] = ma;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Abs(const ScalarVector<T, _Dim>& v) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = std::abs(v._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T>
T Det(const ScalarVector<T, 2>& lhs, const ScalarVector<T, 2>& rhs) {
    return lhs.x()*rhs.y() - lhs.y()*rhs.x();
}
//----------------------------------------------------------------------------
template <typename T>
ScalarVector<T, 3> Cross(const ScalarVector<T, 3>& lhs, const ScalarVector<T, 3>& rhs) {
    return ScalarVector<T, 3>(
        lhs.y() * rhs.z() - lhs.z() * rhs.y(),
        lhs.z() * rhs.x() - lhs.x() * rhs.z(),
        lhs.x() * rhs.y() - lhs.y() * rhs.x() );
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Reflect(const ScalarVector<T, _Dim>& incident, const ScalarVector<T, _Dim>& normal) {
    return incident - normal * (2 * Dot(incident, normal));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Refract(const ScalarVector<T, _Dim>& incident, const ScalarVector<T, _Dim>& normal, T refractionIndex) {
    const T N_dot_I = Dot(normal, incident);
    const T k = 1 - refractionIndex * refractionIndex * (1 - N_dot_I * N_dot_I);
    Assert(k >= 0);
    return incident * refractionIndex - normal * (refractionIndex * N_dot_I + std::sqrt(k));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Clamp(const ScalarVector<T, _Dim>& value, T vmin, T vmax) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Clamp(value._data[i], vmin, vmax);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Clamp(const ScalarVector<T, _Dim>& value, const ScalarVector<T, _Dim>& vmin, const ScalarVector<T, _Dim>& vmax) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Clamp(value._data[i], vmin._data[i], vmax._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Frac(const ScalarVector<T, _Dim>& f) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Frac(f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<T, _Dim> Lerp(const ScalarVector<T, _Dim>& v0, const ScalarVector<T, _Dim>& v1, U f) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Lerp(v0._data[i], v1._data[i], f);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<T, _Dim> Lerp(const ScalarVector<T, _Dim>& v0, const ScalarVector<T, _Dim>& v1, const ScalarVector<U, _Dim>& f) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Lerp(v0._data[i], v1._data[i], f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<float, _Dim> LinearStep(const ScalarVector<T, _Dim>& value, const ScalarVector<T, _Dim>& vmin, const ScalarVector<T, _Dim>& vmax) {
    ScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = LinearStep(value._data[i], vmin._data[i], vmax._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Rcp(const ScalarVector<T, _Dim>& f) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Rcp(f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> RSqrt(const ScalarVector<T, _Dim>& f) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = RSqrt(f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<T, _Dim> SLerp(const ScalarVector<T, _Dim>& v0, const ScalarVector<T, _Dim>& v1, U f) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = SLerp(v0._data[i], v1._data[i], f);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<T, _Dim> SLerp(const ScalarVector<T, _Dim>& v0, const ScalarVector<T, _Dim>& v1, const ScalarVector<U, _Dim>& f) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = SLerp(v0._data[i], v1._data[i], f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
ScalarVector<T, _Dim> Step(const ScalarVector<T, _Dim>& y, const ScalarVector<T, _Dim>& x) {
    ScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Step(y._data[i], x._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<U, _Dim> Smoothstep(const ScalarVector<T, _Dim>& vmin, const ScalarVector<T, _Dim>& vmax, U f) {
    ScalarVector<U, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Smoothstep(vmin._data[i], vmax._data[i], f);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
ScalarVector<U, _Dim> Smoothstep(const ScalarVector<T, _Dim>& vmin, const ScalarVector<T, _Dim>& vmax, const ScalarVector<U, _Dim>& f) {
    ScalarVector<U, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Smoothstep(vmin._data[i], vmax._data[i], f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNormalized(const ScalarVector<T, _Dim>& v, float epsilon/* = F_Epsilon */) {
    return std::abs(T(1) - LengthSq(v)) < epsilon;
}
//----------------------------------------------------------------------------
template <typename T>
ScalarVector<T, 2> SinCos(T angle) {
    return ScalarVector<T, 2>(std::sin(angle), std::cos(angle));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> UByte0255_to_Float01(const ScalarVector<u8, _Dim>& value) {
    ScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = UByte0255_to_Float01(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> UByte0255_to_FloatM11(const ScalarVector<u8, _Dim>& value) {
    ScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = UByte0255_to_FloatM11(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u8, _Dim> Float01_to_UByte0255(const ScalarVector<float, _Dim>& value) {
    ScalarVector<u8, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Float01_to_UByte0255(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u8, _Dim> FloatM11_to_UByte0255(const ScalarVector<float, _Dim>& value) {
    ScalarVector<u8, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = FloatM11_to_UByte0255(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> UShort065535_to_Float01(const ScalarVector<u16, _Dim>& value) {
    ScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = UShort065535_to_Float01(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<float, _Dim> UByte065535_to_FloatM11(const ScalarVector<u16, _Dim>& value) {
    ScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = UByte065535_to_FloatM11(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u16, _Dim> Float01_to_UShort065535(const ScalarVector<float, _Dim>& value) {
    ScalarVector<u16, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Float01_to_UShort065535(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
ScalarVector<u16, _Dim> FloatM11_to_UShort065535(const ScalarVector<float, _Dim>& value) {
    ScalarVector<u16, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = FloatM11_to_UShort065535(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
ScalarVector<u8, 4> Float3M11_to_UByte4N(const ScalarVector<float, 3>& value) {
    ScalarVector<u8, 4> result(
        FloatM11_to_UByte0255(value.x()),
        FloatM11_to_UByte0255(value.y()),
        FloatM11_to_UByte0255(value.z()),
        0 );
    return result;
}
//----------------------------------------------------------------------------
ScalarVector<float, 3> UByte4N_to_Float3M11(const ScalarVector<u8, 4>& value) {
    ScalarVector<float, 3> result(
        UByte0255_to_FloatM11(value.x()),
        UByte0255_to_FloatM11(value.y()),
        UByte0255_to_FloatM11(value.z()) );
    result = Normalize3(result);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#pragma warning(pop)
