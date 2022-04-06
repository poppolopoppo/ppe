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
        Scale = RSqrt(static_cast<T>(D_2PI) * Variance);
        StandardDeviation = Sqrt(Variance);
    }

    float PDF(T value) const {
        return float(Scale * Exp(-(Sqr(value - Mean) / (2 * Variance))));
    }

    // confidence interval :

    T Low() const   { return (Mean - 2 * StandardDeviation); } // <=> 95%
    T High() const  { return (Mean + 2 * StandardDeviation); }

    T Min() const   { return (Mean - 3 * StandardDeviation); } // <=> 99.7%
    T Max() const   { return (Mean + 3 * StandardDeviation); }
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

    // following central limit theorem
    // https://en.wikipedia.org/wiki/Central_limit_theorem
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

    u32 Count{ 0 };
    T SampleMin, SampleMax;
    T Samples[Capacity];

    template <typename _Rnd>
    void Add(T sample, _Rnd& rnd) {
        if (Count < Capacity)
            Samples[Count] = sample;
        else {
            const u32 r = checked_cast<u32>(rnd(size_t(Count) + 1));
            if (r < Capacity)
                Samples[r] = sample;

            SampleMin = Min(SampleMin, sample);
            SampleMax = Min(SampleMax, sample);
        }

        Count++;
    }

    void Clear() {
        Count = 0;
        SampleMin = SampleMax = {};
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
    STATIC_CONST_INTEGRAL(u32, MinSamples, _Samples * 3);

    T ApproximateBias{ 0 };
    T ApproximateScale{ 0 };

    u32 NumSamples{ 0 };
    TVarianceEstimator<T> Estimator;
    TVarianceEstimator<T> Histogram[Bins];
    TReservoirSampling<T, _Samples> Reservoir;

    T Mean() const { return Estimator.Mean; }
    T Variance() const { return Estimator.Variance(); }
    T SampleVariance() const { return Estimator.SampleVariance(); }

    T Min()     const { return Percentile(.00f); }
    T Q1()      const { return Percentile(.25f); }
    T Median()  const { return Percentile(.50f); }
    T Q3()      const { return Percentile(.75f); }
    T Max()     const { return Percentile(1.0f); }

    T Mode() const {
        Assert_NoAssume(Reservoir.Count >= MinSamples);
        Assert_NoAssume(NumSamples >= Samples);

        u32 mx = 0;
        forrange(bin, 1, Bins) {
            if (Histogram[bin].Count > Histogram[mx].Count)
                mx = bin;
        }

        return Histogram[mx].Mean;
    }

    T WeightedMean() const {
        Assert_NoAssume(Reservoir.Count >= MinSamples);
        Assert_NoAssume(NumSamples >= Samples);

        T mean{ 0 };
        ONLY_IF_ASSERT(u64 totalForDbg = 0);
        forrange(bin, 0, Bins) {
            ONLY_IF_ASSERT(totalForDbg += Histogram[bin].Count);
            mean += Histogram[bin].Mean * Histogram[bin].Count;
        }

        Assert_NoAssume(totalForDbg == NumSamples);

        return (mean / NumSamples);
    }

    T Percentile(float f) const {
        Assert_NoAssume(f >= 0.f && f <= 1.f);
        Assert_NoAssume(Reservoir.Count >= MinSamples);
        Assert_NoAssume(NumSamples >= Samples);

        const u64 axis = checked_cast<u64>(RoundToInt(f * NumSamples));

        u32 prv = 0;
        u64 total = 0;
        forrange(bin, 0, Bins) {
            if (not Histogram[bin].Count)
                continue;

            Assert_NoAssume(total <= axis);

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

    template <typename _Rnd>
    void AddSample(T sample, _Rnd& rnd) {
        Estimator.Add(sample);
        Reservoir.Add(sample, rnd);

        if (Unlikely(Reservoir.Count == MinSamples))
            CreateBins();

        if (Likely(Reservoir.Count >= MinSamples))
            AddHistogram(sample);
    }

    void AddHistogram(T sample) {
        Assert_NoAssume(Reservoir.Count >= MinSamples);

        const float f = float((sample - ApproximateBias) * ApproximateScale);
        const i32 bin = Clamp(FloorToInt(f * Bins), i32(0), i32(Bins) - 1);

        // the estimator in each bin gives better precision
        Histogram[bin].Add(sample);

        // increment the number of samples present in the histogram
        NumSamples++;
    }

    NO_INLINE void CreateBins() {
        Assert_NoAssume(Reservoir.Count >= MinSamples);

        // extract the confidence interval (95%) from global estimator
        const TNormalDistribution<T> pdf = Estimator.MakeNormalDistribution();
        const T smin = pdf.Low();
        const T smax = pdf.High();
        Assert_NoAssume(smin < smax);

        // deduce the bias and scale to normalize the samples
        ApproximateBias = smin + (smax - smin) / (T(2) * _Bins);
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
// #TODO : requires FFT
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
