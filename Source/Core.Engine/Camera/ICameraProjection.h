#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Maths/Transform/ScalarMatrix_fwd.h"
#include "Core/Maths/Transform/ScalarRectangle_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Timeline;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTERFACE_REFPTR(CameraProjection)
class ICameraProjection : public RefCountable {
public:
    virtual ~ICameraProjection() {}

    ICameraProjection(const ICameraProjection& ) = delete;
    ICameraProjection& operator =(const ICameraProjection& ) = delete;

    virtual float4x4 ProjectionMatrix(const Timeline& time, float znear, float zfar) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
