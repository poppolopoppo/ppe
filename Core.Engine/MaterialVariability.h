#pragma once

#include "Engine.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class MaterialVariability {
    Always      = 0,
    Batch       = 1,
    Material    = 2,
    Scene       = 3,
    World       = 4,
    Frame       = 5,
    Once        = 6,
};
//----------------------------------------------------------------------------
inline bool SameOrMoreVariability(MaterialVariability lhs, MaterialVariability rhs) {
    return size_t(lhs) <= size_t(rhs);
}
//----------------------------------------------------------------------------
const char *MaterialVariabilityToCStr(MaterialVariability value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct VariabilitySeed {
    enum : size_t {
        Invalid = 0,
        Count   = 7
    };

    size_t Value = VariabilitySeed::Invalid;

    void Randomize(size_t salt);
    void Reset() { Value = size_t(Invalid) + 1; }

    void Next() { if (++Value == Invalid) ++Value; }
};
//----------------------------------------------------------------------------
inline bool operator ==(const VariabilitySeed& lhs, const VariabilitySeed& rhs) {
    return lhs.Value == rhs.Value;
}
//----------------------------------------------------------------------------
inline bool operator !=(const VariabilitySeed& lhs, const VariabilitySeed& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct MaterialVariabilitySeed {
    MaterialVariability Variability = MaterialVariability::Once;
    VariabilitySeed     Seed;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
