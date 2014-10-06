#pragma once

#include "Engine.h"

#include "Core/MemoryView.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MaterialEffect);
FWD_REFPTR(Scene);
struct VariabilitySeed;
//----------------------------------------------------------------------------
struct MaterialContext {
    const Engine::Scene *Scene;
    const Engine::MaterialEffect *MaterialEffect;
    const MemoryView<const Engine::VariabilitySeed> Seeds;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
