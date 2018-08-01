#pragma once

#include "Core/Core.h"

#include "Core/Container/Stack.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Misc/Function.h"

#include <functional>

namespace Core {
class ITaskContext;
class FTaskManager;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ETaskPriority : u32 {
    High = 0,
    Normal,
    Low,
    Internal, // Do not use for userland tasks ! (reserved for system)
};
//----------------------------------------------------------------------------
using FTaskFunc = TFunction<void(ITaskContext&)>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
