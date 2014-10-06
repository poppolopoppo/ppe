#include "stdafx.h"

#include "MaterialVariability.h"

#include <chrono>

#include "Core/Hash.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void VariabilitySeed::Randomize(size_t salt) {
    const i64 t = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
#ifdef ARCH_X64
    Value = hash_value(t, salt ^ (salt << 32));
#else
    Value = hash_value(t, salt ^ (salt << 16));
#endif
}
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
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
