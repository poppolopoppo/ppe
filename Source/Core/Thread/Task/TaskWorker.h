#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class TaskEvaluator;
//----------------------------------------------------------------------------
class TaskWorker {
public:
    TaskWorker(const char *name, TaskEvaluator *evaluator, size_t threadIndex);
    ~TaskWorker();

    TaskWorker(const TaskWorker& ) = delete;
    TaskWorker& operator =(const TaskWorker& ) = delete;

    const char *Name() const { return _name; }
    TaskEvaluator *Evaluator() const { return _evaluator; }
    size_t ThreadIndex() const { return _threadIndex; }

    void Execute();

private:
    const char *_name;
    TaskEvaluator *_evaluator;
    size_t _threadIndex;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
