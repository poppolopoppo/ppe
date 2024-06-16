#pragma once

#include "Core_fwd.h"

#include "Container/Vector.h"

#include "Maths/EigenMatrixSolver.h"
#include "Maths/RandomGenerator.h"
#include "Maths/ScalarMatrix.h"
#include "Maths/ScalarMatrixHelpers.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FGaussian2f {
    float2      Mean;
    float2x2    Covariance;

    FGaussian2f() = default;
    FGaussian2f(const float2& mean, const float2& std_dev, const float2x2& rotation) NOEXCEPT
    :   Mean(mean)
    ,   Covariance(rotation.Multiply(MakeDiagonalMatrix(std_dev)).Multiply(rotation.Transpose()))
    {}

    void ToEllipse(float2* outOrigin, float2* outRadii, float* outOrientation, const size_t maxIterations = 50) const NOEXCEPT {
        TSymmetricEigensolver<float, 2> es(maxIterations);
        es.Solve(Covariance, 1/* from lower to higher eigen values */);

        float2 eigenValues;
        es.GetEigenvalues(eigenValues);
        float2 eigenVector;
        es.GetEigenvector(0, eigenVector);

        *outOrigin = Mean;
        *outRadii = Sqrt(Max(eigenValues, float2::Zero));
        *outOrientation = Atan2(eigenVector.y, eigenVector.x);
    }

    static FGaussian2f FromEllipse(const float2& origin, const float2& radii, const float2x2& rotation) NOEXCEPT {
        return FGaussian2f(origin, radii * radii, rotation);
    }

    static FGaussian2f FromQuad(const float2& p0, const float2& p1, const float2& p2, const float2& p3) NOEXCEPT {
        FGaussian2f result;
        result.Mean = (p0 + p1 + p2 + p3) / 4.f;

        const float2 l0 = (p0 - result.Mean);
        const float2 l1 = (p1 - result.Mean);
        const float2 l2 = (p2 - result.Mean);
        const float2 l3 = (p3 - result.Mean);

        const float3 cov = float3(
            l0.x * l0.x + l1.x * l1.x + l2.x * l2.x + l3.x * l3.x,
            l0.x * l0.y + l1.x * l1.y + l2.x * l2.y + l3.x * l3.y,
            l0.y * l0.y + l1.y * l1.y + l2.y * l2.y + l3.y * l3.y) / 4.f;
        result.Covariance = float2x2(cov.xy, cov.yz);
        return result;
    }
};
PPE_ASSUME_TYPE_AS_POD(FGaussian2f);
//----------------------------------------------------------------------------
struct FGaussian3f {
    float3      Mean;
    float3x3    Covariance;

    FGaussian3f() = default;
    FGaussian3f(const float3& mean, const float3& std_dev, const float3x3& rotation) NOEXCEPT
    :   Mean(mean)
    ,   Covariance(rotation.Multiply(MakeDiagonalMatrix(std_dev)).Multiply(rotation.Transpose()))
    {}

    void ToEllipsoid(float3* outOrigin, float3* outRadii, float3x3* outRotation, const size_t maxIterations = 50) const NOEXCEPT {
        TSymmetricEigensolver<float, 3> es(maxIterations);
        es.Solve(Covariance, 1/* from lower to higher eigen values */);

        es.GetEigenvalues(*outRadii);
        es.GetEigenvectors(*outRotation);

        *outOrigin = Mean;
        *outRadii = Sqrt(Max(*outRadii, float3::Zero));
    }

    static FGaussian3f FromEllipsoid(const float3& origin, const float3& radii, const float3x3& rotation) NOEXCEPT {
        return FGaussian3f(origin, radii * radii, rotation);
    }
};
PPE_ASSUME_TYPE_AS_POD(FGaussian3f);
//----------------------------------------------------------------------------
struct FGaussianRandom3f {
    float3      Mean;
    float3x3    L;

    FGaussianRandom3f() = default;
    FGaussianRandom3f(const FGaussian3f& g) NOEXCEPT
    :   Mean(g.Mean)
    ,   L(float3x3::Zero()) {
        // Cholesky decomposition to extract lower triangular matrix L
        L.SetDiagonal(Sqrt(g.Covariance.Diagonal()));

        float L10_sum = g.Covariance[1][0];
        L10_sum -= L[1][0] * L[0][0];
        L[1][0] = L10_sum / L[0][0];

        float L20_sum = g.Covariance[2][0];
        L20_sum -= L[2][0] * L[0][0];
        float L21_sum = g.Covariance[2][1];
        L21_sum -= L[2][1] * L[1][1];
        L[2][0] = L20_sum / L[0][0];
        L[2][1] = L21_sum / L[1][1];
    }

    float3 BoxMullerTransform(const float4& v01) const {
        // Sample from standard normal using Box-Muller transform
        const float z1 = Sqrt(-2.f * Log(v01.x)) * Cos(2.f * PI * v01.y);
        const float z2 = Sqrt(-2.f * Log(v01.x)) * Sin(2.f * PI * v01.y);
        const float z3 = Sqrt(-2.f * Log(v01.z)) * Sin(2.f * PI * v01.w);

        // Transform to desired distribution
        return (L.Multiply(float3(z1, z2, z3)) + Mean);
    }
};
PPE_ASSUME_TYPE_AS_POD(FGaussianRandom3f);
//----------------------------------------------------------------------------
struct FGaussianSampler2f {
    float2      Mean;
    float       Normalization;
    float2x2    InvCovariance;

    FGaussianSampler2f() = default;
    explicit FGaussianSampler2f(const FGaussian2f& g) NOEXCEPT
    :   Mean(g.Mean)
    ,   InvCovariance(Invert(g.Covariance)) {
        const float detCovariance = Det(g.Covariance);
        Normalization = Rcp(2.f * PI * Sqrt(detCovariance) + Epsilon);
    }

    float PDF(const float2& x) const NOEXCEPT {
        const float2 diff = (x - Mean);
        const float power = -0.5f * Dot(diff, InvCovariance.Multiply(diff));
        if (Ensure(power < 0.f))
            return Exp(power) * Normalization;
        return 0.f;
    }

    float3 Derivative(const float2& x) const NOEXCEPT {
        const float2 term1 = -InvCovariance.Multiply(x - Mean);
        const float term2 = PDF(x);
        return float3(term1, term2);
    }
};
PPE_ASSUME_TYPE_AS_POD(FGaussianSampler2f);
//----------------------------------------------------------------------------
struct FGaussianSampler3f {
    float3      Mean;
    float       Normalization;
    float3x3    InvCovariance;

    FGaussianSampler3f() = default;
    explicit FGaussianSampler3f(const FGaussian3f& g) NOEXCEPT
    :   Mean(g.Mean)
    ,   InvCovariance(Invert(g.Covariance)) {
        const float detCovariance = Det(g.Covariance);
        Normalization = RSqrt(Pow(2.f * PI, 3.f) * detCovariance + Epsilon);
    }

    float PDF(const float3& x) const NOEXCEPT {
        const float3 diff = (x - Mean);
        const float power = -0.5f * Dot(diff, InvCovariance.Multiply(diff));
        if (Ensure(power < 0.f))
            return Exp(power) * Normalization;
        return 0.f;
    }

    float4 Derivative(const float3& x) const NOEXCEPT {
        const float3 term1 = -InvCovariance.Multiply(x - Mean);
        const float term2 = PDF(x);
        return float4(term1, term2);
    }
};
PPE_ASSUME_TYPE_AS_POD(FGaussianSampler3f);
//----------------------------------------------------------------------------
struct FGaussianMixtureModels3f {
    VECTOR(Maths, FGaussian3f) Mixtures;
    VECTOR(Maths, VECTOR(Maths, float)) Responsabilities;
    VECTOR(Maths, float) Weights;

    float LogLikelihood{ 0 };

    PPE_CORE_API explicit FGaussianMixtureModels3f(size_t numSamples, size_t numMixtures = 8);

    size_t NumMixtures() const { return Mixtures.size(); }
    size_t NumSamples() const { return Responsabilities[0].size(); }

    PPE_CORE_API void InitializeFibonacciMixtures(const FGaussian3f& g);
    PPE_CORE_API void InitializeRandomMixtures(FRandomGenerator& rng, const FGaussian3f& g);
    PPE_CORE_API void InitializeRandomMixtures(FRandomGenerator& rng, const float3& radii, const TMemoryView<const float3>& samples);

    bool Solve(const TMemoryView<const float3>& samples, size_t maxIterations = 32000, float tolerance = LargeEpsilon) {
        float prevLogLikelihood = 0.f;
        forrange(i, 0, maxIterations) {
            EStep(samples);
            MStep(samples);

            if (Abs(prevLogLikelihood - LogLikelihood) > tolerance)
                return true;

            prevLogLikelihood = LogLikelihood;
        }
        return false; // did not converge
    }

    PPE_CORE_API void EStep(const TMemoryView<const float3>& samples);
    PPE_CORE_API void MStep(const TMemoryView<const float3>& samples);

    PPE_CORE_API void Reset();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
