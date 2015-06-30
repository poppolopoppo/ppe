#pragma once

#include "Core.Engine/Engine.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class MaterialVariability {
    Always      = 0,
    Batch       ,
    Material    ,
    Scene       ,
    World       ,
    Frame       ,
    Once        ,

    _Count,
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
    enum : size_t { Invalid = 0, Count = size_t(MaterialVariability::_Count) };

    size_t Value = VariabilitySeed::Invalid;

    void Reset() { Value = size_t(Invalid) + 1; }

    void Next() { if (++Value == Invalid) ++Value; }
};
//----------------------------------------------------------------------------
typedef VariabilitySeed VariabilitySeeds[VariabilitySeed::Count]; 
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
