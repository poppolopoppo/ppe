#include "stdafx.h"

#include "MaterialVariability.h"

#include <chrono>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView MaterialVariabilityToCStr(MaterialVariability value) {
    switch (value)
    {
    case Core::Engine::MaterialVariability::Always:
        return MakeStringView("Always");
    case Core::Engine::MaterialVariability::Batch:
        return MakeStringView("Batch");
    case Core::Engine::MaterialVariability::Material:
        return MakeStringView("Material");
    case Core::Engine::MaterialVariability::Scene:
        return MakeStringView("Scene");
    case Core::Engine::MaterialVariability::World:
        return MakeStringView("World");
    case Core::Engine::MaterialVariability::Frame:
        return MakeStringView("Frame");
    case Core::Engine::MaterialVariability::Once:
        return MakeStringView("Once");
    default:
        AssertNotImplemented();
    }
    return StringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
