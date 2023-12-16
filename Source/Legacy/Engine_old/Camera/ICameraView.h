#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Maths/ScalarMatrix_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class FTimeline;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTERFACE_REFPTR(CameraView)
class ICameraView : public FRefCountable {
public:
    virtual ~ICameraView() {}

    ICameraView(const ICameraView& ) = delete;
    ICameraView& operator =(const ICameraView& ) = delete;

    virtual float4x4 ViewMatrix(const FTimeline& time) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
