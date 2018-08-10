#include "stdafx.h"

#include "MaterialVariability.h"

#include <chrono>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView MaterialVariabilityToCStr(EMaterialVariability value) {
    switch (value)
    {
    case PPE::Engine::EMaterialVariability::Always:
        return MakeStringView("Always");
    case PPE::Engine::EMaterialVariability::Batch:
        return MakeStringView("Batch");
    case PPE::Engine::EMaterialVariability::FMaterial:
        return MakeStringView("FMaterial");
    case PPE::Engine::EMaterialVariability::FScene:
        return MakeStringView("FScene");
    case PPE::Engine::EMaterialVariability::FWorld:
        return MakeStringView("FWorld");
    case PPE::Engine::EMaterialVariability::FFrame:
        return MakeStringView("FFrame");
    case PPE::Engine::EMaterialVariability::Once:
        return MakeStringView("Once");
    default:
        AssertNotImplemented();
    }
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
