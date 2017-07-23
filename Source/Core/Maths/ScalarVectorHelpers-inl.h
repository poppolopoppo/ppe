#pragma once

#include "Core/Maths/ScalarVectorHelpers.h"

#pragma warning(push)
#pragma warning(disable: 6201) // L'index 'XXX' est en dehors de la plage d'index valide 'XXX' à 'XXX' pour la mémoire tampon 'XXX' allouée sans doute par la pile.
#pragma warning(disable: 6294) // Boucle mal définie : la condition initiale ne satisfait pas les tests. Le corps de la boucle n'est pas exécuté.


namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Dot(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    T result(0);
    for (size_t i = 0; i < _Dim; ++i)
        result += lhs._data[i] * rhs._data[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Dot2(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    STATIC_ASSERT(2 <= _Dim);
    return lhs.x()*rhs.x() + lhs.y()*rhs.y();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Dot3(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    STATIC_ASSERT(3 <= _Dim);
    return lhs.x()*rhs.x() + lhs.y()*rhs.y() + lhs.z()*rhs.z();
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Dot4(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    STATIC_ASSERT(4 <= _Dim);
    return lhs.x()*rhs.x() + lhs.y()*rhs.y() + lhs.z()*rhs.z() + lhs.w()*rhs.w();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T LengthSq(const TScalarVector<T, _Dim>& v) {
    return Dot(v, v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T LengthSq2(const TScalarVector<T, _Dim>& v) {
    return Dot2(v, v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T LengthSq3(const TScalarVector<T, _Dim>& v) {
    return Dot3(v, v);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T LengthSq4(const TScalarVector<T, _Dim>& v) {
    return Dot4(v, v);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Length(const TScalarVector<T, _Dim>& v) {
    return std::sqrt(static_cast<float>(LengthSq(v)));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Length2(const TScalarVector<T, _Dim>& v) {
    return std::sqrt(static_cast<float>(LengthSq2(v)));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Length3(const TScalarVector<T, _Dim>& v) {
    return std::sqrt(static_cast<float>(LengthSq3(v)));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Length4(const TScalarVector<T, _Dim>& v) {
    return std::sqrt(static_cast<float>(LengthSq(v)));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float DistanceSq(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b) {
    return LengthSq(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq2(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b) {
    return LengthSq2(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq3(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b) {
    return LengthSq3(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float DistanceSq4(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b) {
    return LengthSq4(a - b);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
float Distance(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b) {
    return Length(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance2(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b) {
    return Length2(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance3(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b) {
    return Length3(a - b);
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
FORCE_INLINE float Distance4(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b) {
    return Length4(a - b);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Normalize(const TScalarVector<T, _Dim>& v) {
    const float length = Length(v);
    Assert(length > F_SmallEpsilon); // no zero length normalize
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = static_cast<T>(v._data[i] / length);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Normalize2(const TScalarVector<T, _Dim>& v) {
    const float length = Length2(v);
    Assert(length > F_SmallEpsilon); // no zero length normalize
    TScalarVector<T, _Dim> result;
    result.x() = static_cast<T>(v.x() / length);
    result.y() = static_cast<T>(v.y() / length);
    for (size_t i = 2; i < _Dim; ++i)
        result._data[i] = v._data[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Normalize3(const TScalarVector<T, _Dim>& v) {
    const float length = Length3(v);
    Assert(length > F_SmallEpsilon); // no zero length normalize
    TScalarVector<T, _Dim> result;
    result.x() = static_cast<T>(v.x() / length);
    result.y() = static_cast<T>(v.y() / length);
    result.z() = static_cast<T>(v.z() / length);
    for (size_t i = 3; i < _Dim; ++i)
        result._data[i] = v._data[i];
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Normalize4(const TScalarVector<T, _Dim>& v) {
    const float length = Length4(v);
    Assert(length > F_SmallEpsilon); // no zero length normalize
    TScalarVector<T, _Dim> result(
        static_cast<T>(v.x() / length),
        static_cast<T>(v.y() / length),
        static_cast<T>(v.z() / length),
        static_cast<T>(v.w() / length) );
    for (size_t i = 4; i < _Dim; ++i)
        result._data[i] = v._data[i];
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Min(const TScalarVector<T, _Dim>& v) {
    T result = v._data[0];
    forrange(i, 1, _Dim)
        result = Min(v._data[i], result);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
T Max(const TScalarVector<T, _Dim>& v) {
    T result = v._data[0];
    forrange(i, 1, _Dim)
        result = Max(v._data[i], result);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Min(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = std::min(lhs._data[i], rhs._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Max(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = std::max(lhs._data[i], rhs._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
void MinMax(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs,
            TScalarVector<T, _Dim>& min, TScalarVector<T, _Dim>& max) {
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
template <typename T>
T Det(const TScalarVector<T, 2>& lhs, const TScalarVector<T, 2>& rhs) {
    return lhs.x() * rhs.y() - lhs.y() * rhs.x();
}
//----------------------------------------------------------------------------
template <typename T>
float Cross(const TScalarVector<T, 2>& o, const TScalarVector<T, 2>& a, const TScalarVector<T, 2>& b) {
    // Det(a - o, b - o) <=> Twiced signed area of the triangle
    return ( (a.x() - o.x()) * (b.y() - o.y()) ) -
           ( (a.y() - o.y()) * (b.x() - o.x()) );
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> Cross(const TScalarVector<T, 3>& lhs, const TScalarVector<T, 3>& rhs) {
    return TScalarVector<T, 3>(
        lhs.y() * rhs.z() - lhs.z() * rhs.y(),
        lhs.z() * rhs.x() - lhs.x() * rhs.z(),
        lhs.x() * rhs.y() - lhs.y() * rhs.x() );
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Reflect(const TScalarVector<T, _Dim>& incident, const TScalarVector<T, _Dim>& normal) {
    return incident - normal * (2 * Dot(incident, normal));
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Refract(const TScalarVector<T, _Dim>& incident, const TScalarVector<T, _Dim>& normal, T refractionIndex) {
    const T N_dot_I = Dot(normal, incident);
    const T k = 1 - refractionIndex * refractionIndex * (1 - N_dot_I * N_dot_I);
    Assert(k >= 0);
    return incident * refractionIndex - normal * (refractionIndex * N_dot_I + std::sqrt(k));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Abs(const TScalarVector<T, _Dim>& v) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Abs(v._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> BarycentricLerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, const TScalarVector<T, _Dim>& v2, U f0, U f1, U f2) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = BarycentricLerp(v0._data[i], v1._data[i], v2._data[i], f0, f1, f2);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> BarycentricLerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, const TScalarVector<T, _Dim>& v2, const TScalarVector<U, 3>& uvw) {
    return BarycentricLerp(v0, v1, v2, uvw.x(), uvw.y(), uvw.z());
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Ceil(const TScalarVector<T, _Dim>& f) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Ceil(f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Clamp(const TScalarVector<T, _Dim>& value, T vmin, T vmax) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Clamp(value._data[i], vmin, vmax);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Clamp(const TScalarVector<T, _Dim>& value, const TScalarVector<T, _Dim>& vmin, const TScalarVector<T, _Dim>& vmax) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Clamp(value._data[i], vmin._data[i], vmax._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Floor(const TScalarVector<T, _Dim>& f) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Floor(f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> FMod(const TScalarVector<T, _Dim>& f, float m) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = FMod(f._data[i], m);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> FMod(const TScalarVector<T, _Dim>& f, const TScalarVector<T, _Dim>& m) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = FMod(f._data[i], m._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Frac(const TScalarVector<T, _Dim>& f) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Frac(f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> Lerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, U f) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Lerp(v0._data[i], v1._data[i], f);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> Lerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, const TScalarVector<U, _Dim>& f) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Lerp(v0._data[i], v1._data[i], f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<float, _Dim> LinearStep(const TScalarVector<T, _Dim>& value, const TScalarVector<T, _Dim>& vmin, const TScalarVector<T, _Dim>& vmax) {
    TScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = LinearStep(value._data[i], vmin._data[i], vmax._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<float, _Dim> Pow(const TScalarVector<T, _Dim>& value, T n) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Pow(value._data[i], n);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<float, _Dim> Pow(const TScalarVector<T, _Dim>& value, const TScalarVector<T, _Dim>& n) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Pow(value._data[i], n._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Rcp(const TScalarVector<T, _Dim>& f) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Rcp(f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Round(const TScalarVector<T, _Dim>& f) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Round(f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> RSqrt(const TScalarVector<T, _Dim>& f) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = RSqrt(f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> SLerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, U f) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = SLerp(v0._data[i], v1._data[i], f);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<T, _Dim> SLerp(const TScalarVector<T, _Dim>& v0, const TScalarVector<T, _Dim>& v1, const TScalarVector<U, _Dim>& f) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = SLerp(v0._data[i], v1._data[i], f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> Step(const TScalarVector<T, _Dim>& y, const TScalarVector<T, _Dim>& x) {
    TScalarVector<T, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Step(y._data[i], x._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<U, _Dim> Smoothstep(const TScalarVector<T, _Dim>& vmin, const TScalarVector<T, _Dim>& vmax, U f) {
    TScalarVector<U, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Smoothstep(vmin._data[i], vmax._data[i], f);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim, typename U>
TScalarVector<U, _Dim> Smoothstep(const TScalarVector<T, _Dim>& vmin, const TScalarVector<T, _Dim>& vmax, const TScalarVector<U, _Dim>& f) {
    TScalarVector<U, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Smoothstep(vmin._data[i], vmax._data[i], f._data[i]);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> GridSnap(const TScalarVector<T, _Dim>& location, T grid) {
    TScalarVector<U, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = GridSnap(location._data[i], grid);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> GridSnap(const TScalarVector<T, _Dim>& location, const TScalarVector<T, _Dim>& grid) {
    TScalarVector<U, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = GridSnap(location._data[i], grid._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNormalized(const TScalarVector<T, _Dim>& v, float epsilon/* = F_Epsilon */) {
    return std::abs(T(1) - LengthSq(v)) < epsilon;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool NearlyEquals(const TScalarVector<T, _Dim>& a, const TScalarVector<T, _Dim>& b, float maxRelDiff/* = F_Epsilon */) {
    for (size_t i = 0; i < _Dim; ++i) {

        if (not NearlyEquals(a._data[i], b._data[i], maxRelDiff))
            return false;
    }
    return true;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 2> SinCos(T angle) {
    return TScalarVector<T, 2>(std::sin(angle), std::cos(angle));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
size_t BiggestComponent2(const TScalarVector<T, 2>& v) {
    return (v.x() < v.y() ? 1 : 0);
}
//----------------------------------------------------------------------------
template <typename T>
size_t BiggestComponent3(const TScalarVector<T, 3>& v) {
    return (size_t)RoundToInt(CubeMapFaceID(v.x(), v.y(), v.z()) * 0.5f);
}
//----------------------------------------------------------------------------
template <typename T>
size_t BiggestComponent4(const TScalarVector<T, 4>& v) {
    constexpr size_t wIndex = 3;
    const size_t xyzIndex = (size_t)RoundToInt(CubeMapFaceID(v.x(), v.y(), v.z()) * 0.5f);
    const bool wBiggest = Abs(v.w()) > Max3(Abs(v.x()), Abs(v.y()), Abs(v.z()));
    return (wBiggest ? wIndex : xyzIndex);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u32, _Dim> FalseMask() {
    return TScalarVector<u32, _Dim>(0x00000000);
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u32, _Dim> TrueMask() {
    return TScalarVector<u32, _Dim>(0xFFFFFFFF);
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u32, _Dim> BitAndMask(const TScalarVector<u32, _Dim>& lhs, const TScalarVector<u32, _Dim>& rhs) {
    TScalarVector<u32, _Dim> result;
    forrange(i, 0, _Dim)
        result._data[i] = (lhs._data[i] & rhs._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u32, _Dim> BitOrMask(const TScalarVector<u32, _Dim>& lhs, const TScalarVector<u32, _Dim>& rhs) {
    TScalarVector<u32, _Dim> result;
    forrange(i, 0, _Dim)
        result._data[i] = (lhs._data[i] | rhs._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u32, _Dim> BitXorMask(const TScalarVector<u32, _Dim>& lhs, const TScalarVector<u32, _Dim>& rhs) {
    TScalarVector<u32, _Dim> result;
    forrange(i, 0, _Dim)
        result._data[i] = (lhs._data[i] ^ rhs._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> EqualMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    TScalarVector<u32, _Dim> result;
    forrange(i, 0, _Dim)
        result._data[i] = (lhs._data[i] == rhs._data[i] ? 0xFFFFFFFF : 0x00000000);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> NotEqualMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    TScalarVector<u32, _Dim> result;
    forrange(i, 0, _Dim)
        result._data[i] = (lhs._data[i] != rhs._data[i] ? 0xFFFFFFFF : 0x00000000);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> LessMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    TScalarVector<u32, _Dim> result;
    forrange(i, 0, _Dim)
        result._data[i] = (lhs._data[i] < rhs._data[i] ? 0xFFFFFFFF : 0x00000000);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> LessEqualMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    TScalarVector<u32, _Dim> result;
    forrange(i, 0, _Dim)
        result._data[i] = (lhs._data[i] <= rhs._data[i] ? 0xFFFFFFFF : 0x00000000);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> GreaterMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    TScalarVector<u32, _Dim> result;
    forrange(i, 0, _Dim)
        result._data[i] = (lhs._data[i] > rhs._data[i] ? 0xFFFFFFFF : 0x00000000);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<u32, _Dim> GreaterEqualMask(const TScalarVector<T, _Dim>& lhs, const TScalarVector<T, _Dim>& rhs) {
    TScalarVector<u32, _Dim> result;
    forrange(i, 0, _Dim)
        result._data[i] = (lhs._data[i] >= rhs._data[i] ? 0xFFFFFFFF : 0x00000000);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
TScalarVector<T, _Dim> SelectMask(const TScalarVector<u32, _Dim>& mask, const TScalarVector<T, _Dim>& ifTrue, const TScalarVector<T, _Dim>& ifFalse) {
    TScalarVector<T, _Dim> result;
    forrange(i, 0, _Dim)
        result._data[i] = (mask._data[i] ? ifTrue._data[i] : ifFalse._data[i]);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsINF(const TScalarVector<T, _Dim>& v) {
    for (size_t i = 0; i < _Dim; ++i) {
        if (IsINF(v._data[i]))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNAN(const TScalarVector<T, _Dim>& v) {
    for (size_t i = 0; i < _Dim; ++i) {
        if (IsNAN(v._data[i]))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
bool IsNANorINF(const TScalarVector<T, _Dim>& v) {
    for (size_t i = 0; i < _Dim; ++i) {
        if (IsNANorINF(v._data[i]))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte0255_to_Float01(const TScalarVector<u8, _Dim>& value) {
    TScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = UByte0255_to_Float01(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte0255_to_FloatM11(const TScalarVector<u8, _Dim>& value) {
    TScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = UByte0255_to_FloatM11(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u8, _Dim> Float01_to_UByte0255(const TScalarVector<float, _Dim>& value) {
    TScalarVector<u8, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Float01_to_UByte0255(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u8, _Dim> FloatM11_to_UByte0255(const TScalarVector<float, _Dim>& value) {
    TScalarVector<u8, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = FloatM11_to_UByte0255(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UShort065535_to_Float01(const TScalarVector<u16, _Dim>& value) {
    TScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = UShort065535_to_Float01(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<float, _Dim> UByte065535_to_FloatM11(const TScalarVector<u16, _Dim>& value) {
    TScalarVector<float, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = UByte065535_to_FloatM11(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u16, _Dim> Float01_to_UShort065535(const TScalarVector<float, _Dim>& value) {
    TScalarVector<u16, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = Float01_to_UShort065535(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TScalarVector<u16, _Dim> FloatM11_to_UShort065535(const TScalarVector<float, _Dim>& value) {
    TScalarVector<u16, _Dim> result;
    for (size_t i = 0; i < _Dim; ++i)
        result._data[i] = FloatM11_to_UShort065535(value._data[i]);
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
TScalarVector<u8, 4> Float3M11_to_UByte4N(const TScalarVector<float, 3>& value) {
    TScalarVector<u8, 4> result(
        FloatM11_to_UByte0255(value.x()),
        FloatM11_to_UByte0255(value.y()),
        FloatM11_to_UByte0255(value.z()),
        0 );
    return result;
}
//----------------------------------------------------------------------------
TScalarVector<float, 3> UByte4N_to_Float3M11(const TScalarVector<u8, 4>& value) {
    TScalarVector<float, 3> result(
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
