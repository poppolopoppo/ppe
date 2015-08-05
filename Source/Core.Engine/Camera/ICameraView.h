#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Maths/Transform/ScalarMatrix_fwd.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Timeline;
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTERFACE_REFPTR(CameraView)
class ICameraView : public RefCountable {
public:
    virtual ~ICameraView() {}

    ICameraView(const ICameraView& ) = delete;
    ICameraView& operator =(const ICameraView& ) = delete;

    virtual float4x4 ViewMatrix(const Timeline& time) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
