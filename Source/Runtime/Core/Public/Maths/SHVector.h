#pragma once

#include "Core.h"

#include "Maths/SHSample.h"
#include "Maths/SHVector_fwd.h"

#include "Container/RawStorage.h"
#include "Maths/ScalarVector.h"
#include "Memory/MemoryView.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
class TSHVector {
public:
    STATIC_ASSERT(0 < _Dim);
    enum : size_t { Dim = _Dim };
    typedef TSHCoefficient<_Dim> shcoefficient_type;

    explicit TSHVector(size_t bands);
    ~TSHVector();

    TSHVector(const TSHVector& ) = delete;
    TSHVector& operator =(const TSHVector& ) = delete;

    size_t Bands() const { return _bands; }

    TMemoryView<shcoefficient_type> Coefficients() { return MakeView(_coefficients.begin(), _coefficients.end()); }
    const RAWSTORAGE(Maths, shcoefficient_type)& Coefficients() const { return _coefficients; }

    void Reset(SHScalar broadcast = 0);
    void Reset(const shcoefficient_type& value);

private:
    size_t _bands;
    RAWSTORAGE(Maths, shcoefficient_type) _coefficients;
};
//----------------------------------------------------------------------------
PPE_ASSUME_TEMPLATE_AS_POD(TSHVector<_Dim>, size_t _Dim)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#include "Maths/SHVector-inl.h"
