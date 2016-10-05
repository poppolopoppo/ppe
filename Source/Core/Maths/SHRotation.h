#pragma once

#include "Core/Core.h"

#include "Core/Maths/SHSample.h"
#include "Core/Maths/SHVector_fwd.h"

#include "Core/Container/RawStorage.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSHRotation {
public:
    explicit FSHRotation(size_t bands);
    ~FSHRotation();

    FSHRotation(const FSHRotation& ) = delete;
    FSHRotation& operator =(const FSHRotation& ) = delete;

    size_t Bands() const { return _bands; }

    TMemoryView<SHScalar> Matrices() { return MakeView(_matrices.begin(), _matrices.end()); }
    const RAWSTORAGE(Maths, SHScalar)& Matrices() const { return _matrices; }

    void Transform(TSHVector<1> *dst, const TSHVector<1>& src) const;
    void Transform(TSHVector<2> *dst, const TSHVector<2>& src) const;
    void Transform(TSHVector<3> *dst, const TSHVector<3>& src) const;
    void Transform(TSHVector<4> *dst, const TSHVector<4>& src) const;

    static void AroundX(FSHRotation *rotation, SHScalar radians);
    static void AroundY(FSHRotation *rotation, SHScalar radians);
    static void AroundZ(FSHRotation *rotation, SHScalar radians);

private:
    size_t _bands;
    RAWSTORAGE(Maths, SHScalar) _matrices;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
