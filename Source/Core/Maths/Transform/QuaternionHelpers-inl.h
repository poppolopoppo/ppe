#pragma once

#include "Core/Maths/Transform/QuaternionHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline float Dot(const Quaternion& lhs, const Quaternion& rhs) {
    return Dot4(lhs.Value(), rhs.Value());
}
//----------------------------------------------------------------------------
inline Quaternion Lerp(const Quaternion& v0, const Quaternion& v1, float f) {
    float inverse = 1.0f - f;

    float4 result;

    if (Dot(v0, v1) >= 0.0f) {
        result.x() = (inverse * v0.x()) + (f * v1.x());
        result.y() = (inverse * v0.y()) + (f * v1.y());
        result.z() = (inverse * v0.z()) + (f * v1.z());
        result.w() = (inverse * v0.w()) + (f * v1.w());
    }
    else {
        result.x() = (inverse * v0.x()) - (f * v1.x());
        result.y() = (inverse * v0.y()) - (f * v1.y());
        result.z() = (inverse * v0.z()) - (f * v1.z());
        result.w() = (inverse * v0.w()) - (f * v1.w());
    }

    return Quaternion(Normalize4(result));
}
//----------------------------------------------------------------------------
inline Quaternion SLerp(const Quaternion& v0, const Quaternion& v1, float f) {
    float opposite;
    float inverse;
    float dot = Dot(v0, v1);

    if (std::abs(dot) > 1.0f - F_Epsilon) {
        inverse = 1.0f - f;
        opposite = f * (dot < 0 ? -1 : 1);
    }
    else {
        float facos = std::acos(std::abs(dot));
        float invSin = (1.0f / std::sin(facos));

        inverse = std::sin((1.0f - f) * facos) * invSin;
        opposite = std::sin(f * facos) * invSin * (dot < 0 ? -1 : 1);
    }

    float4 result;
    result.x() = (inverse * v0.x()) + (opposite * v1.x());
    result.y() = (inverse * v0.y()) + (opposite * v1.y());
    result.z() = (inverse * v0.z()) + (opposite * v1.z());
    result.w() = (inverse * v0.w()) + (opposite * v1.w());

    return Quaternion(result);
}
//----------------------------------------------------------------------------
inline Quaternion SQuad(const Quaternion& v0, const Quaternion& v1, const Quaternion& v2, const Quaternion& v3, float f) {
    const Quaternion start = SLerp(v0, v3, f);
    const Quaternion end = SLerp(v1, v2, f);
    return SLerp(start, end, 2 * f * (1 - f));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
