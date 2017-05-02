#include "stdafx.h"

#include "LeastSquaresFitting.h"

#include "EigenMatrixSolver.h"
#include "Plane.h"
#include "ScalarMatrix.h"
#include "ScalarVector.h"

// TODO : many interesting maths tools here :
// http://www.geometrictools.com/Source/Approximation.html
// http://www.geometrictools.com/GTEngine/Include/Mathematics/GteApprOrthogonalPlane3.h
// https://github.com/qloach/GeometricToolsEngine1p0/blob/master/GeometricTools/GTEngine/Include/GteLinearSystem.inl
// https://github.com/qloach/GeometricToolsEngine1p0/blob/master/GeometricTools/GTEngine/Include/GteApprHeightPlane3.inl
// https://github.com/qloach/GeometricToolsEngine1p0/blob/master/GeometricTools/GTEngine/Include/GteApprOrthogonalPlane3.h

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool LeastSquaresFittingGaussian2(float2& center, float2& axis0, float2& axis1, float2& extents, const TMemoryView<const float2>& points) {
    if (points.size() < 2)
        return false;

    // Compute the mean of the points.
    float2 mean = float2::Zero();
    for (const float2& p : points)
        mean += p;

    float invSize = ((float)1) / (float)points.size();
    mean *= invSize;

    // Compute the covariance matrix of the points.
    float covar00 = (float)0, covar01 = (float)0, covar11 = (float)0;
    for (const float2& p : points) {
        const float2 diff = (p - mean);
        covar00 += diff[0] * diff[0];
        covar01 += diff[0] * diff[1];
        covar11 += diff[1] * diff[1];
    }
    covar00 *= invSize;
    covar01 *= invSize;
    covar11 *= invSize;

    // Solve the eigensystem.
    TSymmetricEigensolver<float, 2> es(32);
    float2x2 M =
    {
        covar00, covar01,
        covar01, covar11
    };
    float2x2 R;
    float2 D;
    es.Solve(M, +1);  // D[0] <= D[1]
    es.GetEigenvalues(D);
    es.GetEigenvectors(R);

    if (es.IsRotation())
    {
        axis0 = float2(R.data().raw[0], R.data().raw[2]);
        axis1 = float2(R.data().raw[1], R.data().raw[3]);
    }
    else
    {
        axis0 = float2(R.data().raw[1], R.data().raw[3]);
        axis1 = float2(R.data().raw[0], R.data().raw[2]);
    }

    center = mean;
    extents[0] = D[0];
    extents[1] = D[1];

    return true;
}
//----------------------------------------------------------------------------
bool LeastSquaresFittingGaussian3(float3& center, float3& axis0, float3& axis1, float3& axis2, float3& extents, const TMemoryView<const float3>& points) {
    if (points.size() < 2)
        return false;

    // Compute the mean of the points.
    float3 mean = float3::Zero();
    for (const float3& p : points)
        mean += p;

    float invSize = ((float)1) / (float)points.size();
    mean *= invSize;

    // Compute the covariance matrix of the points.
    float covar00 = (float)0, covar01 = (float)0, covar02 = (float)0;
    float covar11 = (float)0, covar12 = (float)0, covar22 = (float)0;
    for (const float3& p : points) {
        const float3 diff = (p - mean);
        covar00 += diff[0] * diff[0];
        covar01 += diff[0] * diff[1];
        covar02 += diff[0] * diff[2];
        covar11 += diff[1] * diff[1];
        covar12 += diff[1] * diff[2];
        covar22 += diff[2] * diff[2];
    }
    covar00 *= invSize;
    covar01 *= invSize;
    covar02 *= invSize;
    covar11 *= invSize;
    covar12 *= invSize;
    covar22 *= invSize;

    // Solve the eigensystem.
    TSymmetricEigensolver<float, 3> es(32);
    float3x3 M =
    {
        covar00, covar01, covar02,
        covar01, covar11, covar12,
        covar02, covar12, covar22
    };
    float3x3 R;
    float3 D;
    es.Solve(M, +1);  // D[0] <= D[1] <= D[2]
    es.GetEigenvalues(D);
    es.GetEigenvectors(R);

    if (es.IsRotation())
    {
        axis0 = float3(R.data().raw[0], R.data().raw[3], R.data().raw[6]);
        axis1 = float3(R.data().raw[1], R.data().raw[4], R.data().raw[7]);
        axis2 = float3(R.data().raw[2], R.data().raw[5], R.data().raw[8]);
    }
    else
    {
        axis0 = float3(R.data().raw[0], R.data().raw[3], R.data().raw[6]);
        axis1 = float3(R.data().raw[2], R.data().raw[5], R.data().raw[8]);
        axis2 = float3(R.data().raw[1], R.data().raw[4], R.data().raw[7]);
    }

    center = mean;
    extents[0] = D[0];
    extents[1] = D[1];
    extents[2] = D[2];
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
float LeastSquaresFittingQuadratic2(TScalarVector<float, 6>& coefficients, const TMemoryView<const float2>& points) {
    Assert(points.size() > 1);

    TScalarMatrix<float, 6, 6> A(Meta::FForceInit{});
    for (const float2& p : points) {
        float x = p[0];
        float y = p[1];
        float x2 = x*x;
        float y2 = y*y;
        float xy = x*y;
        float x3 = x*x2;
        float xy2 = x*y2;
        float x2y = x*xy;
        float y3 = y*y2;
        float x4 = x*x3;
        float x2y2 = x*xy2;
        float x3y = x*x2y;
        float y4 = y*y3;
        float xy3 = x*y3;

        A(0, 1) += x;
        A(0, 2) += y;
        A(0, 3) += x2;
        A(0, 4) += y2;
        A(0, 5) += xy;
        A(1, 3) += x3;
        A(1, 4) += xy2;
        A(1, 5) += x2y;
        A(2, 4) += y3;
        A(3, 3) += x4;
        A(3, 4) += x2y2;
        A(3, 5) += x3y;
        A(4, 4) += y4;
        A(4, 5) += xy3;
    }

    A(0, 0) = static_cast<float>(points.size());
    A(1, 1) = A(0, 3);
    A(1, 2) = A(0, 5);
    A(2, 2) = A(0, 4);
    A(2, 3) = A(1, 5);
    A(2, 5) = A(1, 4);
    A(5, 5) = A(3, 4);

    for (int row = 0; row < 6; ++row)
    {
        for (int col = 0; col < row; ++col)
        {
            A(row, col) = A(col, row);
        }
    }

    float invNumPoints = ((float)1) / static_cast<float>(points.size());
    for (int row = 0; row < 6; ++row)
    {
        for (int col = 0; col < 6; ++col)
        {
            A(row, col) *= invNumPoints;
        }
    }

    TSymmetricEigensolver<float, 6> es(1024);
    es.Solve(A, +1);
    es.GetEigenvector(0, coefficients);

    // For an exact fit, numeric round-off errors might make the minimum
    // eigenvalue just slightly negative.  Return the absolute value since
    // the application might rely on the return value being nonnegative.
    return Abs(es.GetEigenvalue(0));
}
//----------------------------------------------------------------------------
float LeastSquaresFittingQuadraticCircle2(float2& center, float radius, const TMemoryView<const float2>& points) {
    Assert(points.size() > 1);

    float4x4 A(Meta::FForceInit{});
    for (const float2& p : points) {
        float x = p[0];
        float y = p[1];
        float x2 = x*x;
        float y2 = y*y;
        float xy = x*y;
        float r2 = x2+y2;
        float xr2 = x*r2;
        float yr2 = y*r2;
        float r4 = r2*r2;

        A(0, 1) += x;
        A(0, 2) += y;
        A(0, 3) += r2;
        A(1, 1) += x2;
        A(1, 2) += xy;
        A(1, 3) += xr2;
        A(2, 2) += y2;
        A(2, 3) += yr2;
        A(3, 3) += r4;
    }

    A(0, 0) = static_cast<float>(points.size());

    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < row; ++col)
        {
            A(row, col) = A(col, row);
        }
    }

    float invNumPoints = ((float)1) / static_cast<float>(points.size());
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            A(row, col) *= invNumPoints;
        }
    }

    TSymmetricEigensolver<float, 4> es(1024);
    es.Solve(A, +1);
    float4 evector;
    es.GetEigenvector(0, evector);

    float inv = ((float)1) / evector[3];  // TODO: Guard against zero divide?
    float coefficients[3];
    for (int row = 0; row < 3; ++row)
    {
        coefficients[row] = inv * evector[row];
    }

    center[0] = ((float)-0.5) * coefficients[1];
    center[1] = ((float)-0.5) * coefficients[2];
    radius = Sqrt(Abs(Dot2(center, center) - coefficients[0]));

    // For an exact fit, numeric round-off errors might make the minimum
    // eigenvalue just slightly negative.  Return the absolute value since
    // the application might rely on the return value being nonnegative.
    return Abs(es.GetEigenvalue(0));
}
//----------------------------------------------------------------------------
float LeastSquaresFittingQuadratic3(TScalarVector<float, 10>& coefficients, const TMemoryView<const float3>& points) {
    Assert(points.size() > 1);

    TScalarMatrix<float, 10, 10> A(Meta::FForceInit{});
    for (const float3& p : points)
    {
        float x = p[0];
        float y = p[1];
        float z = p[2];
        float x2 = x*x;
        float y2 = y*y;
        float z2 = z*z;
        float xy = x*y;
        float xz = x*z;
        float yz = y*z;
        float x3 = x*x2;
        float xy2 = x*y2;
        float xz2 = x*z2;
        float x2y = x*xy;
        float x2z = x*xz;
        float xyz = x*y*z;
        float y3 = y*y2;
        float yz2 = y*z2;
        float y2z = y*yz;
        float z3 = z*z2;
        float x4 = x*x3;
        float x2y2 = x*xy2;
        float x2z2 = x*xz2;
        float x3y = x*x2y;
        float x3z = x*x2z;
        float x2yz = x*xyz;
        float y4 = y*y3;
        float y2z2 = y*yz2;
        float xy3 = x*y3;
        float xy2z = x*y2z;
        float y3z = y*y2z;
        float z4 = z*z3;
        float xyz2 = x*yz2;
        float xz3 = x*z3;
        float yz3 = y*z3;

        A(0, 1) += x;
        A(0, 2) += y;
        A(0, 3) += z;
        A(0, 4) += x2;
        A(0, 5) += y2;
        A(0, 6) += z2;
        A(0, 7) += xy;
        A(0, 8) += xz;
        A(0, 9) += yz;
        A(1, 4) += x3;
        A(1, 5) += xy2;
        A(1, 6) += xz2;
        A(1, 7) += x2y;
        A(1, 8) += x2z;
        A(1, 9) += xyz;
        A(2, 5) += y3;
        A(2, 6) += yz2;
        A(2, 9) += y2z;
        A(3, 6) += z3;
        A(4, 4) += x4;
        A(4, 5) += x2y2;
        A(4, 6) += x2z2;
        A(4, 7) += x3y;
        A(4, 8) += x3z;
        A(4, 9) += x2yz;
        A(5, 5) += y4;
        A(5, 6) += y2z2;
        A(5, 7) += xy3;
        A(5, 8) += xy2z;
        A(5, 9) += y3z;
        A(6, 6) += z4;
        A(6, 7) += xyz2;
        A(6, 8) += xz3;
        A(6, 9) += yz3;
        A(9, 9) += y2z2;
    }

    A(0, 0) = static_cast<float>(points.size());
    A(1, 1) = A(0, 4);
    A(1, 2) = A(0, 7);
    A(1, 3) = A(0, 8);
    A(2, 2) = A(0, 5);
    A(2, 3) = A(0, 9);
    A(2, 4) = A(1, 7);
    A(2, 7) = A(1, 5);
    A(2, 8) = A(1, 9);
    A(3, 3) = A(0, 6);
    A(3, 4) = A(1, 8);
    A(3, 5) = A(2, 9);
    A(3, 7) = A(1, 9);
    A(3, 8) = A(1, 6);
    A(3, 9) = A(2, 6);
    A(7, 7) = A(4, 5);
    A(7, 8) = A(4, 9);
    A(7, 9) = A(5, 8);
    A(8, 8) = A(4, 6);
    A(8, 9) = A(6, 7);
    A(9, 9) = A(5, 6);

    for (int row = 0; row < 10; ++row)
    {
        for (int col = 0; col < row; ++col)
        {
            A(row, col) = A(col, row);
        }
    }

    float invNumPoints = ((float)1) / static_cast<float>(points.size());
    for (int row = 0; row < 10; ++row)
    {
        for (int col = 0; col < 10; ++col)
        {
            A(row, col) *= invNumPoints;
        }
    }

    TSymmetricEigensolver<float, 10> es(1024);
    es.Solve(A, +1);
    es.GetEigenvector(0, coefficients);

    // For an exact fit, numeric round-off errors might make the minimum
    // eigenvalue just slightly negative.  Return the absolute value since
    // the application might rely on the return value being nonnegative.
    return Abs(es.GetEigenvalue(0));
}
//----------------------------------------------------------------------------
float LeastSquaresFittingQuadraticSphere3(float3& center, float radius, const TMemoryView<const float3>& points) {
    Assert(points.size() > 1);

    TScalarMatrix<float, 5, 5> A(Meta::FForceInit{});
    for (const float3& p : points)
    {
        float x = p[0];
        float y = p[1];
        float z = p[2];
        float x2 = x*x;
        float y2 = y*y;
        float z2 = z*z;
        float xy = x*y;
        float xz = x*z;
        float yz = y*z;
        float r2 = x2+y2+z2;
        float xr2 = x*r2;
        float yr2 = y*r2;
        float zr2 = z*r2;
        float r4 = r2*r2;

        A(0, 1) += x;
        A(0, 2) += y;
        A(0, 3) += z;
        A(0, 4) += r2;
        A(1, 1) += x2;
        A(1, 2) += xy;
        A(1, 3) += xz;
        A(1, 4) += xr2;
        A(2, 2) += y2;
        A(2, 3) += yz;
        A(2, 4) += yr2;
        A(3, 3) += z2;
        A(3, 4) += zr2;
        A(4, 4) += r4;
    }

    A(0, 0) = static_cast<float>(points.size());

    for (int row = 0; row < 5; ++row)
    {
        for (int col = 0; col < row; ++col)
        {
            A(row, col) = A(col, row);
        }
    }

    float invNumPoints = ((float)1) / static_cast<float>(points.size());
    for (int row = 0; row < 5; ++row)
    {
        for (int col = 0; col < 5; ++col)
        {
            A(row, col) *= invNumPoints;
        }
    }

    TSymmetricEigensolver<float, 5> es(1024);
    es.Solve(A, +1);
    TScalarVector<float, 5> evector;
    es.GetEigenvector(0, evector);

    float inv = ((float)1) / evector[4];  // TODO: Guard against zero divide?
    float coefficients[4];
    for (int row = 0; row < 4; ++row)
    {
        coefficients[row] = inv * evector[row];
    }

    center[0] = ((float)-0.5) * coefficients[1];
    center[1] = ((float)-0.5) * coefficients[2];
    center[2] = ((float)-0.5) * coefficients[3];
    radius = Sqrt(Abs(Dot3(center, center) - coefficients[0]));

    // For an exact fit, numeric round-off errors might make the minimum
    // eigenvalue just slightly negative.  Return the absolute value since
    // the application might rely on the return value being nonnegative.
    return Abs(es.GetEigenvalue(0));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool LeastSquaresFittingOrthogonalLine2(float2& origin, float2& direction, const TMemoryView<const float2>& points) {
    if (points.size() < 2)
        return false;

    // Compute the mean of the points.
    float2 mean = float2::Zero();
    for (const float2& p : points)
        mean += p;
    mean /= (float)points.size();

    // Compute the covariance matrix of the points.
    float covar00 = (float)0, covar01 = (float)0, covar11 = (float)0;
    for (const float2& p : points) {
        const float2 diff = (p - mean);
        covar00 += diff[0] * diff[0];
        covar01 += diff[0] * diff[1];
        covar11 += diff[1] * diff[1];
    }

    // Solve the eigensystem.
    TSymmetricEigensolver<float, 2> es(32);
    float2x2 M =
    {
        covar00, covar01,
        covar01, covar11
    };
    float2x2 R;
    float2 D;
    es.Solve(M, +1);  // D[0] <= D[1]
    es.GetEigenvalues(D);
    es.GetEigenvectors(R);

    // The line direction is the eigenvector in the direction of largest
    // variance of the points.
    origin = mean;
    direction = float2(R.data().raw[1], R.data().raw[3]);

    // The fitted line is unique when the maximum eigenvalue has
    // multiplicity 1.
    return (D[0] < D[1]);
}
//----------------------------------------------------------------------------
bool LeastSquaresFittingOrthogonalLine3(float3& origin, float3& direction, const TMemoryView<const float3>& points) {
    if (points.size() < 2)
        return false;

    float3 mean = float3::Zero();
    for (const float3& p : points)
        mean += p;

    float invSize = ((float)1) / (float)points.size();
    mean *= invSize;

    // Compute the covariance matrix of the points.
    float covar00 = (float)0, covar01 = (float)0, covar02 = (float)0;
    float covar11 = (float)0, covar12 = (float)0, covar22 = (float)0;
    for (const float3& p : points) {
        const float3 diff = p - mean;
        covar00 += diff[0] * diff[0];
        covar01 += diff[0] * diff[1];
        covar02 += diff[0] * diff[2];
        covar11 += diff[1] * diff[1];
        covar12 += diff[1] * diff[2];
        covar22 += diff[2] * diff[2];
    }
    covar00 *= invSize;
    covar01 *= invSize;
    covar02 *= invSize;
    covar11 *= invSize;
    covar12 *= invSize;
    covar22 *= invSize;

    // Solve the eigensystem.
    TSymmetricEigensolver<float, 3> es(32);
    float3x3 M =
    {
        covar00, covar01, covar02,
        covar01, covar11, covar12,
        covar02, covar12, covar22
    };
    float3x3 R;
    float3 D;
    es.Solve(M, +1);  // D[0] <= D[1] <= D[2]
    es.GetEigenvalues(D);
    es.GetEigenvectors(R);

    // The line direction is the eigenvector in the direction of largest
    // variance of the points.
    origin = mean;
    direction = float3(R.data().raw[2], R.data().raw[5], R.data().raw[8]);

    // The fitted line is unique when the maximum eigenvalue has
    // multiplicity 1.
    return (D[1]< D[2]);
}
//----------------------------------------------------------------------------
bool LeastSquaresFittingOrthogonalPlane3(FPlane& plane, const TMemoryView<const float3>& points) {
    if (points.size() < 3)
        return false;

    // Compute the mean of the points.
    float3 mean(0.f);
    for (const float3& p : points)
        mean += p;
    mean /= float(points.size());

    // Compute the covariance matrix of the points.
    // Compute the covariance matrix of the points.
    float covar00 = (float)0, covar01 = (float)0, covar02 = (float)0;
    float covar11 = (float)0, covar12 = (float)0, covar22 = (float)0;
    for (const float3& p : points) {
        const float3 diff = p - mean;
        covar00 += diff[0] * diff[0];
        covar01 += diff[0] * diff[1];
        covar02 += diff[0] * diff[2];
        covar11 += diff[1] * diff[1];
        covar12 += diff[1] * diff[2];
        covar22 += diff[2] * diff[2];
    }

    // Solve the eigensystem.
    TSymmetricEigensolver<float, 3> es(32);
    float3x3 M =
    {
        covar00, covar01, covar02,
        covar01, covar11, covar12,
        covar02, covar12, covar22
    };
    float3x3 R;
    float3 D;
    es.Solve(M, +1);  // D[0] <= D[1] <= D[2]
    es.GetEigenvalues(D);
    es.GetEigenvectors(R);

    // The plane normal is the eigenvector in the direction of smallest
    // variance of the points.
    const float3 normal(R.data().raw[0], R.data().raw[3], R.data().raw[6]);
    plane = FPlane(normal, mean);

    // The fitted plane is unique when the minimum eigenvalue has
    // multiplicity 1.
    return (D[0] < D[1]);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
