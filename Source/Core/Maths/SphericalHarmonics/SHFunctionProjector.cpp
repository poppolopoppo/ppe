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
struct SHPolarFunctor_ {
    SHPolarFunction<_Dim> Func;
    SHCoefficient<_Dim> operator ()(const SHSphericalCoord& thetaPhi, const SHDirection& ) const {
        return (*Func)(thetaPhi);
    }
};
//----------------------------------------------------------------------------
template <size_t _Dim>
struct SH3DFunctor_ {
    SH3DFunction<_Dim> Func;
    SHCoefficient<_Dim> operator ()(const SHSphericalCoord& , const SHDirection& dir) const {
        return (*Func)(dir);
    }
};
//----------------------------------------------------------------------------
template <size_t _Dim, typename _SHFunctor>
static void SHProjectFunctor_(SHVector<_Dim> *coefficients, const _SHFunctor& functor, const SHSampleCollection& samples) {
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

    SHCoefficient<_Dim> *const output = coefficients->Coefficients().Pointer();

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
    void SHProjectFunction(SHVector<_DIM> *coefficients, CONCAT(_FUNCTYPE, Function)<_DIM> func, const SHSampleCollection& samples) { \
        Assert(func); \
        const CONCAT(_FUNCTYPE, Functor_)<_DIM> functor = {func}; \
        SHProjectFunctor_(coefficients, functor, samples); \
    }
//----------------------------------------------------------------------------
DEF_SHPROJECTFUNCTION_IMPL(1, SHPolar)
DEF_SHPROJECTFUNCTION_IMPL(2, SHPolar)
DEF_SHPROJECTFUNCTION_IMPL(3, SHPolar)
DEF_SHPROJECTFUNCTION_IMPL(4, SHPolar)
//----------------------------------------------------------------------------
DEF_SHPROJECTFUNCTION_IMPL(1, SH3D)
DEF_SHPROJECTFUNCTION_IMPL(2, SH3D)
DEF_SHPROJECTFUNCTION_IMPL(3, SH3D)
DEF_SHPROJECTFUNCTION_IMPL(4, SH3D)
//----------------------------------------------------------------------------
#undef DEF_SHPROJECTFUNCTION_IMPL
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
