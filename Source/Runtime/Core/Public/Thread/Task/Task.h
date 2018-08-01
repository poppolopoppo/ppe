#pragma once

#include "Core.h"

#include "Container/Stack.h"
#include "Memory/RefPtr.h"
#include "Misc/Function.h"

#include <functional>

namespace PPE {
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
} //!namespace PPE
