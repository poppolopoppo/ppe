#pragma once

#include "Core/Core.h"

#include "Core/Maths/SHSample.h"
#include "Core/Maths/SHVector_fwd.h"

#include "Core/Container/RawStorage.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SHRotation {
public:
    explicit SHRotation(size_t bands);
    ~SHRotation();

    SHRotation(const SHRotation& ) = delete;
    SHRotation& operator =(const SHRotation& ) = delete;

    size_t Bands() const { return _bands; }

    MemoryView<SHScalar> Matrices() { return MakeView(_matrices.begin(), _matrices.end()); }
    const RAWSTORAGE(Maths, SHScalar)& Matrices() const { return _matrices; }

    void Transform(SHVector<1> *dst, const SHVector<1>& src) const;
    void Transform(SHVector<2> *dst, const SHVector<2>& src) const;
    void Transform(SHVector<3> *dst, const SHVector<3>& src) const;
    void Transform(SHVector<4> *dst, const SHVector<4>& src) const;

    static void AroundX(SHRotation *rotation, SHScalar radians);
    static void AroundY(SHRotation *rotation, SHScalar radians);
    static void AroundZ(SHRotation *rotation, SHScalar radians);

private:
    size_t _bands;
    RAWSTORAGE(Maths, SHScalar) _matrices;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
