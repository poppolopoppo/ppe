#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Engine {
class World;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class LightingEnvironment : public RefCountable {
public:
    LightingEnvironment();
    ~LightingEnvironment();

    const float3& SunColor() const { return _sunColor; }
    void SetSunColor(const float3& value) { _sunColor = value; }

    const float3& SunDirection() const { return _sunDirection; }
    void SetSunDirection(const float3& value) { _sunDirection = value; }

    float Exposure() const { return _exposure; }
    void SetExposure(float value) { _exposure = value; }

    float WhitePoint() const { return _whitePoint; }
    void SetWhitePoint(float value) { _whitePoint = value; }

    void Update(const World *world);

private:
    float3 _sunColor;
    float3 _sunDirection;

    float _exposure;
    float _whitePoint;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
