#pragma once

#include "Maths/SHVector.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
TSHVector<_Dim>::TSHVector(size_t bands)
:   _bands(bands) {
    Assert(0 < bands);
    _coefficients.Resize_DiscardData(_bands * _bands);
}
//----------------------------------------------------------------------------
template <size_t _Dim>
TSHVector<_Dim>::~TSHVector() {}
//----------------------------------------------------------------------------
template <size_t _Dim>
void TSHVector<_Dim>::Reset(SHScalar broadcast /* = 0 */) {
    for (shcoefficient_type& coeff : _coefficients)
        coeff.Broadcast(broadcast);
}
//----------------------------------------------------------------------------
template <size_t _Dim>
void TSHVector<_Dim>::Reset(const shcoefficient_type& value) {
    for (shcoefficient_type& coeff : _coefficients)
        coeff = value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE