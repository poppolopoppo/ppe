#include "stdafx.h"

#include "SHSampleCollection.h"

#include "Maths/RandomGenerator.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
FORCE_INLINE static size_t SHLmToIndex_(int l, int m) {
    return checked_cast<size_t>(l*(l+1)+m);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SHSampleCollection::SHSampleCollection(size_t bands)
:   _bands(bands), _sampleCount(0) {
    Assert(0 < _bands);
}
//----------------------------------------------------------------------------
SHSampleCollection::~SHSampleCollection() {}
//----------------------------------------------------------------------------
void SHSampleCollection::GenerateSphericalSamples_JiterredStratification(size_t sampleCount, RandomGenerator& random) {
    Assert(0 < sampleCount);
    _sampleCount = sampleCount;

    _thetaPhi.Resize_DiscardData(_sampleCount);
    _direction.Resize_DiscardData(_sampleCount);

    const size_t shCoeffsPerSample = _bands * _bands;
    _coefficients.Resize_DiscardData(shCoeffsPerSample * _sampleCount);

    const SHScalar sqrt_nb_samplesF = std::sqrt(SHScalar(_sampleCount));
    const SHScalar oo_sqrt_nb_samplesF = 1/sqrt_nb_samplesF;
    const size_t sqrt_nb_samples = size_t(sqrt_nb_samplesF);

    size_t sample = 0;
    for (size_t a = 0; a < sqrt_nb_samples; ++a)
        for (size_t b = 0; b < sqrt_nb_samples; ++b) {
            // stratified sampling position
            const SHScalar x((a + random.NextFloat01())*oo_sqrt_nb_samplesF);
            const SHScalar y((b + random.NextFloat01())*oo_sqrt_nb_samplesF);

            // spherical coordinate generation
            SHSphericalCoord& thetaPhi = _thetaPhi[sample];
            thetaPhi.x() = 2 * std::acos(std::sqrt(1-x));
            thetaPhi.y() = SHScalar(2 * 3.1415926535897932384626433832795 * y);

            // direction coordinate generation
            SHDirection& direction = _direction[sample];
            direction = SHSphericalCoordToDirection(thetaPhi);

            // pre compute all SH coefficients for this sample
            SHScalar *const coefficients = &_coefficients[sample * shCoeffsPerSample];
            for (int l = 0; l < int(_bands); ++l)
                for (int m = -l; m <= l; ++m) {
                    const size_t idx = SHLmToIndex_(l, m);
                    Assert(idx < shCoeffsPerSample);
                    coefficients[idx] = SHPointSampleFunction(l, m, thetaPhi);
                }

            ++sample;
        }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
