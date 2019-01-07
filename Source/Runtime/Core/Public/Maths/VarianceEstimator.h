#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformMaths.h"
#include "Maths/MathHelpers.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Normal distribution
// https://en.wikipedia.org/wiki/Normal_distribution
//----------------------------------------------------------------------------
template <typename T>
struct TNormalDistribution {
    T Mean;
    T Variance;
    T Scale;
    T StandardDeviation;

    TNormalDistribution(T mean, T variance)
    :   Mean(mean)
    ,   Variance(variance) {
        Assert_NoAssume(Variance > T(0));
        Scale = RSqrt(F_2PI * Variance);
        StandardDeviation = Sqrt(Variance);
    }

    float PDF(T value) const {
        return float(Scale * Exp(-(Sqr(value - Mean) / (2 * Variance))));
    }

    // confidence interval :

    T Low() const { return (Mean - 2 * StandardDeviation); } // <=> 95%
    T High() const { return (Mean + 2 * StandardDeviation); }

    T Min() const { return (Mean - 3 * StandardDeviation); } // <=> 99.7%
    T Max() const { return (Mean + 3 * StandardDeviation); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Welford's Online algorithm
// https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
//----------------------------------------------------------------------------
template <typename T>
struct TVarianceEstimator {
    u64 Count{ 0 };
    T Mean{ 0 }, M2{ 0 };

    void Add(T x) {
        ++Count;

        const T delta = (x - Mean);
        Mean += delta / Count;

        const T delta2 = (x - Mean);
        M2 += (delta * delta2);
    }

    void Reset() {
        Count = 0;
        Mean = M2 = 0;
    }

    T Variance() const {
        Assert_NoAssume(Count);
        return (M2 / Count);
    }

    T SampleVariance() const {
        Assert_NoAssume(Count > 1);
        return (M2 / (Count - 1));
    }

    TNormalDistribution<T> MakeNormalDistribution() const {
        return TNormalDistribution<T>{ Mean, Variance() };
    }
};
//----------------------------------------------------------------------------
using FVarianceEstimator = TVarianceEstimator<double>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Algorithm R
// https://en.wikipedia.org/wiki/Reservoir_sampling
//----------------------------------------------------------------------------
template <typename T, u32 _Capacity = 64>
struct TReservoirSampling {
    STATIC_ASSERT(_Capacity > 1);
    STATIC_CONST_INTEGRAL(u32, Capacity, _Capacity);

    u64 Count{ 0 };
    T SampleMin, SampleMax;
    T Samples[Capacity];

    template <typename _Rnd>
    void Add(T sample, _Rnd& rnd) {
        if (Count < Capacity)
            Samples[Count] = sample;
        else {
            const u64 r = u64(rnd(Count + 1));
            if (r < Capacity)
                Samples[r] = sample;

            SampleMin = Min(SampleMin, sample);
            SampleMax = Min(SampleMax, sample);
        }

        Count++;
    }

    void Finalize() {
        Assert_NoAssume(Count >= Capacity);
        std::sort(std::begin(Samples), std::end(Samples));
        SampleMin = Samples[0];
        SampleMax = Samples[_Capacity - 1];
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Approximate Histogram
//----------------------------------------------------------------------------
template <typename T, u32 _Bins, u32 _Samples>
struct TApproximateHistogram {
    STATIC_ASSERT(_Bins> 1);
    STATIC_CONST_INTEGRAL(u32, Bins, _Bins);
    STATIC_CONST_INTEGRAL(u32, Samples, _Samples);

    T ApproximateBias;
    T ApproximateScale;

    TVarianceEstimator<T> Estimator;
    TVarianceEstimator<T> Histogram[Bins];
    TReservoirSampling<T, _Samples> Reservoir;

    u64 NumSamples() const { return Estimator.Count; }

    T Mean() const { return Estimator.Mean; }
    T Variance() const { return Estimator.Variance(); }
    T SampleVariance() const { return Estimator.SampleVariance(); }

    T Low()     const { return Percentile(.02f); }
    T Median()  const { return Percentile(.50f); }
    T High()    const { return Percentile(.98f); }

    T Percentile(float f) const {
        Assert_NoAssume(f >= 0.f && f <= 1.f);
        Assert_NoAssume(Reservoir.Count >= Samples);

        const u64 axis = FPlatformMaths::RoundToInt(f * Reservoir.Count);

        u32 prv = 0;
        u64 total = 0;
        forrange(bin, 0, Bins) {
            if (not Histogram[bin].Count)
                continue;

            Assert_NoAssume(total < axis);

            if (total + Histogram[bin].Count >= axis) {
                if (total) {
                    const float fx = float(axis - total) / Histogram[bin].Count;
                    return Lerp(Histogram[prv].Mean, Histogram[bin].Mean, fx);
                }
                else {
                    return Histogram[bin].Mean;
                }
            }

            prv = bin;
            total += Histogram[bin].Count;
        }

        AssertNotReached();
    }

    T WeightedMean() const {
        Assert_NoAssume(Reservoir.Count >= Samples);

        T mean{ 0 };
        u64 total = 0;
        forrange(bin, 0, Bins) {
            total += Histogram[bin].Count;
            mean += Histogram[bin].Mean * Histogram[bin].Count;
        }

        return (mean / total);
    }

    template <typename _Rnd>
    void AddSample(T sample, _Rnd& rnd) {
        Estimator.Add(sample);
        Reservoir.Add(sample, rnd);

        if (Unlikely(Reservoir.Count == Samples))
            CreateBins();

        if (Likely(Reservoir.Count >= Samples))
            AddHistogram(sample);
    }

    void AddHistogram(T sample) {
        Assert_NoAssume(Reservoir.Count >= Samples);

        const float x = float((sample - ApproximateBias) * ApproximateScale);
        const i32 bin = Clamp(FPlatformMaths::FloorToInt(x * Bins), i32(0), i32(Bins) - 1);

        // the estimator in each bin gives better precision
        Histogram[bin].Add(sample);
    }

    NO_INLINE void CreateBins() {
        Assert_NoAssume(Samples <= Reservoir.Count);

        // sort the samples to filter N% lowest and highest samples
        Reservoir.Finalize();

        // construct a new estimator with filtered reservoir
        TVarianceEstimator<T> lowpass;
        const u32 mErr = 1; // ignore lowest and highest samples
        const u32 mMid = (Samples / 2);
        reverseforrange(r, mErr, mMid) {
            // add lowest, then highest, while diverging from mean value
            lowpass.Add(Reservoir.Samples[r]);
            lowpass.Add(Reservoir.Samples[Samples - r - 1]);
        }

        // extract the confidence interval (95%) from this estimator
        const TNormalDistribution<T> pdf = lowpass.MakeNormalDistribution();
        const T smin = pdf.Low();
        const T smax = pdf.High();
        Assert_NoAssume(smin < smax);

        // deduce the bias and scale to normalize the samples
        ApproximateBias = smin + (smax - smin) / (2 * _Bins);
        ApproximateScale = Rcp(smax - smin);

        // finally construct the histogram bins and add all samples
        for (TVarianceEstimator<T>& histogram : Histogram)
            histogram.Reset();
        for (T sample : Reservoir.Samples)
            AddHistogram(sample);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Kernel Density Estimator
// http://www.neuralengine.org//res/kernel.html
//----------------------------------------------------------------------------
// #TODO : https://github.com/cooperlab/AdaptiveKDE/blob/master/adaptivekde/ssvkernel.py
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
