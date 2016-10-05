#include "stdafx.h"

#include "SHFunctionProjector.h"

#include "SHSampleCollection.h"
#include "SHVector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <size_t _Dim>
struct TSHPolarFunctor_ {
    TSHPolarFunction<_Dim> Func;
    TSHCoefficient<_Dim> operator ()(const SHSphericalCoord& thetaPhi, const SHDirection& ) const {
        return (*Func)(thetaPhi);
    }
};
//----------------------------------------------------------------------------
template <size_t _Dim>
struct TSH3DFunctor_ {
    TSH3DFunction<_Dim> Func;
    TSHCoefficient<_Dim> operator ()(const SHSphericalCoord& , const SHDirection& dir) const {
        return (*Func)(dir);
    }
};
//----------------------------------------------------------------------------
template <size_t _Dim, typename _SHFunctor>
static void SHProjectFunctor_(TSHVector<_Dim> *coefficients, const _SHFunctor& functor, const FSHSampleCollection& samples) {
    Assert(coefficients);
    Assert(coefficients->Bands() == samples.Bands());
    Assert(samples.SampleCount());

    const size_t bands = samples.Bands();
    const size_t sampleCount = samples.SampleCount();
    const size_t shCoeffsPerSamples = bands * bands;

    const SHSphericalCoord *sampleThetaPhi = samples.ThetaPhi().Pointer();
    const SHDirection *sampleDir = samples.Direction().Pointer();
    const SHScalar *sampleCoeffs = samples.Coefficients().Pointer();

    const SHScalar scale = static_cast<SHScalar>((4.0*3.1415926535897932384626433832795/* 4 PI */) / sampleCount);

    coefficients->Reset(); // set to zero SH coefficients

    TSHCoefficient<_Dim> *const output = coefficients->Coefficients().Pointer();

    for (size_t i = 0; i < sampleCount; ++i) {
        for (size_t j = 0; j < shCoeffsPerSamples; ++j, ++sampleCoeffs)
            output[j] += functor(*sampleThetaPhi, *sampleDir) * (*sampleCoeffs);

        ++sampleThetaPhi;
        ++sampleDir;
    }

    // normalize the coefficient (based on probability and number of sample)
    for (size_t j = 0; j < shCoeffsPerSamples; ++j)
        output[j] *= scale;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define DEF_SHPROJECTFUNCTION_IMPL(_DIM, _FUNCTYPE) \
    void SHProjectFunction(TSHVector<_DIM> *coefficients, CONCAT(_FUNCTYPE, Function)<_DIM> func, const FSHSampleCollection& samples) { \
        Assert(func); \
        const CONCAT(_FUNCTYPE, Functor_)<_DIM> functor = {func}; \
        SHProjectFunctor_(coefficients, functor, samples); \
    }
//----------------------------------------------------------------------------
DEF_SHPROJECTFUNCTION_IMPL(1, TSHPolar)
DEF_SHPROJECTFUNCTION_IMPL(2, TSHPolar)
DEF_SHPROJECTFUNCTION_IMPL(3, TSHPolar)
DEF_SHPROJECTFUNCTION_IMPL(4, TSHPolar)
//----------------------------------------------------------------------------
DEF_SHPROJECTFUNCTION_IMPL(1, TSH3D)
DEF_SHPROJECTFUNCTION_IMPL(2, TSH3D)
DEF_SHPROJECTFUNCTION_IMPL(3, TSH3D)
DEF_SHPROJECTFUNCTION_IMPL(4, TSH3D)
//----------------------------------------------------------------------------
#undef DEF_SHPROJECTFUNCTION_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
