#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Model);
FWD_REFPTR(ModelMesh);
FWD_REFPTR(ModelMeshSubPart);
//----------------------------------------------------------------------------
struct RenderCommandRegistration;
typedef UniquePtr<const RenderCommandRegistration> URenderCommand;
//----------------------------------------------------------------------------
struct ModelRenderCommand;
typedef UniquePtr<const ModelRenderCommand> UModelRenderCommand;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
