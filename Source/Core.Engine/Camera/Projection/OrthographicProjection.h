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
FWD_REFPTR(OrthographicProjection)
class OrthographicProjection : public ICameraProjection {
public:
    virtual float4x4 ProjectionMatrix(  const Timeline& time,
                                        float znear, float zfar,
                                        const ViewportF& viewport ) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
