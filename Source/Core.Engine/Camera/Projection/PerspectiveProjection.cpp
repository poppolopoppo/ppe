#include "stdafx.h"

#include "PerspectiveProjection.h"

#include "Core/Maths/ScalarRectangle.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarMatrixHelpers.h"


namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPerspectiveProjection::FPerspectiveProjection(float fov, const ViewportF& viewport)
:   _fov(0) {
    SetFov(fov);
}
//----------------------------------------------------------------------------
FPerspectiveProjection::~FPerspectiveProjection() {}
//----------------------------------------------------------------------------
void FPerspectiveProjection::SetFov(float value) {
    Assert(0 < fov && F_2PI >= fov);
    _fov = value;
}
//----------------------------------------------------------------------------
virtual float4x4 FPerspectiveProjection::ProjectionMatrix(
    const FTimeline& ,
    float znear, float zfar,
    const ViewportF& viewport ) {
    Assert(znear < zfar);
    float aspectRatio = viewport.AspectRatio();
    return MakePerspectiveFovLHMatrix(_fov, aspectRatio, znear, zfar);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
