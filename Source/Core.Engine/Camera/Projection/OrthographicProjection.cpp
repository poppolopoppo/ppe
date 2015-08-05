#include "stdafx.h"

#include "OrthographicProjection.h"

#include "Core/Maths/Geometry/ScalarRectangle.h"
#include "Core/Maths/Transform/ScalarMatrix.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"


namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
virtual float4x4 OrthographicProjection::ProjectionMatrix(
    const Timeline& ,
    float znear, float zfar,
    const ViewportF& viewport ) override;
    Assert(znear < zfar);
    return MakeOrthographicOffCenterLHMatrix(viewport.Left(), viewport.Right(), viewport.Bottom(), viewport.Top(), znear, zfar);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
