// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Viewport/Camera.h"

#include "Maths/ScalarMatrixHelpers.h"
#include "Maths/ScalarRectangle.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FCamera::UpdateModel(const FCameraModel& model, const FRectangle2f& viewport) NOEXCEPT {
    Assert_NoAssume(not IsNANorINF(model.Right));
    Assert_NoAssume(not IsNANorINF(model.Up));
    Assert_NoAssume(not IsNANorINF(model.Forward));
    Assert_NoAssume(not IsNANorINF(model.Position));
    Assert_NoAssume(not IsNANorINF(model.ZNear));
    Assert_NoAssume(not IsNANorINF(model.ZFar));
    Assert_NoAssume(IsNormalized(model.Right));
    Assert_NoAssume(IsNormalized(model.Up));
    Assert_NoAssume(IsNormalized(model.Forward));
    Assert_NoAssume(model.Fov >= 0.f && model.Fov <= 2.f * PI);
    Assert_NoAssume(model.ZNear < model.ZFar);

    const auto exclusiveData = _data.LockExclusive();
    exclusiveData->PreviousState = exclusiveData->CurrentState;

    const float3 cameraTarget{ model.Position + model.Forward };

    exclusiveData->CurrentState.Model = model;
    exclusiveData->CurrentState.View = MakeLookAtRHMatrix(model.Position, cameraTarget, model.Up);
    Assert_NoAssume(not IsNANorINF(exclusiveData->CurrentState.View));

    switch (exclusiveData->Mode) {
    case ECameraProjection::Orthographic:
        exclusiveData->CurrentState.Projection = MakeOrthographicOffCenterRHMatrix(
            viewport.Left(),
            viewport.Right(),
            viewport.Bottom(),
            viewport.Top(),
            model.ZNear,
            model.ZFar);
        break;

    case ECameraProjection::Perspective:
        exclusiveData->CurrentState.Projection = MakePerspectiveFovRHMatrix(
            model.Fov,
            viewport.AspectRatio(),
            model.ZNear,
            model.ZFar);
        break;
    }
    Assert_NoAssume(not IsNANorINF(exclusiveData->CurrentState.Projection));

    const float4x4 viewProjection = exclusiveData->CurrentState.View * exclusiveData->CurrentState.Projection;
    Assert_NoAssume(not IsNANorINF(viewProjection));
    exclusiveData->CurrentState.Frustum.SetMatrix(viewProjection);

    exclusiveData->CurrentState.InvertView = Invert(exclusiveData->CurrentState.View);
    exclusiveData->CurrentState.InvertProjection = Invert(exclusiveData->CurrentState.Projection);
    exclusiveData->CurrentState.InvertViewProjection = Invert(viewProjection);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
