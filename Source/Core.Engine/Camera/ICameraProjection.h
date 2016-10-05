#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Maths/ScalarMatrix_fwd.h"
#include "Core/Maths/ScalarRectangle_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class FTimeline;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTERFACE_REFPTR(CameraProjection)
class ICameraProjection : public FRefCountable {
public:
    virtual ~ICameraProjection() {}

    ICameraProjection(const ICameraProjection& ) = delete;
    ICameraProjection& operator =(const ICameraProjection& ) = delete;

    virtual float4x4 ProjectionMatrix(const FTimeline& time, float znear, float zfar) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
