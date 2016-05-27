#pragma once

#include "Core/Core.h"

#include "Core/Memory/RefPtr.h"
#include "Core/Meta/Delegate.h"

#include <functional>

namespace Core {
class ITaskContext;
class TaskManager;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef void (*TaskFunc)(ITaskContext& context);
typedef Delegate<TaskFunc> TaskDelegate;
//----------------------------------------------------------------------------
enum class TaskPriority : u32 {
    High = 0,
    Normal,
    Low,

    _Count
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Task);
class Task : public RefCountable {
public:
    // Won't be deleted if the task was never run !

    virtual ~Task() {}

    operator TaskDelegate ();

    void RunAndSuicide(ITaskContext& ctx);

protected:
    virtual void Run(ITaskContext& ctx) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
