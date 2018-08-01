#pragma once

#include "Core.h"

#include "Maths/SHSample.h"

#include "Container/RawStorage.h"
#include "Container/Vector.h"
#include "Memory/MemoryView.h"

namespace PPE {
class FRandomGenerator;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSHSampleCollection {
public:
    explicit FSHSampleCollection(size_t bands);
    ~FSHSampleCollection();

    FSHSampleCollection(const FSHSampleCollection& ) = delete;
    FSHSampleCollection& operator =(const FSHSampleCollection& ) = delete;

    size_t Bands() const { return _bands; }
    size_t SampleCount() const { return _sampleCount; }

    const RAWSTORAGE(Maths, SHSphericalCoord)& ThetaPhi() const { return _thetaPhi; }
    const RAWSTORAGE(Maths, SHDirection)& Direction() const { return _direction; }
    const RAWSTORAGE(Maths, SHScalar)& Coefficients() const { return _coefficients; }

    void GenerateSphericalSamples_JiterredStratification(size_t sampleCount, FRandomGenerator& random);

    void Sample(FSHSample *sample, size_t i) const {
        Assert(sample);
        Assert(i < _sampleCount);
        sample->ThetaPhi = _thetaPhi[i];
        sample->Direction = _direction[i];
        sample->Coefficients = SampleCoefficients(i);
    }

    TMemoryView<const SHScalar> SampleCoefficients(size_t i) const {
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
} //!namespace PPE
