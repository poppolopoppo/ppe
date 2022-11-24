// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DirectionalLight.h"

#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDirectionalLight::FDirectionalLight()
:   _color(Color::White)
,   _direction(float3::Down())
,   _intensity(1.0f) {}
//----------------------------------------------------------------------------
FDirectionalLight::~FDirectionalLight() {}
//----------------------------------------------------------------------------
FDirectionalLight::FDirectionalLight(
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
