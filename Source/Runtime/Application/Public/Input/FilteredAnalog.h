#pragma once

#include "Application.h"

#include "Maths/MathHelpers.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFilteredAnalog {
public:
    FFilteredAnalog()
        : _raw(0)
        , _filtered(NAN)
        , _sensitivity(1.f)
    {}

    float Raw() const { return _raw; }
    float Filtered() const { return (IsNAN(_filtered) ? _raw : _filtered); }
    float Sensitivity() const { return _sensitivity; }

    void SetRaw(float raw) {
        _raw = raw;
    }
    void SetSensitivity(float s) {
        Assert(s > 1e-3f);
        _sensitivity = s;
    }

    float operator *() const { return Filtered(); }

    void Update(float dt) {
        if (IsNAN(_filtered)) {
            _filtered = _raw;
        }
        else {
            _filtered += (_raw - _filtered) * Pow(dt, 1.f / _sensitivity);
        }
    }

    void Clear() {
        _raw = 0;
        _filtered = NAN;
    }

private:
    float _raw;
    float _filtered;
    float _sensitivity;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE