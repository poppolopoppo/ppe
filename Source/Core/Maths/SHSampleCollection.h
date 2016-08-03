#pragma once

#include "Core/Core.h"

#include "Core/Maths/SHSample.h"

#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
class RandomGenerator;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SHSampleCollection {
public:
    explicit SHSampleCollection(size_t bands);
    ~SHSampleCollection();

    SHSampleCollection(const SHSampleCollection& ) = delete;
    SHSampleCollection& operator =(const SHSampleCollection& ) = delete;

    size_t Bands() const { return _bands; }
    size_t SampleCount() const { return _sampleCount; }

    const RAWSTORAGE(Maths, SHSphericalCoord)& ThetaPhi() const { return _thetaPhi; }
    const RAWSTORAGE(Maths, SHDirection)& Direction() const { return _direction; }
    const RAWSTORAGE(Maths, SHScalar)& Coefficients() const { return _coefficients; }

    void GenerateSphericalSamples_JiterredStratification(size_t sampleCount, RandomGenerator& random);

    void Sample(SHSample *sample, size_t i) const {
        Assert(sample);
        Assert(i < _sampleCount);
        sample->ThetaPhi = _thetaPhi[i];
        sample->Direction = _direction[i];
        sample->Coefficients = SampleCoefficients(i);
    }

    MemoryView<const SHScalar> SampleCoefficients(size_t i) const {
        const size_t shCoeffCount = _bands * _bands;
        const SHScalar *shCoeffsPtr = &_coefficients[shCoeffCount * i];
        return MakeView(shCoeffsPtr, shCoeffsPtr + shCoeffCount);
    }

private:
    size_t _bands;
    size_t _sampleCount;

    RAWSTORAGE(Maths, SHSphericalCoord) _thetaPhi;
    RAWSTORAGE(Maths, SHDirection) _direction;
    RAWSTORAGE(Maths, SHScalar) _coefficients;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
