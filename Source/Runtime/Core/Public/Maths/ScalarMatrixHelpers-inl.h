#pragma once

#include "Maths/ScalarMatrixHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
bool IsHomogeneous(const TScalarMatrix<T, _Width, _Height>& m, float epsilon/* = Epsilon */) {
    for (u32 i = 0; i < _Width - 1; ++i)
        if (std::abs(m.at_(i, _Height - 1)) > epsilon)
            return false;
    return std::abs(T(1) - m.at_(_Width - 1, _Height - 1)) < epsilon;
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
bool IsInversible(const TScalarMatrix<T, _Width, _Height>& m, float epsilon/* = Epsilon */) {
    float norm = 0;
    for (u32 j = 0; j < _Height; ++j)
        for (u32 i = 0; i < _Width; ++i)
            norm += m.at_(j, i) * m.at_(i, j);

    norm = std::sqrt(norm);
    if (norm <= epsilon)
        return false;

    const float epsilon3 = epsilon * epsilon * epsilon;
    Assert(epsilon3 > 0);

    const float det = Det(m);
    return std::abs(det) > epsilon3*norm;
}
//----------------------------------------------------------------------------
template <typename T>
bool IsInvertible(const TScalarMatrix<T, 4, 4>& m) {
    return IsInvertible(m.template Crop<3, 3>());
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
bool IsOrthogonal(const TScalarMatrix<T, _Width, _Height>& m, float epsilon/* = Epsilon */) {
    for(u32 j = 0; j < _Width; ++j)
        for(u32 i = 0; i < _Width; ++i)
            if (i != j) {
                const auto ca = m.Column(i);
                const auto cb = m.Column(j);
                const T d = Dot(ca, cb);
                const T lenSumSq = LengthSq(ca) + LengthSq(cb);
                const T leftPart = d*d;
                const T rightPart = Sqr(epsilon)*lenSumSq;
                if (leftPart > rightPart)
                    return false;
            }
    return true;
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
bool IsOrthonormal(const TScalarMatrix<T, _Width, _Height>& m, float epsilon/* = Epsilon */) {
    const T sqrtEpsilon(std::sqrt(epsilon));
    for (u32 i = 0; i < _Width; ++i) {
        const T rowLenMinusOne = std::abs(T(1) - LengthSq(m.Row(i)));
        if (rowLenMinusOne > sqrtEpsilon)
            return false;
    }
    return IsOrthogonal(m, epsilon);
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
bool IsSymetrical(const TScalarMatrix<T, _Width, _Height>& m, float epsilon/* = Epsilon */) {
    for(u32 j=0; j < _Height; ++j)
        for(u32 i=0; i < j; ++i)
            if (not NearlyEquals(m.at_(j, i), m.at_(i, j), epsilon))
                return false;
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
bool IsINF(const TScalarMatrix<T, _Width, _Height>& m) {
    for (T f : m._data.raw) {
        if (IsINF(f))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
bool IsNAN(const TScalarMatrix<T, _Width, _Height>& m) {
    for (T f : m._data.raw) {
        if (IsNAN(f))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
bool IsNANorINF(const TScalarMatrix<T, _Width, _Height>& m) {
    for (T f : m._data.raw) {
        if (IsNANorINF(f))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
bool NearlyEquals(  const TScalarMatrix<T, _Width, _Height>& lhs,
                    const TScalarMatrix<T, _Width, _Height>& rhs,
                    float maxRelDiff/* = Epsilon */) {
    forrange(i, 0, lengthof(lhs._data.raw))
        if (not NearlyEquals(lhs._data.raw[i], rhs._data.raw[i], maxRelDiff))
            return false;
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
TScalarMatrix<T, _Width, _Height> Abs(const TScalarMatrix<T, _Width, _Height>& m) {
    TScalarMatrix<T, _Width, _Height> result;
    for (u32 i = 0; i < TScalarMatrix<T, _Width, _Height>::Dim; ++i)
        result._data.raw[i] = std::abs(m._data.raw[i]);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
TScalarMatrix<T, _Width, _Height> OuterProduct(const TScalarVector<T, _Height>& c,
                                               const TScalarVector<T, _Width>& r) {
    TScalarMatrix<T, 1, _Height> col;
    col.SetColumn(0, c);
    TScalarMatrix<T, _Width, 1> row;
    row.SetRow(0, r);
    return (col.Multiply(row));
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
void Lerp(  const TScalarMatrix<T, _Width, _Height>& v0,
            const TScalarMatrix<T, _Width, _Height>& v1,
            float f,
            TScalarMatrix<T, _Width, _Height>& result) {
    // TODO : this has probably no sense :'(
    for (u32 i = 0; i < TScalarMatrix<T, _Width, _Height>::Dim; ++i)
        result._data.raw[i] = Lerp(v0._data.raw[i], v1._data.raw[i], f);
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
TScalarMatrix<T, _Width, _Height> Lerp(
    const TScalarMatrix<T, _Width, _Height>& v0,
    const TScalarMatrix<T, _Width, _Height>& v1,
    float f ) {
    // TODO : this has probably no sense :'(
    TScalarMatrix<T, _Width, _Height> result;
    Lerp(v0, v1, f, result);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
void SLerp( const TScalarMatrix<T, _Width, _Height>& v0,
            const TScalarMatrix<T, _Width, _Height>& v1,
            float f,
            TScalarMatrix<T, _Width, _Height>& result) {
    // TODO : this has probably no sense :'(
    for (u32 i = 0; i < TScalarMatrix<T, _Width, _Height>::Dim; ++i)
        result._data.raw[i] = SLerp(v0._data.raw[i], v1._data.raw[i], f);
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
void Smoothstep(const TScalarMatrix<T, _Width, _Height>& v0,
                const TScalarMatrix<T, _Width, _Height>& v1,
                float f,
                TScalarMatrix<T, _Width, _Height>& result) {
    // TODO : this has probably no sense :'(
    for (u32 i = 0; i < TScalarMatrix<T, _Width, _Height>::Dim; ++i)
        result._data.raw[i] = Smoothstep(v0._data.raw[i], v1._data.raw[i], f);
}
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
void Smootherstep(  const TScalarMatrix<T, _Width, _Height>& v0,
                    const TScalarMatrix<T, _Width, _Height>& v1,
                    float f,
                    TScalarMatrix<T, _Width, _Height>& result) {
    // TODO : this has probably no sense :'(
    for (u32 i = 0; i < TScalarMatrix<T, _Width, _Height>::Dim; ++i)
        result._data.raw[i] = Smootherstep(v0._data.raw[i], v1._data.raw[i], f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Orthogonalize(const TScalarMatrix<T, 4, 4>& m) {
    //Uses the modified Gram-Schmidt process.
    //q1 = m1
    //q2 = m2 - ((q1 * m2) / (q1 * q1)) * q1
    //q3 = m3 - ((q1 * m3) / (q1 * q1)) * q1 - ((q2 * m3) / (q2 * q2)) * q2
    //q4 = m4 - ((q1 * m4) / (q1 * q1)) * q1 - ((q2 * m4) / (q2 * q2)) * q2 - ((q3 * m4) / (q3 * q3)) * q3

    //By separating the above algorithm into multiple lines, we actually increase accuracy.
    TScalarMatrix<T, 4, 4> result(m);

    result.SetRow_y(result.Row_y() - (Dot4(result.Row_x(), result.Row_y()) / Dot4(result.Row_x(), result.Row_x())) * result.Row_x() );

    result.SetRow_z(result.Row_z() - (Dot4(result.Row_x(), result.Row_z()) / Dot4(result.Row_x(), result.Row_x())) * result.Row_x() );
    result.SetRow_z(result.Row_z() - (Dot4(result.Row_y(), result.Row_z()) / Dot4(result.Row_y(), result.Row_y())) * result.Row_y() );

    result.SetRow_w(result.Row_w() - (Dot4(result.Row_x(), result.Row_w()) / Dot4(result.Row_x(), result.Row_x())) * result.Row_x() );
    result.SetRow_w(result.Row_w() - (Dot4(result.Row_y(), result.Row_w()) / Dot4(result.Row_y(), result.Row_y())) * result.Row_y() );
    result.SetRow_w(result.Row_w() - (Dot4(result.Row_z(), result.Row_w()) / Dot4(result.Row_z(), result.Row_z())) * result.Row_z() );

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Orthonormalize(const TScalarMatrix<T, 4, 4>& m) {
    //Uses the modified Gram-Schmidt process.
    //Because we are making unit vectors, we can optimize the math for orthogonalization
    //and simplify the projection operation to remove the division.
    //q1 = m1 / |m1|
    //q2 = (m2 - (q1 * m2) * q1) / |m2 - (q1 * m2) * q1|
    //q3 = (m3 - (q1 * m3) * q1 - (q2 * m3) * q2) / |m3 - (q1 * m3) * q1 - (q2 * m3) * q2|
    //q4 = (m4 - (q1 * m4) * q1 - (q2 * m4) * q2 - (q3 * m4) * q3) / |m4 - (q1 * m4) * q1 - (q2 * m4) * q2 - (q3 * m4) * q3|

    //By separating the above algorithm into multiple lines, we actually increase accuracy.
    TScalarMatrix<T, 4, 4> result(m);

    result.SetRow_x(Normalize4(result.Row_x()) );

    result.SetRow_y(result.Row_y() - Dot4(result.Row_x(), result.Row_y()) * result.Row_x() );
    result.SetRow_y(Normalize4(result.Row_y()) );

    result.SetRow_z(result.Row_z() - Dot4(result.Row_x(), result.Row_z()) * result.Row_x() );
    result.SetRow_z(result.Row_z() - Dot4(result.Row_y(), result.Row_z()) * result.Row_y() );
    result.SetRow_z(Normalize4(result.Row_z()) );

    result.SetRow_w(result.Row_w() - Dot4(result.Row_x(), result.Row_w()) * result.Row_x() );
    result.SetRow_w(result.Row_w() - Dot4(result.Row_y(), result.Row_w()) * result.Row_y() );
    result.SetRow_w(result.Row_w() - Dot4(result.Row_z(), result.Row_w()) * result.Row_z() );
    result.SetRow_w(Normalize4(result.Row_w()));

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void DecomposeQR(const TScalarMatrix<T, 4, 4>& m, TScalarMatrix<T, 4, 4>& q, TScalarMatrix<T, 4, 4>& r) {
    // Decomposes a matrix into an ortho-normalized matrix Q and a right triangular matrix R.

    q = Orthonormalize(m.Transpose()).Transpose();

    r.SetBroadcast(0);

    r._11() = Dot4(q.Column_x(), m.Column_x());
    r._12() = Dot4(q.Column_x(), m.Column_y());
    r._13() = Dot4(q.Column_x(), m.Column_z());
    r._14() = Dot4(q.Column_x(), m.Column_w());

    r._22() = Dot4(q.Column_y(), m.Column_y());
    r._23() = Dot4(q.Column_y(), m.Column_z());
    r._24() = Dot4(q.Column_y(), m.Column_w());

    r._33() = Dot4(q.Column_z(), m.Column_z());
    r._34() = Dot4(q.Column_z(), m.Column_w());

    r._44() = Dot4(q.Column_w(), m.Column_w());
}
//----------------------------------------------------------------------------
template <typename T>
void DecomposeLQ(const TScalarMatrix<T, 4, 4>& m, TScalarMatrix<T, 4, 4>& l, TScalarMatrix<T, 4, 4>& q) {
    // Decomposes a matrix into a lower triangular matrix L and an ortho-normalized matrix Q.

    q = Orthonormalize(m);

    l.SetBroadcast(0);

    l._11() = Dot4(q.Row_x(), m.Row_x());

    l._21() = Dot4(q.Row_x(), m.Row_y());
    l._22() = Dot4(q.Row_y(), m.Row_y());

    l._31() = Dot4(q.Row_x(), m.Row_z());
    l._32() = Dot4(q.Row_y(), m.Row_z());
    l._33() = Dot4(q.Row_z(), m.Row_z());

    l._41() = Dot4(q.Row_x(), m.Row_w());
    l._42() = Dot4(q.Row_y(), m.Row_w());
    l._43() = Dot4(q.Row_z(), m.Row_w());
    l._44() = Dot4(q.Row_w(), m.Row_w());
}
//----------------------------------------------------------------------------
template <typename T>
void Decompose( const TScalarMatrix<T, 4, 4>& transform,
                TScalarVector<T, 3>& scale,
                FQuaternion& rotation,
                TScalarVector<T, 3>& translation ) {
    //Source: Unknown
    //References: http://www.gamedev.net/community/forums/topic.asp?topic_id=441695

    //Get the translation.
    translation.x = transform._41();
    translation.y = transform._42();
    translation.z = transform._43();

    //Scaling is the length of the rows.
    scale.x = std::sqrt((transform._11() * transform._11()) + (transform._12() * transform._12()) + (transform._13() * transform._13()));
    scale.y = std::sqrt((transform._21() * transform._21()) + (transform._22() * transform._22()) + (transform._23() * transform._23()));
    scale.z = std::sqrt((transform._31() * transform._31()) + (transform._32() * transform._32()) + (transform._33() * transform._33()));

    //If any of the scaling factors are zero, than the rotation matrix can not exist.
    Assert( std::abs(scale.x) > Epsilon &&
            std::abs(scale.y) > Epsilon &&
            std::abs(scale.z) > Epsilon );

    //The rotation is the left over matrix after dividing out the scaling.
    TScalarMatrix<T, 4, 4> rotationmatrix(0);
    rotationmatrix._11() = transform._11() / scale.x;
    rotationmatrix._12() = transform._12() / scale.x;
    rotationmatrix._13() = transform._13() / scale.x;

    rotationmatrix._21() = transform._21() / scale.y;
    rotationmatrix._22() = transform._22() / scale.y;
    rotationmatrix._23() = transform._23() / scale.y;

    rotationmatrix._31() = transform._31() / scale.z;
    rotationmatrix._32() = transform._32() / scale.z;
    rotationmatrix._33() = transform._33() / scale.z;

    rotationmatrix._44() = 1.0f;

    rotation = MakeQuaternionFromRotationMatrix(rotationmatrix);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Det(const TScalarMatrix<T, 2, 2>& m) {
    return  m.m00() * m.m11() - m.m10() * m.m01();
}
//----------------------------------------------------------------------------
template <typename T>
T Det(const TScalarMatrix<T, 3, 3>& m) {
    return  m.m00() * ( m.m11() * m.m22() - m.m12() * m.m21() ) -
            m.m01() * ( m.m10() * m.m22() - m.m12() * m.m20() ) +
            m.m02() * ( m.m10() * m.m21() - m.m11() * m.m20() );
}
//----------------------------------------------------------------------------
template <typename T>
T Det(const TScalarMatrix<T, 4, 4>& m) {
    TScalarMatrix<T, 4, 4> tmp;

    tmp._11() = +(m._22() * (m._33() * m._44() - m._34() * m._43()) -
        m._32() * (m._23() * m._44() - m._24() * m._43()) +
        m._42() * (m._23() * m._34() - m._24() * m._33()));
    tmp._12() = -(m._21() * (m._33() * m._44() - m._34() * m._43()) -
        m._31() * (m._23() * m._44() - m._24() * m._43()) +
        m._41() * (m._23() * m._34() - m._24() * m._33()));
    tmp._13() = +(m._21() * (m._32() * m._44() - m._34() * m._42()) -
        m._31() * (m._22() * m._44() - m._24() * m._42()) +
        m._41() * (m._22() * m._34() - m._24() * m._32()));
    tmp._14() = -(m._21() * (m._32() * m._43() - m._33() * m._42()) -
        m._31() * (m._22() * m._43() - m._23() * m._42()) +
        m._41() * (m._22() * m._33() - m._23() * m._32()));

    tmp._21() = -(m._12() * (m._33() * m._44() - m._34() * m._43()) -
        m._32() * (m._13() * m._44() - m._14() * m._43()) +
        m._42() * (m._13() * m._34() - m._14() * m._33()));
    tmp._22() = +(m._11() * (m._33() * m._44() - m._34() * m._43()) -
        m._31() * (m._13() * m._44() - m._14() * m._43()) +
        m._41() * (m._13() * m._34() - m._14() * m._33()));
    tmp._23() = -(m._11() * (m._32() * m._44() - m._34() * m._42()) -
        m._31() * (m._12() * m._44() - m._14() * m._42()) +
        m._41() * (m._12() * m._34() - m._14() * m._32()));
    tmp._24() = +(m._11() * (m._32() * m._43() - m._33() * m._42()) -
        m._31() * (m._12() * m._43() - m._13() * m._42()) +
        m._41() * (m._12() * m._33() - m._13() * m._32()));

    tmp._31() = +(m._12() * (m._23() * m._44() - m._24() * m._43()) -
        m._22() * (m._13() * m._44() - m._14() * m._43()) +
        m._42() * (m._13() * m._24() - m._14() * m._23()));
    tmp._32() = -(m._11() * (m._23() * m._44() - m._24() * m._43()) -
        m._21() * (m._13() * m._44() - m._14() * m._43()) +
        m._41() * (m._13() * m._24() - m._14() * m._23()));
    tmp._33() = +(m._11() * (m._22() * m._44() - m._24() * m._42()) -
        m._21() * (m._12() * m._44() - m._14() * m._42()) +
        m._41() * (m._12() * m._24() - m._14() * m._22()));
    tmp._34() = -(m._11() * (m._22() * m._43() - m._23() * m._42()) -
        m._21() * (m._12() * m._43() - m._13() * m._42()) +
        m._41() * (m._12() * m._23() - m._13() * m._22()));

    tmp._41() = -(m._12() * (m._23() * m._34() - m._24() * m._33()) -
        m._22() * (m._13() * m._34() - m._14() * m._33()) +
        m._32() * (m._13() * m._24() - m._14() * m._23()));
    tmp._42() = +(m._11() * (m._23() * m._34() - m._24() * m._33()) -
        m._21() * (m._13() * m._34() - m._14() * m._33()) +
        m._31() * (m._13() * m._24() - m._14() * m._23()));
    tmp._43() = -(m._11() * (m._22() * m._34() - m._24() * m._32()) -
        m._21() * (m._12() * m._34() - m._14() * m._32()) +
        m._31() * (m._12() * m._24() - m._14() * m._22()));
    tmp._44() = +(m._11() * (m._22() * m._33() - m._23() * m._32()) -
        m._21() * (m._12() * m._33() - m._13() * m._32()) +
        m._31() * (m._12() * m._23() - m._13() * m._22()));

    const T d = m._11() * tmp._11() +
                m._21() * tmp._21() +
                m._31() * tmp._31() +
                m._41() * tmp._41();

    return d;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const TScalarMatrix<T, 2, 2>& m) {
    return m.m00() * m.m11() - m.m10() * m.m01();
}
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const TScalarMatrix<T, 3, 3>& m) {
    return m.m00() * m.m11() - m.m10() * m.m01();
}
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const TScalarMatrix<T, 4, 4>& m) {
    return m.m00() * m.m11() - m.m10() * m.m01();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 2, 2> Invert(const TScalarMatrix<T, 2, 2>& m) {
    float det = Det2x2(m);
    Assert(det != 0);
    float ooDet = 1.f/det;

    TScalarMatrix<T, 2, 2> result;
    result.m00() = ooDet * m.m11();
    result.m10() = -ooDet * m.m10();
    result.m01() = -ooDet * m.m01();
    result.m11() = ooDet * m.m00();

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Invert(const TScalarMatrix<T, 3, 3>& m) {
    float det = Det(m);
    Assert(det != 0);
    float ooDet = 1.0f/det;

    TScalarMatrix<T, 3, 3> result;
    result.m00() =  ooDet * ( m.m11() * m.m22() - m.m12() * m.m21() );
    result.m01() = -ooDet * ( m.m01() * m.m22() - m.m02() * m.m21() );
    result.m02() =  ooDet * ( m.m01() * m.m12() - m.m02() * m.m11() );

    result.m10() = -ooDet * ( m.m10() * m.m22() - m.m12() * m.m20() );
    result.m11() =  ooDet * ( m.m00() * m.m22() - m.m02() * m.m20() );
    result.m12() = -ooDet * ( m.m00() * m.m12() - m.m02() * m.m10() );

    result.m20() =  ooDet * ( m.m10() * m.m21() - m.m11() * m.m20() );
    result.m21() = -ooDet * ( m.m00() * m.m21() - m.m01() * m.m20() );
    result.m22() =  ooDet * ( m.m00() * m.m11() - m.m01() * m.m10() );

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Invert(const TScalarMatrix<T, 4, 4>& m) {
    float b0 = (m._31() * m._42()) - (m._32() * m._41());
    float b1 = (m._31() * m._43()) - (m._33() * m._41());
    float b2 = (m._34() * m._41()) - (m._31() * m._44());
    float b3 = (m._32() * m._43()) - (m._33() * m._42());
    float b4 = (m._34() * m._42()) - (m._32() * m._44());
    float b5 = (m._33() * m._44()) - (m._34() * m._43());

    float d11 = m._22() * b5 + m._23() * b4 + m._24() * b3;
    float d12 = m._21() * b5 + m._23() * b2 + m._24() * b1;
    float d13 = m._21() * -b4 + m._22() * b2 + m._24() * b0;
    float d14 = m._21() * b3 + m._22() * -b1 + m._23() * b0;

    float det = m._11() * d11 - m._12() * d12 + m._13() * d13 - m._14() * d14;
    Assert(std::abs(det) > Epsilon);

    det = 1.0f / det;

    float a0 = (m._11() * m._22()) - (m._12() * m._21());
    float a1 = (m._11() * m._23()) - (m._13() * m._21());
    float a2 = (m._14() * m._21()) - (m._11() * m._24());
    float a3 = (m._12() * m._23()) - (m._13() * m._22());
    float a4 = (m._14() * m._22()) - (m._12() * m._24());
    float a5 = (m._13() * m._24()) - (m._14() * m._23());

    float d21 = m._12() * b5 + m._13() * b4 + m._14() * b3;
    float d22 = m._11() * b5 + m._13() * b2 + m._14() * b1;
    float d23 = m._11() * -b4 + m._12() * b2 + m._14() * b0;
    float d24 = m._11() * b3 + m._12() * -b1 + m._13() * b0;

    float d31 = m._42() * a5 + m._43() * a4 + m._44() * a3;
    float d32 = m._41() * a5 + m._43() * a2 + m._44() * a1;
    float d33 = m._41() * -a4 + m._42() * a2 + m._44() * a0;
    float d34 = m._41() * a3 + m._42() * -a1 + m._43() * a0;

    float d41 = m._32() * a5 + m._33() * a4 + m._34() * a3;
    float d42 = m._31() * a5 + m._33() * a2 + m._34() * a1;
    float d43 = m._31() * -a4 + m._32() * a2 + m._34() * a0;
    float d44 = m._31() * a3 + m._32() * -a1 + m._33() * a0;

    TScalarMatrix<T, 4, 4> result;

    result._11() = +d11 * det; result._12() = -d21 * det; result._13() = +d31 * det; result._14() = -d41 * det;
    result._21() = -d12 * det; result._22() = +d22 * det; result._23() = -d32 * det; result._24() = +d42 * det;
    result._31() = +d13 * det; result._32() = -d23 * det; result._33() = +d33 * det; result._34() = -d43 * det;
    result._41() = -d14 * det; result._42() = +d24 * det; result._43() = -d34 * det; result._44() = +d44 * det;

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Invert_AssumeHomogeneous(const TScalarMatrix<T, 4, 4>& m) {
    Assert(IsHomogeneous(m));

    const TScalarVector<T, 4> abc_ = m.Row_x();
    const TScalarVector<T, 4> def_ = m.Row_y();
    const TScalarVector<T, 4> ghi_ = m.Row_z();

    const T det = Det(m);
    AssertRelease(det != 0);

    const T ooDet = TRcp(det);

    const TScalarVector<T, 4> ecb_(def_.y, abc_.z, abc_.y, 0.f);
    const TScalarVector<T, 4> ihf_(ghi_.z, ghi_.y, def_.z, 0.f);

    const TScalarVector<T, 4> fbc_(def_.z, abc_.y, abc_.z, 0.f);
    const TScalarVector<T, 4> hie_(ghi_.y, ghi_.z, def_.y, 0.f);

    const TScalarVector<T, 4> fac_(def_.z, abc_.x, abc_.z, 0.f);
    const TScalarVector<T, 4> gid_(ghi_.x, ghi_.z, def_.x, 0.f);

    const TScalarVector<T, 4> dca_(def_.x, abc_.z, abc_.x, 0.f);
    const TScalarVector<T, 4> igf_(ghi_.z, ghi_.x, def_.z, 0.f);

    const TScalarVector<T, 4> dba_(def_.x, abc_.y, abc_.x, 0.f);
    const TScalarVector<T, 4> hge_(ghi_.y, ghi_.x, def_.y, 0.f);

    const TScalarVector<T, 4> eab_(def_.y, abc_.x, abc_.y, 0.f);
    const TScalarVector<T, 4> ghd_(ghi_.x, ghi_.y, def_.x, 0.f);

    const TScalarVector<T, 4> adjointL0 = ecb_ * ihf_ - fbc_ * hie_;
    const TScalarVector<T, 4> adjointL1 = fac_ * gid_ - dca_ * igf_;
    const TScalarVector<T, 4> adjointL2 = dba_ * hge_ - eab_ * ghd_;

    TScalarVector<T, 4> resultL0 = adjointL0 * ooDet;
    TScalarVector<T, 4> resultL1 = adjointL1 * ooDet;
    TScalarVector<T, 4> resultL2 = adjointL2 * ooDet;

    const TScalarVector<T, 4> minus_t = -m.AxisT().ZeroExtend();
    const T newTx = Dot3(resultL0, minus_t);
    const T newTy = Dot3(resultL1, minus_t);
    const T newTz = Dot3(resultL2, minus_t);

    resultL0.w = newTx;
    resultL1.w = newTy;
    resultL2.w = newTz;

    TScalarMatrix<T, 4, 4> result;
    result.SetRow(0, resultL0);
    result.SetRow(1, resultL1);
    result.SetRow(2, resultL2);

    result.m03() = result.m13() = result.m23() = T(0);
    result.m33() = T(1);

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Width, u32 _Height>
TScalarMatrix<T, _Height, _Width> InvertTranspose(const TScalarMatrix<T, _Width, _Height>& m) {
    return Invert(m.Transpose());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeBillboardMatrix( const TScalarVector<T, 3>& objectPosition,
                                            const TScalarVector<T, 3>& cameraPosition,
                                            const TScalarVector<T, 3>& cameraUpVector,
                                            const TScalarVector<T, 3>& cemaraForward) {
    TScalarVector<T, 3> difference = objectPosition - cameraPosition;

    T lengthSq = LengthSq3(difference);
    if (lengthSq < Epsilon)
        difference = -cemaraForward;
    else
        difference /= std::sqrt(lengthSq);

    const TScalarVector<T, 3> crossed = Normalize3(Cross(cameraUpVector, difference));
    const TScalarVector<T, 3> f = Cross(difference, crossed);

    TScalarMatrix<T, 4, 4> result(0);
    result._11() = crossed.x;
    result._12() = crossed.y;
    result._13() = crossed.z;
    result._14() = 0.0f;
    result._21() = f.x;
    result._22() = f.y;
    result._23() = f.z;
    result._24() = 0.0f;
    result._31() = difference.x;
    result._32() = difference.y;
    result._33() = difference.z;
    result._34() = 0.0f;
    result._41() = objectPosition.x;
    result._42() = objectPosition.y;
    result._43() = objectPosition.z;
    result._44() = 1.0f;

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeLookAtLHMatrix(   const TScalarVector<T, 3>& eye,
                                            const TScalarVector<T, 3>& target,
                                            const TScalarVector<T, 3>& up ) {
    TScalarVector<T, 3> zaxis = Normalize(target - eye);
    TScalarVector<T, 3> xaxis = Normalize(Cross(up, zaxis));
    TScalarVector<T, 3> yaxis = Cross(zaxis, xaxis);

    TScalarMatrix<T, 4, 4> result = TScalarMatrix<T, 4, 4>::Identity();
    result._11() = xaxis.x; result._21() = xaxis.y; result._31() = xaxis.z;
    result._12() = yaxis.x; result._22() = yaxis.y; result._32() = yaxis.z;
    result._13() = zaxis.x; result._23() = zaxis.y; result._33() = zaxis.z;

    result._41() = Dot(xaxis, eye);
    result._42() = Dot(yaxis, eye);
    result._43() = Dot(zaxis, eye);

    result._41() = -result._41();
    result._42() = -result._42();
    result._43() = -result._43();

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeLookAtRHMatrix(   const TScalarVector<T, 3>& eye,
                                            const TScalarVector<T, 3>& target,
                                            const TScalarVector<T, 3>& up ) {
    TScalarVector<T, 3> zaxis = Normalize(eye - target);
    TScalarVector<T, 3> xaxis = Normalize(Cross(up, zaxis));
    TScalarVector<T, 3> yaxis = Cross(zaxis, xaxis);

    TScalarMatrix<T, 4, 4> result = TScalarMatrix<T, 4, 4>::Identity();
    result._11() = xaxis.x; result._21() = xaxis.y; result._31() = xaxis.z;
    result._12() = yaxis.x; result._22() = yaxis.y; result._32() = yaxis.z;
    result._13() = zaxis.x; result._23() = zaxis.y; result._33() = zaxis.z;

    result._41() = Dot(xaxis, eye);
    result._42() = Dot(yaxis, eye);
    result._43() = Dot(zaxis, eye);

    result._41() = -result._41();
    result._42() = -result._42();
    result._43() = -result._43();

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeOrthographicLHMatrix( T width,
                                                 T height,
                                                 T znear,
                                                 T zfar ) {
    T halfWidth = width * 0.5f;
    T halfHeight = height * 0.5f;

    return MakeOrthographicOffCenterLHMatrix(-halfWidth, halfWidth, -halfHeight, halfHeight, znear, zfar);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeOrthographicRHMatrix( T width,
                                                 T height,
                                                 T znear,
                                                 T zfar ) {
    T halfWidth = width * 0.5f;
    T halfHeight = height * 0.5f;

    return MakeOrthographicOffCenterRHMatrix(-halfWidth, halfWidth, -halfHeight, halfHeight, znear, zfar);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeOrthographicOffCenterLHMatrix(T left,
                                                         T right,
                                                         T bottom,
                                                         T top,
                                                         T znear,
                                                         T zfar ) {
    T zRange = 1.0f / (zfar - znear);

    TScalarMatrix<T, 4, 4> result = TScalarMatrix<T, 4, 4>::Identity();
    result._11() = 2.0f / (right - left);
    result._22() = 2.0f / (top - bottom);
    result._33() = zRange;
    result._41() = (left + right) / (left - right);
    result._42() = (top + bottom) / (bottom - top);
    result._43() = -znear * zRange;

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeOrthographicOffCenterRHMatrix(T left,
                                                         T right,
                                                         T bottom,
                                                         T top,
                                                         T znear,
                                                         T zfar ) {
    TScalarMatrix<T, 4, 4> result = MakeOrthographicOffCenterLHMatrix(left, right, bottom, top, znear, zfar);
    result._33() *= -1.0f;
    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveLHMatrix( T width,
                                                T height,
                                                T znear,
                                                T zfar ) {
    T halfWidth = width * 0.5f;
    T halfHeight = height * 0.5f;

    return MakePerspectiveOffCenterLHMatrix(-halfWidth, halfWidth, -halfHeight, halfHeight, znear, zfar);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveRHMatrix( T width,
                                                T height,
                                                T znear,
                                                T zfar ) {
    T halfWidth = width * 0.5f;
    T halfHeight = height * 0.5f;

    return MakePerspectiveOffCenterRHMatrix(-halfWidth, halfWidth, -halfHeight, halfHeight, znear, zfar);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveOffCenterLHMatrix(T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar ) {
    T zRange = zfar / (zfar - znear);

    TScalarMatrix<T, 4, 4> result(0);
    result._11() = 2.0f * znear / (right - left);
    result._22() = 2.0f * znear / (top - bottom);
    result._31() = (left + right) / (left - right);
    result._32() = (top + bottom) / (bottom - top);
    result._33() = zRange;
    result._34() = 1.0f;
    result._43() = -znear * zRange;

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveOffCenterRHMatrix(T left,
                                                        T right,
                                                        T bottom,
                                                        T top,
                                                        T znear,
                                                        T zfar ) {
    TScalarMatrix<T, 4, 4> result = MakePerspectiveOffCenterLHMatrix(left, right, bottom, top, znear, zfar);
    result._31() *= -1.0f;
    result._32() *= -1.0f;
    result._33() *= -1.0f;
    result._34() *= -1.0f;
    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveFovLHMatrix(  T fov,
                                                    T aspect,
                                                    T znear,
                                                    T zfar ) {
    T yScale = 1.0f / std::tan(fov * 0.5f);
    T xScale = yScale / aspect;

    T halfWidth = znear / xScale;
    T halfHeight = znear / yScale;

    return MakePerspectiveOffCenterLHMatrix(-halfWidth, halfWidth, -halfHeight, halfHeight, znear, zfar);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakePerspectiveFovRHMatrix(  T fov,
                                                    T aspect,
                                                    T znear,
                                                    T zfar ) {
    T yScale = 1.0f / std::tan(fov * 0.5f);
    T xScale = yScale / aspect;

    T halfWidth = znear / xScale;
    T halfHeight = znear / yScale;

    return MakePerspectiveOffCenterRHMatrix(-halfWidth, halfWidth, -halfHeight, halfHeight, znear, zfar);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> MakeReflectionMatrix(const FPlane& plane) {
    T x = plane.Normal().x;
    T y = plane.Normal().y;
    T z = plane.Normal().z;
    T x2 = -2.0f * x;
    T y2 = -2.0f * y;
    T z2 = -2.0f * z;

    TScalarMatrix<T, 4, 4> result;
    result._11() = (x2 * x) + 1.0f;
    result._12() = y2 * x;
    result._13() = z2 * x;
    result._14() = 0.0f;
    result._21() = x2 * y;
    result._22() = (y2 * y) + 1.0f;
    result._23() = z2 * y;
    result._24() = 0.0f;
    result._31() = x2 * z;
    result._32() = y2 * z;
    result._33() = (z2 * z) + 1.0f;
    result._34() = 0.0f;
    result._41() = x2 * plane.D();
    result._42() = y2 * plane.D();
    result._43() = z2 * plane.D();
    result._44() = 1.0f;

    return result;
}
//----------------------------------------------------------------------------
// light : The light direction. If the W component is 0, the light is directional light; if the
/// W component is 1, the light is a point light.
template <typename T>
TScalarMatrix<T, 4, 4> MakeShadowMatrix(const TScalarVector<T, 4>& light, const FPlane& plane) {
    T dot = (plane.Normal().x * light.x) + (plane.Normal().y * light.y) + (plane.Normal().z * light.z) + (plane.D() * light.w);
    T x = -plane.Normal().x;
    T y = -plane.Normal().y;
    T z = -plane.Normal().z;
    T d = -plane.D();

    TScalarMatrix<T, 4, 4> result;
    result._11() = (x * light.x) + dot;
    result._21() = y * light.x;
    result._31() = z * light.x;
    result._41() = d * light.x;
    result._12() = x * light.y;
    result._22() = (y * light.y) + dot;
    result._32() = z * light.y;
    result._42() = d * light.y;
    result._13() = x * light.z;
    result._23() = y * light.z;
    result._33() = (z * light.z) + dot;
    result._43() = d * light.z;
    result._14() = x * light.w;
    result._24() = y * light.w;
    result._34() = z * light.w;
    result._44() = (d * light.w) + dot;

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _N>
TScalarMatrix<T, _N, _N> MakeDiagonalMatrix(const TScalarVector<T, _N>& diagonal) {
    TScalarMatrix<T, _N, _N> result(0);
    result.SetDiagonal(diagonal);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, u32 _N>
TScalarMatrix<T, _N + 1, _N + 1> MakeHomogeneousMatrix(const TScalarMatrix<T, _N, _N>& m) {
    TScalarMatrix<T, _N + 1, _N + 1> result;
    for (u32 col = 0; col < _N; ++col) {
        for (u32 row = 0; row < _N; ++row)
            result.at_(row, col) = m.at_(row, col);
        result.at_(_N, col) = T(0);
    }
    for (u32 row = 0; row < _N; ++row)
        result.at_(row, _N) = T(0);
    result.at_(_N, _N) = T(1);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, u32 _N>
TScalarMatrix<T, _N + 1, _N + 1> MakeScalingMatrix(const TScalarVector<T, _N>& scale) {
    TScalarMatrix<T, _N + 1, _N + 1> result(T(0));
    for (u32 i = 0; i < _N; ++i)
        result.at_(i, i) = scale[i];
    result.at(_N, _N) = T(1);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, u32 _N>
TScalarMatrix<T, _N + 1, _N + 1> MakeTranslationMatrix(const TScalarVector<T, _N>& translate) {
    TScalarMatrix<T, _N + 1, _N + 1> result = TScalarMatrix<T, _N + 1, _N + 1>::Identity();
    result.SetAxisT(translate);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, u32 _N>
TScalarMatrix<T, _N, _N - 1> PackHomogeneousMatrix(const TScalarMatrix<T, _N, _N>& m) {
    return Meta::static_for<_N>([&](auto... idx) NOEXCEPT -> TScalarMatrix<T, _N, _N - 1> {
        return { m.template Column<idx>().Shift()... };
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 2, 2> Make2DRotationMatrix(T radians) {
    float fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return Make2DRotationMatrix(fsin, fcos);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 2, 2> Make2DRotationMatrix(T fsin, T fcos) {
    TScalarMatrix<T, 2, 2> result;
    result.m00() = fcos;
    result.m10() = fsin;
    result.m01() = -fsin;
    result.m11() = fcos;
    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 2> Make2DTransformMatrix(const TScalarVector<T, 2>& translate, T scale, T radians) {
    float fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return Make2DTransformMatrix(translate, TScalarVector<T, 2>(scale), fsin, fcos);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 2> Make2DTransformMatrix(const TScalarVector<T, 2>& translate, const TScalarVector<T, 2>& scale, T fsin, T fcos) {
    TScalarMatrix<T, 3, 2> result;
    result.m00() = scale.x*fcos;
    result.m10() = scale.x*fsin;
    result.m01() = -scale.y*fsin;
    result.m11() = scale.y*fcos;
    result.m02() = translate.x;
    result.m12() = translate.y;
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline TScalarMatrix<float, 3, 3> Make3DRotationMatrix(const FQuaternion& rotation) {
    float xx = rotation.x * rotation.x;
    float yy = rotation.y * rotation.y;
    float zz = rotation.z * rotation.z;
    float xy = rotation.x * rotation.y;
    float zw = rotation.z * rotation.w;
    float zx = rotation.z * rotation.x;
    float yw = rotation.y * rotation.w;
    float yz = rotation.y * rotation.z;
    float xw = rotation.x * rotation.w;

    TScalarMatrix<float, 3, 3> result = TScalarMatrix<float, 3, 3>::Identity();
    result._11() = 1.0f - (2.0f * (yy + zz));
    result._12() = 2.0f * (xy + zw);
    result._13() = 2.0f * (zx - yw);
    result._21() = 2.0f * (xy - zw);
    result._22() = 1.0f - (2.0f * (zz + xx));
    result._23() = 2.0f * (yz + xw);
    result._31() = 2.0f * (zx + yw);
    result._32() = 2.0f * (yz - xw);
    result._33() = 1.0f - (2.0f * (yy + xx));

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrix(const TScalarVector<T, 3>& axis) {
    return Make3DRotationMatrix(axis, T(0), T(1)); // SinCos(0)
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrix(const TScalarVector<T, 3>& axis, T radians) {
    float fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return Make3DRotationMatrix(axis, fsin, fcos);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrix(const TScalarVector<T, 3>& axis, T fsin, T fcos) {
    Assert(IsNormalized(axis));

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;

    TScalarMatrix<T, 3, 3> result = TScalarMatrix<T, 3, 3>::Identity();
    result._11() = xx + (fcos * (1.0f - xx));
    result._12() = (xy - (fcos * xy)) + (fsin * z);
    result._13() = (xz - (fcos * xz)) - (fsin * y);
    result._21() = (xy - (fcos * xy)) - (fsin * z);
    result._22() = yy + (fcos * (1.0f - yy));
    result._23() = (yz - (fcos * yz)) + (fsin * x);
    result._31() = (xz - (fcos * xz)) + (fsin * y);
    result._32() = (yz - (fcos * yz)) - (fsin * x);
    result._33() = zz + (fcos * (1.0f - zz));

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> MakeYawPitchRollMatrix(T yaw, T pitch, T roll) {
    return Make3DRotationMatrix(MakeYawPitchRollQuaternion(yaw, pitch, roll));
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundX(T radians) {
    T fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return Make3DRotationMatrixAroundX(fsin, fcos);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundX(T fsin, T fcos) {
    TScalarMatrix<T, 3, 3> result = TScalarMatrix<T, 3, 3>::Identity();
    result._22() = fcos;
    result._23() = fsin;
    result._32() = -fsin;
    result._33() = fcos;
    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundY(T radians) {
    T fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return Make3DRotationMatrixAroundY(fsin, fcos);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundY(T fsin, T fcos) {
    TScalarMatrix<T, 3, 3> result = TScalarMatrix<T, 3, 3>::Identity();
    result._11() = fcos;
    result._13() = -fsin;
    result._31() = fsin;
    result._33() = fcos;
    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundZ(T radians) {
    T fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return Make3DRotationMatrixAroundZ(fsin, fcos);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Make3DRotationMatrixAroundZ(T fsin, T fcos) {
    TScalarMatrix<T, 3, 3> result = TScalarMatrix<T, 3, 3>::Identity();
    result._11() = fcos;
    result._12() = fsin;
    result._21() = -fsin;
    result._22() = fcos;
    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, T scale, const TScalarVector<T, 3>& axis, T radians) {
    T fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return Make3DTransformMatrix(translate, TScalarVector<T, 3>(scale), axis, fsin, fcos);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, T scale, const FQuaternion& rotation) {
    return Make3DTransformMatrix(translate, TScalarVector<T, 3>(scale), rotation);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, const TScalarVector<T, 3>& axis, T fsin, T fcos) {
    return Make3DTransformMatrix(translate, scale, Make3DRotationMatrix(axis, fsin, fcos).OneExtend());
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, const FQuaternion& rotation) {
    return Make3DTransformMatrix(translate, scale, Make3DRotationMatrix(rotation).OneExtend());
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrix(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, const TScalarMatrix<T, 4, 4>& rotation) {
    TScalarMatrix<T, 4, 4> result;
    result.SetColumn_x(rotation.Column_x() * scale.x);
    result.SetColumn_y(rotation.Column_y() * scale.y);
    result.SetColumn_z(rotation.Column_z() * scale.z);
    result.SetColumn_w(TScalarVector<T, 4>{ translate, T(1) });
    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundX(const TScalarVector<T, 3>& translate, T scale, T radians) {
    T fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return Make3DTransformMatrixAroundX(translate, TScalarVector<T, 3>(scale), fsin, fcos);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundX(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, T fsin, T fcos) {
    const TScalarMatrix<T, 4, 4> rotation = Make3DRotationMatrixAroundX(fsin, fcos);
    return Make3DTransformMatrix(translate, scale, rotation);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundY(const TScalarVector<T, 3>& translate, T scale, T radians) {
    T fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return Make3DTransformMatrixAroundY(translate, TScalarVector<T, 3>(scale), fsin, fcos);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundY(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, T fsin, T fcos) {
    const TScalarMatrix<T, 4, 4> rotation = Make3DRotationMatrixAroundY(fsin, fcos);
    return Make3DTransformMatrix(translate, scale, rotation);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundZ(const TScalarVector<T, 3>& translate, T scale, T radians) {
    T fsin, fcos;
    SinCos(radians, &fsin, &fcos);
    return Make3DTransformMatrixAroundZ(translate, TScalarVector<T, 3>(scale), fsin, fcos);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Make3DTransformMatrixAroundZ(const TScalarVector<T, 3>& translate, const TScalarVector<T, 3>& scale, T fsin, T fcos) {
    const TScalarMatrix<T, 4, 4> rotation = Make3DRotationMatrixAroundZ(fsin, fcos);
    return Make3DTransformMatrix(translate, scale, rotation);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> Transform3(const TScalarMatrix<T, 3, 3>& m, const TScalarVector<T, 3>& v) {
    const float fX = (m.m00()*v.x)+(m.m10()*v.y)+(m.m20()*v.z);
    const float fY = (m.m01()*v.x)+(m.m11()*v.y)+(m.m21()*v.z);
    const float fZ = (m.m02()*v.x)+(m.m12()*v.y)+(m.m22()*v.z);
    return TScalarVector<T, 3>(fX, fY, fZ);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> TransformPosition3(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v) {
    const TScalarVector<T, 4> homogeneous = Transform3_OneExtend(m, v);
    Assert_NoAssume(Abs(homogeneous.w) > SmallEpsilon);
    return (homogeneous.xyz / homogeneous.w);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> TransformVector3(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v) {
    return Transform3_ZeroExtend(m, v).xyz;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 4> Transform3_OneExtend(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v) {
    return TScalarVector<T, 4>(
        (m.m00()*v.x) + (m.m10()*v.y) + (m.m20()*v.z) + (m.m30()),
        (m.m01()*v.x) + (m.m11()*v.y) + (m.m21()*v.z) + (m.m31()),
        (m.m02()*v.x) + (m.m12()*v.y) + (m.m22()*v.z) + (m.m32()),
        (m.m03()*v.x) + (m.m13()*v.y) + (m.m23()*v.z) + (m.m33()) );
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 4> Transform3_ZeroExtend(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v) {
    return TScalarVector<T, 4>(
        (m.m00()*v.x) + (m.m10()*v.y) + (m.m20()*v.z),
        (m.m01()*v.x) + (m.m11()*v.y) + (m.m21()*v.z),
        (m.m02()*v.x) + (m.m12()*v.y) + (m.m22()*v.z),
        (m.m03()*v.x) + (m.m13()*v.y) + (m.m23()*v.z) );
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 4> Transform4(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 4>& v) {
    return TScalarVector<T, 4>(
        (m.m00()*v.x) + (m.m10()*v.y) + (m.m20()*v.z) + (m.m30()*v.w),
        (m.m01()*v.x) + (m.m11()*v.y) + (m.m21()*v.z) + (m.m31()*v.w),
        (m.m02()*v.x) + (m.m12()*v.y) + (m.m22()*v.z) + (m.m32()*v.w),
        (m.m03()*v.x) + (m.m13()*v.y) + (m.m23()*v.z) + (m.m33()*v.w) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
