#include "stdafx.h"

#include "DirectionalLight.h"

#include "Core/Maths/Geometry/ScalarVectorHelpers.h"
#include "Core/Maths/Transform/ScalarMatrix.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DirectionalLight::DirectionalLight()
:   _color(Color::White)
,   _direction(float3::Down())
,   _intensity(1.0f) {}
//----------------------------------------------------------------------------
DirectionalLight::~DirectionalLight() {}
//----------------------------------------------------------------------------
DirectionalLight::DirectionalLight(
    const ColorRGBA& color,
    const float3& direction,
    float intensity ) 
:   _color(color)
,   _direction(direction) 
,   _intensity(intensity) {
    Assert(IsNormalized(direction));
    Assert(_intensity > 0);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
