#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Color/Color.h"
#include "Core/Maths/ScalarMatrix_fwd.h"
#include "Core/Maths/ScalarVector.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DirectionalLight {
public:
    DirectionalLight();
    ~DirectionalLight();

    DirectionalLight(   const ColorRGBA& color,
                        const float3& direction,
                        float intensity );

    const ColorRGBA& Color() const { return _color; }
    void SetColor(const ColorRGBA& color) { _color = color; }

    const float3& Direction() const { return _direction; }
    void SetDirection(const float3& direction) { _direction = direction; }

    float Intensity() const { return _intensity; }
    void SetIntensity(float intensity) { _intensity = intensity; }

private:
    ColorRGBA _color;
    float3 _direction;
    float _intensity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
