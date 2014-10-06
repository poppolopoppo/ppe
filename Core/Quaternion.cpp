#include "stdafx.h"

#include "Quaternion.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Quaternion Quaternion::Exponential() const {
    float angle = Angle();
    float fsin = std::sin(angle);

    float4 result;

    if (std::abs(fsin) > F_Epsilon) {
        float coeff = fsin / angle;
        result.x() = coeff * _value.x();
        result.y() = coeff * _value.y();
        result.z() = coeff * _value.z();
    }
    else {
        result = _value;
    }

    result.w() = std::cos(angle);
    return Quaternion(result);
}
//----------------------------------------------------------------------------
Quaternion Quaternion::Logarithm() const {
    float4 result;

    if (std::abs(_value.w()) < 1.0f) {
        float angle = std::acos(_value.w());
        float fsin = std::sin(angle);

        if (std::abs(fsin) > F_Epsilon) {
            float coeff = angle / fsin;
            result.x() = coeff * _value.x();
            result.y() = coeff * _value.y();
            result.z() = coeff * _value.z();
        }
        else {
            result = _value;
        }
    }
    else {
        result = _value;
    }

    result.w() = 0.0f;
    return Quaternion(result);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
