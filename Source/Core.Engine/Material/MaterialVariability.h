#pragma once

#include "Core.Engine/Engine.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EMaterialVariability {
    Always      = 0,
    Batch       ,
    FMaterial    ,
    FScene       ,
    FWorld       ,
    FFrame       ,
    Once        ,

    _Count,
};
//----------------------------------------------------------------------------
inline bool SameOrMoreVariability(EMaterialVariability lhs, EMaterialVariability rhs) {
    return size_t(lhs) <= size_t(rhs);
}
//----------------------------------------------------------------------------
const char *MaterialVariabilityToCStr(EMaterialVariability value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVariabilitySeed {
    enum : size_t { Invalid = 0, Count = size_t(EMaterialVariability::_Count) };

    size_t FValue = FVariabilitySeed::Invalid;

    void Reset() { FValue = size_t(Invalid) + 1; }

    void Next() { if (++FValue == Invalid) ++FValue; }
};
//----------------------------------------------------------------------------
typedef FVariabilitySeed VariabilitySeeds[FVariabilitySeed::Count]; 
//----------------------------------------------------------------------------
inline bool operator ==(const FVariabilitySeed& lhs, const FVariabilitySeed& rhs) {
    return lhs.Value == rhs.Value;
}
//----------------------------------------------------------------------------
inline bool operator !=(const FVariabilitySeed& lhs, const FVariabilitySeed& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FMaterialVariabilitySeed {
    EMaterialVariability Variability = EMaterialVariability::Once;
    FVariabilitySeed     Seed;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
