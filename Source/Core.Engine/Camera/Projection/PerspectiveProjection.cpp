#include "stdafx.h"

#include "PerspectiveProjection.h"

#include "Core/Maths/Geometry/ScalarRectangle.h"
#include "Core/Maths/Transform/ScalarMatrix.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"


namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PerspectiveProjection::PerspectiveProjection(float fov, const ViewportF& viewport)
:   _fov(0) {
    SetFov(fov);
}
//----------------------------------------------------------------------------
PerspectiveProjection::~PerspectiveProjection() {}
//----------------------------------------------------------------------------
void PerspectiveProjection::SetFov(float value) {
    Assert(0 < fov && F_2PI >= fov);
    _fov = value;
}
//----------------------------------------------------------------------------
virtual float4x4 PerspectiveProjection::ProjectionMatrix(
    const Timeline& ,
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
