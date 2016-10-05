#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class FFilename;
template <typename T, typename _Allocator>
class TRawStorage;
namespace Engine {
FWD_REFPTR(Model);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef TRawStorage<char, THREAD_LOCAL_ALLOCATOR(MeshGeneration, char)> ModelStream;
//----------------------------------------------------------------------------
bool LoadModel(PModel& pModel, const FFilename& filename);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
