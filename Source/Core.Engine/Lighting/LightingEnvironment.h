#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Lighting/DirectionalLight.h"

#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Engine {
class FWorld;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLightingEnvironment : public FRefCountable {
public:
    FLightingEnvironment();
    ~FLightingEnvironment();

    float Exposure() const { return _exposure; }
    void SetExposure(float value) { _exposure = value; }
    float WhitePoint() const { return _whitePoint; }
    void SetWhitePoint(float value) { _whitePoint = value; }
    FDirectionalLight& Sun() { return _sun; }
    const FDirectionalLight& Sun() const { return _sun; }
private:
    float _exposure;
    float _whitePoint;
    FDirectionalLight _sun;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
