#pragma once

#include "Maths/ScalarMatrixHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
bool IsHomogeneous(const TScalarMatrix<T, _Rows, _Cols>& m, float epsilon/* = Epsilon */) {
    return NearlyEquals(m.rows[_Rows - 1], TScalarVector<T, _Cols>::Homogeneous, epsilon);
}
//----------------------------------------------------------------------------
template <typename T, u32 N>
bool IsInversible(const TScalarMatrix<T, N, N>& m, float epsilon/* = Epsilon */) {
    float norm = 0;
    for (u32 j = 0; j < N; ++j)
        for (u32 i = 0; i < N; ++i)
            norm += m.m[j][i] * m.m[i][j];

    norm = Sqrt(norm);
    if (norm <= epsilon)
        return false;

    const float epsilon3 = epsilon * epsilon * epsilon;
    Assert(epsilon3 > 0);

    const float det = Det(m);
    return Abs(det) > epsilon3*norm;
}
//----------------------------------------------------------------------------
template <typename T, u32 N>
bool IsIdentity(const TScalarMatrix<T, N, N>& m, float epsilon/* = Epsilon */) {
    return NearlyEquals(TScalarMatrix<T, N, N>::Identity, m, epsilon);
}
//----------------------------------------------------------------------------
template <typename T, u32 N>
bool IsOrthogonal(const TScalarMatrix<T, N, N>& m, float epsilon/* = Epsilon */) {
    const TScalarMatrix<T, N, N> id = (m * m.transposed);
    return IsIdentity(id, epsilon);
}
//----------------------------------------------------------------------------
template <typename T, u32 N>
bool IsOrthonormal(const TScalarMatrix<T, N, N>& m, float epsilon/* = Epsilon */) {
    const T sqrtEpsilon(Sqrt(epsilon));
    for (u32 i = 0; i < N; ++i) {
        const T rowLenMinusOne = Abs(T(1) - LengthSq(m.rows[i]));
        if (rowLenMinusOne > sqrtEpsilon)
            return false;
    }
    return IsOrthogonal(m, epsilon);
}
//----------------------------------------------------------------------------
template <typename T, u32 N>
bool IsSymmetric(const TScalarMatrix<T, N, N>& m, float epsilon/* = Epsilon */) {
    return NearlyEquals(m, m.transposed, epsilon);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
bool IsINF(const TScalarMatrix<T, _Rows, _Cols>& m) {
    for (const T& f : m.data) {
        if (IsINF(f))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
bool IsNAN(const TScalarMatrix<T, _Rows, _Cols>& m) {
    for (const T& f : m.data) {
        if (IsNAN(f))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
bool IsNANorINF(const TScalarMatrix<T, _Rows, _Cols>& m) {
    for (const T& f : m.data) {
        if (IsNANorINF(f))
            return true;
    }
    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Lhs, typename _Rhs>
bool NearlyEquals(
    const details::TScalarMatrixExpr<T, _Rows, _Cols, _Lhs>& lhs,
    const details::TScalarMatrixExpr<T, _Rows, _Cols, _Rhs>& rhs,
    float maxRelDiff/* = Epsilon */) {
    forrange(i, 0, _Rows)
    forrange(j, 0, _Cols) {
        if (not NearlyEquals(lhs.Get(i, j), rhs.Get(i, j), maxRelDiff))
            return false;
    }
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols, typename _Lhs, typename _Rhs>
TScalarMatrix<T, _Rows, _Cols> OuterProduct(const details::TScalarVectorExpr<T, _Rows, _Lhs>& u,
                                            const details::TScalarVectorExpr<T, _Cols, _Rhs>& v) {
    return Meta::static_for<u32, _Rows, _Cols>([&](auto... it) -> TScalarMatrix<T, _Rows, _Cols> {
        return { (u.template Get<it.first>() * v.template Get<it.second>())... };
    });
}
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
void Lerp(  const TScalarMatrix<T, _Rows, _Cols>& v0,
            const TScalarMatrix<T, _Rows, _Cols>& v1,
            float f,
            TScalarMatrix<T, _Rows, _Cols>& result) {
    // TODO : this has probably no sense :'(
    for (u32 i = 0; i < TScalarMatrix<T, _Rows, _Cols>::Dim; ++i)
        result.data[i] = Lerp(v0.data[i], v1.data[i], f);
}
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
TScalarMatrix<T, _Rows, _Cols> Lerp(
    const TScalarMatrix<T, _Rows, _Cols>& v0,
    const TScalarMatrix<T, _Rows, _Cols>& v1,
    float f ) {
    // TODO : this has probably no sense :'(
    TScalarMatrix<T, _Rows, _Cols> result;
    Lerp(v0, v1, f, result);
    return result;
}
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
void SLerp( const TScalarMatrix<T, _Rows, _Cols>& v0,
            const TScalarMatrix<T, _Rows, _Cols>& v1,
            float f,
            TScalarMatrix<T, _Rows, _Cols>& result) {
    // TODO : this has probably no sense :'(
    for (u32 i = 0; i < TScalarMatrix<T, _Rows, _Cols>::Dim; ++i)
        result.data[i] = SLerp(v0.data[i], v1.data[i], f);
}
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
void Smoothstep(const TScalarMatrix<T, _Rows, _Cols>& v0,
                const TScalarMatrix<T, _Rows, _Cols>& v1,
                float f,
                TScalarMatrix<T, _Rows, _Cols>& result) {
    // TODO : this has probably no sense :'(
    for (u32 i = 0; i < TScalarMatrix<T, _Rows, _Cols>::Dim; ++i)
        result.data[i] = Smoothstep(v0.data[i], v1.data[i], f);
}
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
void Smootherstep(  const TScalarMatrix<T, _Rows, _Cols>& v0,
                    const TScalarMatrix<T, _Rows, _Cols>& v1,
                    float f,
                    TScalarMatrix<T, _Rows, _Cols>& result) {
    // TODO : this has probably no sense :'(
    for (u32 i = 0; i < TScalarMatrix<T, _Rows, _Cols>::Dim; ++i)
        result.data[i] = Smootherstep(v0.data[i], v1.data[i], f);
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

    result.rows[1] = result.rows[1] - (Dot(result.rows[0], result.rows[1]) / Dot(result.rows[0], result.rows[0])) * result.rows[0] ;

    result.rows[2] = result.rows[2] - (Dot(result.rows[0], result.rows[2]) / Dot(result.rows[0], result.rows[0])) * result.rows[0] ;
    result.rows[2] = result.rows[2] - (Dot(result.rows[1], result.rows[2]) / Dot(result.rows[1], result.rows[1])) * result.rows[1] ;

    result.rows[3] = result.rows[3] - (Dot(result.rows[0], result.rows[3]) / Dot(result.rows[0], result.rows[0])) * result.rows[0] ;
    result.rows[3] = result.rows[3] - (Dot(result.rows[1], result.rows[3]) / Dot(result.rows[1], result.rows[1])) * result.rows[1] ;
    result.rows[3] = result.rows[3] - (Dot(result.rows[2], result.rows[3]) / Dot(result.rows[2], result.rows[2])) * result.rows[2] ;

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

    result.rows[0] = Normalize(result.rows[0]) ;

    result.rows[1] = result.rows[1] - Dot(result.rows[0], result.rows[1]) * result.rows[0] ;
    result.rows[1] = Normalize(result.rows[1]) ;

    result.rows[2] = result.rows[2] - Dot(result.rows[0], result.rows[2]) * result.rows[0] ;
    result.rows[2] = result.rows[2] - Dot(result.rows[1], result.rows[2]) * result.rows[1] ;
    result.rows[2] = Normalize(result.rows[2]) ;

    result.rows[3] = result.rows[3] - Dot(result.rows[0], result.rows[3]) * result.rows[0] ;
    result.rows[3] = result.rows[3] - Dot(result.rows[1], result.rows[3]) * result.rows[1] ;
    result.rows[3] = result.rows[3] - Dot(result.rows[2], result.rows[3]) * result.rows[2] ;
    result.rows[3] = Normalize(result.rows[3]);

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
void DecomposeQR(const TScalarMatrix<T, 4, 4>& m, TScalarMatrix<T, 4, 4>& q, TScalarMatrix<T, 4, 4>& r) {
    // Decomposes a matrix into an ortho-normalized matrix Q and a right triangular matrix R.

    q = Orthonormalize(m.transposed).transposed;

    r.SetBroadcast(0);

    r.m[0][0] = Dot(q.template Column<0>(), m.template Column<0>());
    r.m[0][1] = Dot(q.template Column<0>(), m.template Column<1>());
    r.m[0][2] = Dot(q.template Column<0>(), m.template Column<2>());
    r.m[0][3] = Dot(q.template Column<0>(), m.template Column<3>());

    r.m[1][1] = Dot(q.template Column<1>(), m.template Column<1>());
    r.m[1][2] = Dot(q.template Column<1>(), m.template Column<2>());
    r.m[1][3] = Dot(q.template Column<1>(), m.template Column<3>());

    r.m[2][2] = Dot(q.template Column<2>(), m.template Column<2>());
    r.m[2][3] = Dot(q.template Column<2>(), m.template Column<3>());

    r.m[3][3] = Dot(q.template Column<3>(), m.template Column<3>());
}
//----------------------------------------------------------------------------
template <typename T>
void DecomposeLQ(const TScalarMatrix<T, 4, 4>& m, TScalarMatrix<T, 4, 4>& l, TScalarMatrix<T, 4, 4>& q) {
    // Decomposes a matrix into a lower triangular matrix L and an ortho-normalized matrix Q.

    q = Orthonormalize(m);

    l.SetBroadcast(0);

    l.m[0][0] = Dot(q.rows[0], m.rows[0]);

    l.m[1][0] = Dot(q.rows[0], m.rows[1]);
    l.m[1][1] = Dot(q.rows[1], m.rows[1]);

    l.m[2][0] = Dot(q.rows[0], m.rows[2]);
    l.m[2][1] = Dot(q.rows[1], m.rows[2]);
    l.m[2][2] = Dot(q.rows[2], m.rows[2]);

    l.m[3][0] = Dot(q.rows[0], m.rows[3]);
    l.m[3][1] = Dot(q.rows[1], m.rows[3]);
    l.m[3][2] = Dot(q.rows[2], m.rows[3]);
    l.m[3][3] = Dot(q.rows[3], m.rows[3]);
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
    translation = transform.template Column<3, 3>();

    //Scaling is the length of the rows.
    scale.x = Length(transform.rows[0].xyz);
    scale.y = Length(transform.rows[1].xyz);
    scale.z = Length(transform.rows[2].xyz);

    //If any of the scaling factors are zero, than the rotation matrix can not exist.
    Assert( Abs(scale.x) > Epsilon &&
            Abs(scale.y) > Epsilon &&
            Abs(scale.z) > Epsilon );

    //The rotation is the left over matrix after dividing out the scaling.
    rotation = MakeQuaternionFromRotationMatrix(TScalarMatrix<T, 3, 3>{
        { transform.rows[0].xyz / scale.x },
        { transform.rows[1].xyz / scale.y },
        { transform.rows[2].xyz / scale.z },
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Det(const TScalarMatrix<T, 2, 2>& m) {
    return  m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1];
}
//----------------------------------------------------------------------------
template <typename T>
T Det(const TScalarMatrix<T, 3, 3>& m) {
    return  m.m[0][0] * ( m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1] ) -
            m.m[0][1] * ( m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0] ) +
            m.m[0][2] * ( m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0] );
}
//----------------------------------------------------------------------------
template <typename T>
T Det(const TScalarMatrix<T, 4, 4>& m) {
    TScalarMatrix<T, 4, 4> tmp;

    tmp.m[0][0] = +(m.m[1][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) -
        m.m[2][1] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) +
        m.m[3][1] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2]));
    tmp.m[0][1] = -(m.m[1][0] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) -
        m.m[2][0] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) +
        m.m[3][0] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2]));
    tmp.m[0][2] = +(m.m[1][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) -
        m.m[2][0] * (m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1]) +
        m.m[3][0] * (m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1]));
    tmp.m[0][3] = -(m.m[1][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) -
        m.m[2][0] * (m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1]) +
        m.m[3][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]));

    tmp.m[1][0] = -(m.m[0][1] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) -
        m.m[2][1] * (m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2]) +
        m.m[3][1] * (m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2]));
    tmp.m[1][1] = +(m.m[0][0] * (m.m[2][2] * m.m[3][3] - m.m[2][3] * m.m[3][2]) -
        m.m[2][0] * (m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2]) +
        m.m[3][0] * (m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2]));
    tmp.m[1][2] = -(m.m[0][0] * (m.m[2][1] * m.m[3][3] - m.m[2][3] * m.m[3][1]) -
        m.m[2][0] * (m.m[0][1] * m.m[3][3] - m.m[0][3] * m.m[3][1]) +
        m.m[3][0] * (m.m[0][1] * m.m[2][3] - m.m[0][3] * m.m[2][1]));
    tmp.m[1][3] = +(m.m[0][0] * (m.m[2][1] * m.m[3][2] - m.m[2][2] * m.m[3][1]) -
        m.m[2][0] * (m.m[0][1] * m.m[3][2] - m.m[0][2] * m.m[3][1]) +
        m.m[3][0] * (m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]));

    tmp.m[2][0] = +(m.m[0][1] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) -
        m.m[1][1] * (m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2]) +
        m.m[3][1] * (m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2]));
    tmp.m[2][1] = -(m.m[0][0] * (m.m[1][2] * m.m[3][3] - m.m[1][3] * m.m[3][2]) -
        m.m[1][0] * (m.m[0][2] * m.m[3][3] - m.m[0][3] * m.m[3][2]) +
        m.m[3][0] * (m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2]));
    tmp.m[2][2] = +(m.m[0][0] * (m.m[1][1] * m.m[3][3] - m.m[1][3] * m.m[3][1]) -
        m.m[1][0] * (m.m[0][1] * m.m[3][3] - m.m[0][3] * m.m[3][1]) +
        m.m[3][0] * (m.m[0][1] * m.m[1][3] - m.m[0][3] * m.m[1][1]));
    tmp.m[2][3] = -(m.m[0][0] * (m.m[1][1] * m.m[3][2] - m.m[1][2] * m.m[3][1]) -
        m.m[1][0] * (m.m[0][1] * m.m[3][2] - m.m[0][2] * m.m[3][1]) +
        m.m[3][0] * (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]));

    tmp.m[3][0] = -(m.m[0][1] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2]) -
        m.m[1][1] * (m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2]) +
        m.m[2][1] * (m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2]));
    tmp.m[3][1] = +(m.m[0][0] * (m.m[1][2] * m.m[2][3] - m.m[1][3] * m.m[2][2]) -
        m.m[1][0] * (m.m[0][2] * m.m[2][3] - m.m[0][3] * m.m[2][2]) +
        m.m[2][0] * (m.m[0][2] * m.m[1][3] - m.m[0][3] * m.m[1][2]));
    tmp.m[3][2] = -(m.m[0][0] * (m.m[1][1] * m.m[2][3] - m.m[1][3] * m.m[2][1]) -
        m.m[1][0] * (m.m[0][1] * m.m[2][3] - m.m[0][3] * m.m[2][1]) +
        m.m[2][0] * (m.m[0][1] * m.m[1][3] - m.m[0][3] * m.m[1][1]));
    tmp.m[3][3] = +(m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
        m.m[1][0] * (m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1]) +
        m.m[2][0] * (m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1]));

    const T d = m.m[0][0] * tmp.m[0][0] +
                m.m[1][0] * tmp.m[1][0] +
                m.m[2][0] * tmp.m[2][0] +
                m.m[3][0] * tmp.m[3][0];

    return d;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const TScalarMatrix<T, 2, 2>& m) {
    return m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1];
}
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const TScalarMatrix<T, 3, 3>& m) {
    return m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1];
}
//----------------------------------------------------------------------------
template <typename T>
T Det2x2(const TScalarMatrix<T, 4, 4>& m) {
    return m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1];
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
T Det3x3(const TScalarMatrix<T, 3, 3>& m) {
    return  m.m[0][0] * ( m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1] ) -
            m.m[0][1] * ( m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0] ) +
            m.m[0][2] * ( m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0] );
}
//----------------------------------------------------------------------------
template <typename T>
T Det3x3(const TScalarMatrix<T, 4, 4>& m) {
    return  m.m[0][0] * ( m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1] ) -
            m.m[0][1] * ( m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0] ) +
            m.m[0][2] * ( m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0] );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 2, 2> Invert(const TScalarMatrix<T, 2, 2>& m) {
    float det = Det2x2(m);
    Assert(det != 0);
    float ooDet = Rcp(det);

    TScalarMatrix<T, 2, 2> result;
    result.m[0][0] =  ooDet * m.m[1][1];
    result.m[1][0] = -ooDet * m.m[1][0];
    result.m[0][1] = -ooDet * m.m[0][1];
    result.m[1][1] =  ooDet * m.m[0][0];

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Invert(const TScalarMatrix<T, 3, 3>& m) {
    float det = Det(m);
    Assert(det != 0);
    float ooDet = Rcp(det);

    TScalarMatrix<T, 3, 3> result;
    result.m[0][0] =  ooDet * ( m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1] );
    result.m[0][1] = -ooDet * ( m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1] );
    result.m[0][2] =  ooDet * ( m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1] );

    result.m[1][0] = -ooDet * ( m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0] );
    result.m[1][1] =  ooDet * ( m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0] );
    result.m[1][2] = -ooDet * ( m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0] );

    result.m[2][0] =  ooDet * ( m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0] );
    result.m[2][1] = -ooDet * ( m.m[0][0] * m.m[2][1] - m.m[0][1] * m.m[2][0] );
    result.m[2][2] =  ooDet * ( m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0] );

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 3, 3> Invert3x3(const TScalarMatrix<T, 4, 4>& m) {
    float det = Det3x3(m);
    Assert(det != 0);
    float ooDet = Rcp(det);

    TScalarMatrix<T, 3, 3> result;
    result.m[0][0] =  ooDet * ( m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1] );
    result.m[0][1] = -ooDet * ( m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1] );
    result.m[0][2] =  ooDet * ( m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1] );

    result.m[1][0] = -ooDet * ( m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0] );
    result.m[1][1] =  ooDet * ( m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0] );
    result.m[1][2] = -ooDet * ( m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0] );

    result.m[2][0] =  ooDet * ( m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0] );
    result.m[2][1] = -ooDet * ( m.m[0][0] * m.m[2][1] - m.m[0][1] * m.m[2][0] );
    result.m[2][2] =  ooDet * ( m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0] );

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Invert(const TScalarMatrix<T, 4, 4>& m) {
    T b0 = (m.m[2][0] * m.m[3][1]) - (m.m[2][1] * m.m[3][0]);
    T b1 = (m.m[2][0] * m.m[3][2]) - (m.m[2][2] * m.m[3][0]);
    T b2 = (m.m[2][3] * m.m[3][0]) - (m.m[2][0] * m.m[3][3]);
    T b3 = (m.m[2][1] * m.m[3][2]) - (m.m[2][2] * m.m[3][1]);
    T b4 = (m.m[2][3] * m.m[3][1]) - (m.m[2][1] * m.m[3][3]);
    T b5 = (m.m[2][2] * m.m[3][3]) - (m.m[2][3] * m.m[3][2]);

    T d11 = m.m[1][1] * b5 + m.m[1][2] * b4 + m.m[1][3] * b3;
    T d12 = m.m[1][0] * b5 + m.m[1][2] * b2 + m.m[1][3] * b1;
    T d13 = m.m[1][0] *-b4 + m.m[1][1] * b2 + m.m[1][3] * b0;
    T d14 = m.m[1][0] * b3 + m.m[1][1] *-b1 + m.m[1][2] * b0;

    T det = m.m[0][0] * d11 - m.m[0][1] * d12 + m.m[0][2] * d13 - m.m[0][3] * d14;
    Assert(std::abs(det) > Epsilon);

    det = Rcp(det);

    T a0 = (m.m[0][0] * m.m[1][1]) - (m.m[0][1] * m.m[1][0]);
    T a1 = (m.m[0][0] * m.m[1][2]) - (m.m[0][2] * m.m[1][0]);
    T a2 = (m.m[0][3] * m.m[1][0]) - (m.m[0][0] * m.m[1][3]);
    T a3 = (m.m[0][1] * m.m[1][2]) - (m.m[0][2] * m.m[1][1]);
    T a4 = (m.m[0][3] * m.m[1][1]) - (m.m[0][1] * m.m[1][3]);
    T a5 = (m.m[0][2] * m.m[1][3]) - (m.m[0][3] * m.m[1][2]);

    T d21 = m.m[0][1] * b5 + m.m[0][2] * b4 + m.m[0][3] * b3;
    T d22 = m.m[0][0] * b5 + m.m[0][2] * b2 + m.m[0][3] * b1;
    T d23 = m.m[0][0] *-b4 + m.m[0][1] * b2 + m.m[0][3] * b0;
    T d24 = m.m[0][0] * b3 + m.m[0][1] *-b1 + m.m[0][2] * b0;

    T d31 = m.m[3][1] * a5 + m.m[3][2] * a4 + m.m[3][3] * a3;
    T d32 = m.m[3][0] * a5 + m.m[3][2] * a2 + m.m[3][3] * a1;
    T d33 = m.m[3][0] *-a4 + m.m[3][1] * a2 + m.m[3][3] * a0;
    T d34 = m.m[3][0] * a3 + m.m[3][1] *-a1 + m.m[3][2] * a0;

    T d41 = m.m[2][1] * a5 + m.m[2][2] * a4 + m.m[2][3] * a3;
    T d42 = m.m[2][0] * a5 + m.m[2][2] * a2 + m.m[2][3] * a1;
    T d43 = m.m[2][0] *-a4 + m.m[2][1] * a2 + m.m[2][3] * a0;
    T d44 = m.m[2][0] * a3 + m.m[2][1] *-a1 + m.m[2][2] * a0;

    TScalarMatrix<T, 4, 4> result;

    result.m[0][0] = +d11 * det; result.m[0][1] = -d21 * det; result.m[0][2] = +d31 * det; result.m[0][3] = -d41 * det;
    result.m[1][0] = -d12 * det; result.m[1][1] = +d22 * det; result.m[1][2] = -d32 * det; result.m[1][3] = +d42 * det;
    result.m[2][0] = +d13 * det; result.m[2][1] = -d23 * det; result.m[2][2] = +d33 * det; result.m[2][3] = -d43 * det;
    result.m[3][0] = -d14 * det; result.m[3][1] = +d24 * det; result.m[3][2] = -d34 * det; result.m[3][3] = +d44 * det;

    return result;
}
//----------------------------------------------------------------------------
template <typename T>
TScalarMatrix<T, 4, 4> Invert_AssumeHomogeneous(const TScalarMatrix<T, 4, 4>& m) {
    Assert(IsHomogeneous(m));

    TScalarMatrix<T, 4, 4> result{ Meta::NoInit };

    // Invert the 3x3 rotation/scaling matrix
    T det = Det3x3(m);
    Assert(det != 0);
    T ooDet = Rcp(det);

    result.m[0][0] =  ooDet * ( m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1] );
    result.m[0][1] = -ooDet * ( m.m[0][1] * m.m[2][2] - m.m[0][2] * m.m[2][1] );
    result.m[0][2] =  ooDet * ( m.m[0][1] * m.m[1][2] - m.m[0][2] * m.m[1][1] );

    result.m[1][0] = -ooDet * ( m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0] );
    result.m[1][1] =  ooDet * ( m.m[0][0] * m.m[2][2] - m.m[0][2] * m.m[2][0] );
    result.m[1][2] = -ooDet * ( m.m[0][0] * m.m[1][2] - m.m[0][2] * m.m[1][0] );

    result.m[2][0] =  ooDet * ( m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0] );
    result.m[2][1] = -ooDet * ( m.m[0][0] * m.m[2][1] - m.m[0][1] * m.m[2][0] );
    result.m[2][2] =  ooDet * ( m.m[0][0] * m.m[1][1] - m.m[0][1] * m.m[1][0] );

    // Compute the new translation part: T' = -R^{-1} * T
    result.m[0][3] = -(result.m[0][0] * m.m[0][3] +
                       result.m[0][1] * m.m[1][3] +
                       result.m[0][2] * m.m[2][3]);

    result.m[1][3] = -(result.m[1][0] * m.m[0][3] +
                       result.m[1][1] * m.m[1][3] +
                       result.m[1][2] * m.m[2][3]);

    result.m[2][3] = -(result.m[2][0] * m.m[0][3] +
                       result.m[2][1] * m.m[1][3] +
                       result.m[2][2] * m.m[2][3]);

    // Set the last row to [0, 0, 0, 1]
    result.m[3][0] = T(0);
    result.m[3][1] = T(0);
    result.m[3][2] = T(0);
    result.m[3][3] = T(1);

    return result;
}
//----------------------------------------------------------------------------
template <typename T, u32 _Rows, u32 _Cols>
TScalarMatrix<T, _Cols, _Rows> InvertTranspose(const TScalarMatrix<T, _Rows, _Cols>& m) {
    return Invert(m.transposed);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> Transform3(const TScalarMatrix<T, 3, 3>& m, const TScalarVector<T, 3>& v) {
    return {Dot(m.rows[0], v),
            Dot(m.rows[1], v),
            Dot(m.rows[2], v) };
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> Transform3(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v) {
    return {Dot(m.rows[0].xyz, v),
            Dot(m.rows[1].xyz, v),
            Dot(m.rows[2].xyz, v) };
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
TScalarVector<T, 3> TransformNormal3(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& n) {
    Assert_NoAssume(IsNormalized(n));
    return Transform3(Invert3x3(m.transposed), n);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 3> TransformNormal3(const TScalarMatrix<T, 3, 3>& m, const TScalarVector<T, 3>& n) {
    Assert_NoAssume(IsNormalized(n));
    return Transform3(Invert(m.transposed), n);
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 4> Transform3_OneExtend(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v) {
    return {Dot(m.rows[0].xyz, v) + m.rows[0][3],
            Dot(m.rows[1].xyz, v) + m.rows[1][3],
            Dot(m.rows[2].xyz, v) + m.rows[2][3],
            Dot(m.rows[3].xyz, v) + m.rows[2][3] };
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 4> Transform3_ZeroExtend(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 3>& v) {
    return {Dot(m.rows[0].xyz, v),
            Dot(m.rows[1].xyz, v),
            Dot(m.rows[2].xyz, v),
            Dot(m.rows[3].xyz, v) };
}
//----------------------------------------------------------------------------
template <typename T>
TScalarVector<T, 4> Transform4(const TScalarMatrix<T, 4, 4>& m, const TScalarVector<T, 4>& v) {
    return {Dot(m.rows[0], v),
            Dot(m.rows[1], v),
            Dot(m.rows[2], v),
            Dot(m.rows[3], v) };
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
