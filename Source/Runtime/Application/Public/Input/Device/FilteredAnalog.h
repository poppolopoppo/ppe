#pragma once

#include "Application_fwd.h"

#include "Maths/MathHelpers.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Meta/Optional.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TBasicFilteredAnalog {
public:
    using value_type = T;

    TBasicFilteredAnalog() = default;

    explicit TBasicFilteredAnalog(const value_type& value, float sensitivity = 2.f)
    :   _raw(value)
    ,   _filtered(value)
    ,   _sensitivity(sensitivity) {
    }

    const value_type& Delta() const { return _delta; }
    const value_type& Raw() const { return _raw; }
    value_type Filtered() const { return _filtered.value_or(_raw); }
    float Sensitivity() const { return _sensitivity; }

    value_type operator *() const { return Filtered(); }

    void Add(const value_type& offset) {
        SetRaw(_raw + offset);
    }

    void AddClamp(const value_type& offset, const value_type& vmin, const value_type& vmax) {
        SetRaw(Clamp(_raw + offset, vmin, vmax));
    }

    void SetRaw(const value_type& raw) {
        if (Likely(_filtered.has_value()))
            _delta = (raw - _raw);
        else
            _delta = value_type{0.f};
        _raw = raw;
    }

    void SetSensitivity(float s) {
        _sensitivity = s;
    }

    void Update(FTimespan dt) {
        dt = Min(dt, FMilliseconds{ 150 }); // defensive time update: clamp DT to 150ms

        if (Likely(_filtered.has_value()))
            _filtered = Lerp(*_filtered, _raw, Saturate(static_cast<float>(
                Pow(FSeconds{dt}.Value(), 1.0 / Max(SmallEpsilon_v<float>, _sensitivity)))));
        else
            _filtered = _raw;
    }

    void Clear() {
        Reset(Meta::MakeForceInit<value_type>());
    }

    void Reset(value_type init) {
        _raw = init;
        _filtered.reset();
    }

private:
    value_type _delta{0.f};
    value_type _raw{Meta::MakeForceInit<value_type>()};
    Meta::TOptional<value_type> _filtered;

    float _sensitivity{2.0f};
};
//----------------------------------------------------------------------------
using FFilteredAnalog = TBasicFilteredAnalog<float>;
using FFilteredAnalog2 = TBasicFilteredAnalog<float2>;
using FFilteredAnalog3 = TBasicFilteredAnalog<float3>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
