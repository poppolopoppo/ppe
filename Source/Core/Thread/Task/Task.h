#pragma once

#include "Core/Core.h"

#include "Core/Memory/RefPtr.h"

#include <functional>

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TaskContext;
//----------------------------------------------------------------------------
enum class TaskResult : bool {
    Succeed = true,
    Failed  = false
};
//----------------------------------------------------------------------------
class ITask: public RefCountable {
public:
    virtual ~ITask() {}

    virtual TaskResult Invoke(const TaskContext& ctx) = 0;
};
typedef RefPtr<ITask> PTask;
typedef RefPtr<const ITask> PCTask;
//----------------------------------------------------------------------------
class LambdaTask : public ITask {
public:
    typedef std::function<TaskResult (const TaskContext& )> function_type;

    explicit LambdaTask(function_type&& lambda);
    virtual ~LambdaTask();

    LambdaTask(const LambdaTask& ) = delete;
    LambdaTask& operator =(const LambdaTask& ) = delete;

    LambdaTask(LambdaTask&& rvalue);
    LambdaTask& operator =(LambdaTask&& rvalue);

    const function_type& Lambda() const { return _lambda; }

    virtual TaskResult Invoke(const TaskContext& ctx) override;

private:
    function_type _lambda;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
