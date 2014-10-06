#include "stdafx.h"

#include "LightingEnvironment.h"

#include "World.h"

#include <math.h>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LightingEnvironment::LightingEnvironment()
:   _sunColor(1, 1, 1)
,   _sunDirection(Normalize3(float3( 0,1,-3)))
,   _exposure(powf(2, 3))
,   _whitePoint(11.2f) {}
//----------------------------------------------------------------------------
LightingEnvironment::~LightingEnvironment() {}
//----------------------------------------------------------------------------
void LightingEnvironment::Update(const World *world) {
    Assert(world);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
