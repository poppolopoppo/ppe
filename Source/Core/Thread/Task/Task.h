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
typedef void (*FTaskFunc)(ITaskContext& context);
typedef TDelegate<FTaskFunc> FTaskDelegate;
//----------------------------------------------------------------------------
enum class ETaskPriority : u32 {
    High = 0,
    Normal,
    Low,

    _Reserved, // used internally, do not use for userland tasks

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

    operator FTaskDelegate ();

    void RunAndSuicide(ITaskContext& ctx);

protected:
    virtual void Run(ITaskContext& ctx) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
