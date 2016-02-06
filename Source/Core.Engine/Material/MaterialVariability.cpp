#include "stdafx.h"

#include "MaterialVariability.h"

#include <chrono>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice MaterialVariabilityToCStr(MaterialVariability value) {
    switch (value)
    {
    case Core::Engine::MaterialVariability::Always:
        return MakeStringSlice("Always");
    case Core::Engine::MaterialVariability::Batch:
        return MakeStringSlice("Batch");
    case Core::Engine::MaterialVariability::Material:
        return MakeStringSlice("Material");
    case Core::Engine::MaterialVariability::Scene:
        return MakeStringSlice("Scene");
    case Core::Engine::MaterialVariability::World:
        return MakeStringSlice("World");
    case Core::Engine::MaterialVariability::Frame:
        return MakeStringSlice("Frame");
    case Core::Engine::MaterialVariability::Once:
        return MakeStringSlice("Once");
    default:
        AssertNotImplemented();
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
