#pragma once

#include "Core/Core.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/Delegate.h"

#include <functional>

namespace Core {
class ITaskContext;
class FTaskManager;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef void (*TaskFunc)(ITaskContext& context);
typedef TDelegate<TaskFunc> TaskDelegate;
//----------------------------------------------------------------------------
enum class ETaskPriority : u32 {
    High = 0,
    Normal,
    Low,

    _Count
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Task);
class FTask : public FRefCountable {
public:
    // Won't be deleted if the task was never run !

    virtual ~FTask() {}

    operator TaskDelegate ();

    void RunAndSuicide(ITaskContext& ctx);

protected:
    virtual void Run(ITaskContext& ctx) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
