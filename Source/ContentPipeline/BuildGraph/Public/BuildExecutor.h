#pragma once

#include "BuildGraph_fwd.h"

#include "Memory/UniquePtr.h"
#include "Thread/Task_fwd.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(BuildExecutor);
//----------------------------------------------------------------------------
class IBuildExecutor {
public:
    virtual ~IBuildExecutor() = default;

    virtual void Queue(const FScanContext& scan, const TMemoryView<const PBuildNode>& deps) = 0;
    virtual void Queue(const FBuildContext& build, const TMemoryView<const PBuildNode>& deps) = 0;
    virtual void Queue(const FCleanContext& clean, const TMemoryView<const PBuildNode>& deps) = 0;

    virtual EBuildResult QueueAndWaitFor(FScanContext& scan, const TMemoryView<const PBuildNode>& deps) = 0;
    virtual EBuildResult QueueAndWaitFor(FBuildContext& build, const TMemoryView<const PBuildNode>& deps) = 0;
    virtual EBuildResult QueueAndWaitFor(FCleanContext& clean, const TMemoryView<const PBuildNode>& deps) = 0;

    virtual void WaitForAll() = 0;

public: // helpers
    void Queue(const FScanContext& scan, const TMemoryView<const SBuildNode>& deps);
    void Queue(const FBuildContext& build, const TMemoryView<const SBuildNode>& deps);
    void Queue(const FCleanContext& clean, const TMemoryView<const SBuildNode>& deps);

    EBuildResult QueueAndWaitFor(FScanContext& scan, const TMemoryView<const SBuildNode>& deps);
    EBuildResult QueueAndWaitFor(FBuildContext& build, const TMemoryView<const SBuildNode>& deps);
    EBuildResult QueueAndWaitFor(FCleanContext& clean, const TMemoryView<const SBuildNode>& deps);
};
//----------------------------------------------------------------------------
PPE_BUILDGRAPH_API UBuildExecutor MakeParallelExecutor(FTaskManager& pool);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
