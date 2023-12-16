// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "OrthographicProjection.h"

#include "Core/Maths/ScalarRectangle.h"
#include "Core/Maths/ScalarMatrix.h"
#include "Core/Maths/ScalarMatrixHelpers.h"


namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
virtual float4x4 FOrthographicProjection::ProjectionMatrix(
    const FTimeline& ,
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
