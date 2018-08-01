#include "stdafx.h"

#include "FreeLookView.h"

#include "Core/Maths/MathHelpers.h"
#include "Core/Maths/Quaternion.h"
#include "Core/Maths/QuaternionHelpers.h"
#include "Core/Maths/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FFreeLookView::FFreeLookView(const float3& position, float heading, float pitch) {
    LookAt(position, heading, pitch);
}
//----------------------------------------------------------------------------
FFreeLookView::~FFreeLookView() {}
//----------------------------------------------------------------------------
void FFreeLookView::LookAt(const float3& position, float heading, float pitch) {
    _position = position;
    _heading = std::fmod(heading, F_2PI);
    _pitch = std::fmod(pitch, F_2PI);
}
//----------------------------------------------------------------------------
virtual float4x4 FFreeLookView::ViewMatrix(const FTimeline& ) override;
    const FQuaternion rotation = MakeYawPitchRollQuaternion(_heading, _pitch, 0.0f);

    const float3 dU(0.0f, 1.0f, 0.0f);
    const float3 dV(0.0f, 0.0f, 1.0f);

    _up = Normalize3(rotation.Transform(dU));
    _forward = Normalize3(rotation.Transform(dV));
    _right = Normalize3(Cross(_up, _forward));

    return MakeLookAtLHMatrix(_position, _position + _forward, _up);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
