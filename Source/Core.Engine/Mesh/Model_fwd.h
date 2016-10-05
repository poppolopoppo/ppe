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
struct FRenderCommandRegistration;
typedef TUniquePtr<const FRenderCommandRegistration> URenderCommand;
//----------------------------------------------------------------------------
struct FModelRenderCommand;
typedef TUniquePtr<const FModelRenderCommand> UModelRenderCommand;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
