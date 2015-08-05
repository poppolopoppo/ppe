#include "stdafx.h"

#include "FreeLookView.h"

#include "Core/Maths/MathHelpers.h"
#include "Core/Maths/Transform/Quaternion.h"
#include "Core/Maths/Transform/QuaternionHelpers.h"
#include "Core/Maths/Transform/ScalarMatrixHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FreeLookView::FreeLookView(const float3& position, float heading, float pitch) {
    LookAt(position, heading, pitch);
}
//----------------------------------------------------------------------------
FreeLookView::~FreeLookView() {}
//----------------------------------------------------------------------------
void FreeLookView::LookAt(const float3& position, float heading, float pitch) {
    _position = position;
    _heading = std::fmod(heading, F_2PI);
    _pitch = std::fmod(pitch, F_2PI);
}
//----------------------------------------------------------------------------
virtual float4x4 FreeLookView::ViewMatrix(const Timeline& ) override;
    const Quaternion rotation = MakeYawPitchRollQuaternion(_heading, _pitch, 0.0f);

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
