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
    case Core::Engine::EMaterialVariability::Always:
        return MakeStringView("Always");
    case Core::Engine::EMaterialVariability::Batch:
        return MakeStringView("Batch");
    case Core::Engine::EMaterialVariability::FMaterial:
        return MakeStringView("FMaterial");
    case Core::Engine::EMaterialVariability::FScene:
        return MakeStringView("FScene");
    case Core::Engine::EMaterialVariability::FWorld:
        return MakeStringView("FWorld");
    case Core::Engine::EMaterialVariability::FFrame:
        return MakeStringView("FFrame");
    case Core::Engine::EMaterialVariability::Once:
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
