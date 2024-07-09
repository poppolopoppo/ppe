// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Maths/Gaussian.h"

#include "Maths/PackedVectors.h"

#include "Thread/Task.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGaussianMixtureModels3f::FGaussianMixtureModels3f(size_t numSamples, size_t numMixtures)
:   Mixtures(numMixtures)
,   Responsabilities(numMixtures, VECTOR(Maths, float)(numSamples, 0.f))
,   Weights(numMixtures, Rcp(float(numMixtures))) {
    Assert_NoAssume(numSamples > 0);
    Assert_NoAssume(numMixtures > 0);
}
//----------------------------------------------------------------------------
void FGaussianMixtureModels3f::InitializeFibonacciMixtures(const FGaussian3f& g) {
    const u32 N = checked_cast<u32>(Mixtures.size());

    FGaussianRandom3f dist(g);
    forrange(i, 0, N) {
        const float3 uvw = FibonacciSphereNormalDecode(i, N);

        FGaussian3f& mixture = Mixtures[i];
        mixture.Mean = dist.BoxMullerTransform(float4(uvw, float(i)/N));
        mixture.Covariance = g.Covariance;
    }
}
//----------------------------------------------------------------------------
void FGaussianMixtureModels3f::InitializeRandomMixtures(FRandomGenerator& rng, const FGaussian3f& g) {
    FGaussianRandom3f dist(g);

    for (FGaussian3f& mixture : Mixtures) {
        float4 rnd;
        rng.Randomize(rnd);
        mixture.Mean = dist.BoxMullerTransform((rnd + 1.f) / 2.f);
        mixture.Covariance = g.Covariance;
    }
}
//----------------------------------------------------------------------------
void FGaussianMixtureModels3f::InitializeRandomMixtures(FRandomGenerator& rng, const float3& radii, const TMemoryView<const float3>& samples) {
    Assert_NoAssume(samples.size() == NumSamples());
    STACKLOCAL_POD_ARRAY(u32, reservoir, Mixtures.size());

    // generate X uniq random sample indices
    rng.RandomizeUniq(reservoir, checked_cast<u32>(samples.size()));

    const FGaussian3f g = FGaussian3f::FromEllipsoid(float3::Zero, radii, float3x3::Identity());

    forrange(i, 0, Mixtures.size()) {
        FGaussian3f& mixture = Mixtures[i];
        mixture.Mean = samples[reservoir[i]];
        mixture.Covariance = g.Covariance;
    }
}
//----------------------------------------------------------------------------
void FGaussianMixtureModels3f::EStep(const TMemoryView<const float3>& samples) {
    Assert_NoAssume(samples.size() == NumSamples());
    STACKLOCAL_POD_ARRAY(FGaussianSampler3f, samplers, Mixtures.size());
    forrange(k, 0, Mixtures.size())
        samplers[k] = FGaussianSampler3f(Mixtures[k]);

    ParallelFor(0, Responsabilities.size(),
        [&](size_t i) {
            const float3 x = samples[i];

            float totalPDF = 0.f;
            forrange(k, 0, Mixtures.size()) {
                float pdf = samplers[k].PDF(x) * Weights[k];
                pdf = Max(pdf, Epsilon_v<float>); // avoid singularity

                totalPDF += pdf;
                Responsabilities[k][i] = pdf;
            }

            forrange(k, 0, Mixtures.size())
                Responsabilities[k][i] /= totalPDF;
        });
}
//----------------------------------------------------------------------------
void FGaussianMixtureModels3f::MStep(const TMemoryView<const float3>& samples) {
    Assert_NoAssume(samples.size() == NumSamples());
    forrange(k, 0, Mixtures.size())
        Mixtures[k] = FGaussian3f();

    ParallelFor(0, Mixtures.size(),
        [&](size_t k) {
            // Update mean
            float4 mean_likelihood = ParallelMapReduce(0, Responsabilities.size(),
                [&](size_t i) -> float4 {
                    return float4(samples[i], 1.f) * Responsabilities[k][i];
                },
                [](const float4& lhs, const float4& rhs) -> float4 {
                    return (lhs + rhs);
                });

            mean_likelihood.w = Max(mean_likelihood.w, Epsilon_v<float>); // avoid singularity
            Mixtures[k].Mean = (mean_likelihood.xyz / mean_likelihood.w);

            // Update covariance
            const float3x3 covariance = ParallelMapReduce(0, Responsabilities.size(),
                [&](size_t i) -> float3x3 {
                    const float3 diff = (samples[i] - Mixtures[k].Mean);
                    return OuterProduct(diff, diff) * Responsabilities[k][i];
                },
                [](const float3x3& lhs, const float3x3& rhs) -> float3x3 {
                    return (lhs + rhs);
                });

            Mixtures[k].Covariance = (covariance / mean_likelihood.w);
            Mixtures[k].Covariance += Epsilon; // avoid singularity

            // Update mixture weight
            Weights[k] = (mean_likelihood.w / samples.size());
        });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
