#include "stdafx.h"

#include "MaterialVariability.h"

#include <chrono>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *MaterialVariabilityToCStr(MaterialVariability value) {
    switch (value)
    {
    case Core::Engine::MaterialVariability::Always:
        return "Always";
    case Core::Engine::MaterialVariability::Batch:
        return "Batch";
    case Core::Engine::MaterialVariability::Material:
        return "Material";
    case Core::Engine::MaterialVariability::Scene:
        return "Scene";
    case Core::Engine::MaterialVariability::World:
        return "World";
    case Core::Engine::MaterialVariability::Frame:
        return "Frame";
    case Core::Engine::MaterialVariability::Once:
        return "Once";
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
