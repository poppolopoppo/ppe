#pragma once

#include "Core/Core.h"

#include "Core/Maths/SHSample.h"
#include "Core/Maths/SHVector_fwd.h"

#include "Core/Container/RawStorage.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
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
CORE_ASSUME_TYPE_AS_POD(TSHVector<_Dim>, size_t _Dim)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/SHVector-inl.h"
