#pragma once

#include "Maths/SHVector.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <u32 _Dim>
TSHVector<_Dim>::TSHVector(size_t bands)
:   _bands(bands) {
    Assert(0 < bands);
    _coefficients.Resize_DiscardData(_bands * _bands);
}
//----------------------------------------------------------------------------
template <u32 _Dim>
TSHVector<_Dim>::~TSHVector() = default;
//----------------------------------------------------------------------------
template <u32 _Dim>
void TSHVector<_Dim>::Reset(SHScalar broadcast /* = 0 */) {
    for (shcoefficient_type& coeff : _coefficients)
        coeff.Broadcast(broadcast);
}
//----------------------------------------------------------------------------
template <u32 _Dim>
void TSHVector<_Dim>::Reset(const shcoefficient_type& value) {
    for (shcoefficient_type& coeff : _coefficients)
        coeff = value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
