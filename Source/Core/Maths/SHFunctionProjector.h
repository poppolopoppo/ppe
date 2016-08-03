#pragma once

#include "Core/Core.h"

#include "Core/Maths/SHSample.h"
#include "Core/Maths/SHVector_fwd.h"

namespace Core {
class SHSampleCollection;

// Spherical Harmonic Lighting: The Gritty Details
// http://www.research.scea.com/gdc2003/spherical-harmonic-lighting.html

// Spherical Harmonics Examples
// https://www.shadertoy.com/view/Xds3Rl

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
using SHPolarFunction = SHCoefficient<_Dim> (*)(const SHSphericalCoord& thetaPhi);
//----------------------------------------------------------------------------
void SHProjectFunction(SHVector<1> *coefficients, SHPolarFunction<1> func, const SHSampleCollection& samples);
void SHProjectFunction(SHVector<2> *coefficients, SHPolarFunction<2> func, const SHSampleCollection& samples);
void SHProjectFunction(SHVector<3> *coefficients, SHPolarFunction<3> func, const SHSampleCollection& samples);
void SHProjectFunction(SHVector<4> *coefficients, SHPolarFunction<4> func, const SHSampleCollection& samples);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <size_t _Dim>
using SH3DFunction = SHCoefficient<_Dim> (*)(const SHDirection& dir);
//----------------------------------------------------------------------------
void SHProjectFunction(SHVector<1> *coefficients, SH3DFunction<1> func, const SHSampleCollection& samples);
void SHProjectFunction(SHVector<2> *coefficients, SH3DFunction<2> func, const SHSampleCollection& samples);
void SHProjectFunction(SHVector<3> *coefficients, SH3DFunction<3> func, const SHSampleCollection& samples);
void SHProjectFunction(SHVector<4> *coefficients, SH3DFunction<4> func, const SHSampleCollection& samples);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
