#pragma once

#include "Core.h"

#include "SHSample.h"
#include "SHVector_fwd.h"

#include "MemoryView.h"
#include "ScalarVector.h"
#include "RawStorage.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
class SHVector {
public:
    STATIC_ASSERT(0 < _Dim);
    enum : size_t { Dim = _Dim };
    typedef SHCoefficient<_Dim> shcoefficient_type;

    explicit SHVector(size_t bands);
    ~SHVector();

    SHVector(const SHVector& ) = delete;
    SHVector& operator =(const SHVector& ) = delete;

    size_t Bands() const { return _bands; }

    MemoryView<shcoefficient_type> Coefficients() { return MakeView(_coefficients.begin(), _coefficients.end()); }
    const RAWSTORAGE(Maths, shcoefficient_type)& Coefficients() const { return _coefficients; }

    void Reset(SHScalar broadcast = 0);
    void Reset(const shcoefficient_type& value);

private:
    size_t _bands;
    RAWSTORAGE(Maths, shcoefficient_type) _coefficients;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "SHVector-inl.h"
