#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Lighting/DirectionalLight.h"

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

    float Exposure() const { return _exposure; }
    void SetExposure(float value) { _exposure = value; }

    float WhitePoint() const { return _whitePoint; }
    void SetWhitePoint(float value) { _whitePoint = value; }

    DirectionalLight& Sun() { return _sun; }
    const DirectionalLight& Sun() const { return _sun; }

    void Update(const World *world);

private:
    float _exposure;
    float _whitePoint;
    DirectionalLight _sun;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
