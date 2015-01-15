#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
class Filename;
template <typename T, typename _Allocator>
class RawStorage;
namespace Engine {
FWD_REFPTR(Model);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef RawStorage<char, THREAD_LOCAL_ALLOCATOR(MeshGeneration, char)> ModelStream;
//----------------------------------------------------------------------------
bool LoadModel(PModel& pModel, const Filename& filename);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
