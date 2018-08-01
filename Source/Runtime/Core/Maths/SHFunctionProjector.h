#pragma once

#include "Core/Core.h"

#include "Core/Maths/SHSample.h"
#include "Core/Maths/SHVector_fwd.h"

namespace Core {
class FSHSampleCollection;

// Spherical Harmonic Lighting: The Gritty Details
// http://www.research.scea.com/gdc2003/spherical-harmonic-lighting.html

// Spherical Harmonics Examples
// https://www.shadertoy.com/view/Xds3Rl

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
using TSHPolarFunction = TSHCoefficient<_Dim> (*)(const SHSphericalCoord& thetaPhi);
//----------------------------------------------------------------------------
void SHProjectFunction(TSHVector<1> *coefficients, TSHPolarFunction<1> func, const FSHSampleCollection& samples);
void SHProjectFunction(TSHVector<2> *coefficients, TSHPolarFunction<2> func, const FSHSampleCollection& samples);
void SHProjectFunction(TSHVector<3> *coefficients, TSHPolarFunction<3> func, const FSHSampleCollection& samples);
void SHProjectFunction(TSHVector<4> *coefficients, TSHPolarFunction<4> func, const FSHSampleCollection& samples);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
using TSH3DFunction = TSHCoefficient<_Dim> (*)(const SHDirection& dir);
//----------------------------------------------------------------------------
void SHProjectFunction(TSHVector<1> *coefficients, TSH3DFunction<1> func, const FSHSampleCollection& samples);
void SHProjectFunction(TSHVector<2> *coefficients, TSH3DFunction<2> func, const FSHSampleCollection& samples);
void SHProjectFunction(TSHVector<3> *coefficients, TSH3DFunction<3> func, const FSHSampleCollection& samples);
void SHProjectFunction(TSHVector<4> *coefficients, TSH3DFunction<4> func, const FSHSampleCollection& samples);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
