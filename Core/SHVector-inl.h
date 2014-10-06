#pragma once

#include "SHVector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
SHVector<_Dim>::SHVector(size_t bands)
:   _bands(bands) {
    Assert(0 < bands);
    _coefficients.Resize_DiscardData(_bands * _bands);
}
//----------------------------------------------------------------------------
template <size_t _Dim>
SHVector<_Dim>::~SHVector() {}
//----------------------------------------------------------------------------
template <size_t _Dim>
void SHVector<_Dim>::Reset(SHScalar broadcast /* = 0 */) {
    for (shcoefficient_type& coeff : _coefficients)
        coeff.Broadcast(broadcast);
}
//----------------------------------------------------------------------------
template <size_t _Dim>
void SHVector<_Dim>::Reset(const shcoefficient_type& value) {
    for (shcoefficient_type& coeff : _coefficients)
        coeff = value;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
