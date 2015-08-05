#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Camera/ICameraProjection.h"

#include "Core/Maths/Transform/ScalarMatrix_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Timeline;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(PerspectiveProjection)
class PerspectiveProjection : public ICameraProjection {
public:
    PerspectiveCamera(float fov/* rad */);

    float Fov() const { return _fov; }
    void SetFov(float value);

    virtual float4x4 ProjectionMatrix(  const Timeline& time,
                                        float znear, float zfar,
                                        const ViewportF& viewport ) override;

private:
    float _fov; // rad
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
