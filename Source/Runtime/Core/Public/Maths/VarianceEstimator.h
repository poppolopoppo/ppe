#pragma once

#include "Core_fwd.h"

namespace PPE {
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

    void Reset() { Count = 0; }

    T Variance() const {
        Assert_NoAssume(Count);
        return (M2 / Count);
    }

    T SampleVariance() const {
        Assert_NoAssume(Count > 1);
        return (M2 / (Count - 1));
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
template <typename T, size_t _Capacity = 64>
struct TReservoirSampling {
    T Samples[_Capacity];
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
