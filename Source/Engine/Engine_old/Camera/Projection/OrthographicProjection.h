#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Camera/ICameraProjection.h"

#include "Core/Maths/ScalarMatrix_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class FTimeline;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(OrthographicProjection)
class FOrthographicProjection : public ICameraProjection {
public:
    virtual float4x4 ProjectionMatrix(  const FTimeline& time,
                                        float znear, float zfar,
                                        const ViewportF& viewport ) override;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
