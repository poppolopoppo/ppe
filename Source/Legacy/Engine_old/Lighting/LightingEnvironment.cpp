﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "LightingEnvironment.h"

#include "World/World.h"

#include <math.h>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLightingEnvironment::FLightingEnvironment()
:   _exposure(powf(2, 3))
,   _whitePoint(11.2f)
,   _sun(ColorRGBA(255, 204, 128, 255), float3(0.0f, 0.31622776601683794f, -0.9486832980505138f), 10.0f) {}
//----------------------------------------------------------------------------
FLightingEnvironment::~FLightingEnvironment() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core