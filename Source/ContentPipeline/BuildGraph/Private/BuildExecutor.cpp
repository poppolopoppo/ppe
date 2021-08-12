#include "stdafx.h"

#include "BuildExecutor.h"

#include "BuildContext.h"
#include "BuildNode.h"

#include "Container/SparseArray.h"

#include "Thread/Task/CompletionPort.h"
#include "Thread/ThreadPool.h"

#include <type_traits>

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParallelExecutor : public IBuildExecutor {
public:
    FParallelExecutor(FTaskManager& pool) NOEXCEPT
    :   _pool(pool)
    {}

    virtual void Queue(const FScanContext& scan, const TMemoryView<const PBuildNode>& deps) override {
        Queue_Dispatch_(scan, deps);
    }
    virtual void Queue(const FBuildContext& build, const TMemoryView<const PBuildNode>& deps) override {
        Queue_Dispatch_(build, deps);
    }
    virtual void Queue(const FCleanContext& clean, const TMemoryView<const PBuildNode>& deps) override {
        Queue_Dispatch_(clean, deps);
    }

    virtual EBuildResult QueueAndWaitFor(FScanContext& scan, const TMemoryView<const PBuildNode>& deps) override {
        return QueueAndWaitFor_Dispatch_(scan, deps);
    }
    virtual EBuildResult QueueAndWaitFor(FBuildContext& build, const TMemoryView<const PBuildNode>& deps) override {
        return QueueAndWaitFor_Dispatch_(build, deps);
    }
    virtual EBuildResult QueueAndWaitFor(FCleanContext& clean, const TMemoryView<const PBuildNode>& deps) override {
        return QueueAndWaitFor_Dispatch_(clean, deps);
    }

    virtual void WaitForAll() override {
        _pool.RunInWorker([this](ITaskContext& task) {
            _global.JoinAndReset(task);
        });
    }

private:
    template <typename _Context>
    void Queue_Dispatch_(const _Context& action, const TMemoryView<const PBuildNode>& deps) {
        if (not deps.empty()) {
            _Context& root = const_cast<_Context&>(action).Root();
            _pool.Run(_global, [&root, deps](ITaskContext& task) {
                _Context local(root);
                Parallelize_<_Context>(task, local, deps, nullptr);
            },  PriorityFromArity_(deps.size()) );
        }
    }

    template <typename _Context>
    EBuildResult QueueAndWaitFor_Dispatch_(_Context& action, const TMemoryView<const PBuildNode>& deps) {
        if (not deps.empty()) {
            _Context local(action);
            volatile EBuildResult result = EBuildResult::Unbuilt;
            _pool.RunInWorker(
                FTaskFunc::Bind< Parallelize_<_Context> >(&local, deps, &result),
                PriorityFromArity_(deps.size()) );
            return result;
        }
        else {
            return EBuildResult::Unbuilt;
        }
    }

private:
    static EBuildResult ExecuteNode_(FScanContext& scan, FBuildNode& node) {
        return scan.Scan(node);
    }
    static EBuildResult ExecuteNode_(FBuildContext& build, FBuildNode& node) {
        return build.Build(node);
    }
    static EBuildResult ExecuteNode_(FCleanContext& clean, FBuildNode& node) {
        return clean.Clean(node);
    }

    template <typename _Context>
    static void DispatchNode_(ITaskContext&, _Context& action, const SBuildNode& node) {
        Assert(node);

        FBuildState& st = node->State();
        st.Result = ExecuteNode_(action, *node);
    }

private:
    // Give a high priority to leaf nodes to minimize fiber usage
    static CONSTEXPR ETaskPriority PriorityFromArity_(size_t n) {
        if (0 == n)
            return ETaskPriority::High;
        else if (4 > n)
            return ETaskPriority::Normal;
        else
            return ETaskPriority::Low;
    }

    static ETaskPriority PriorityFromNode_(const FBuildNode& node) {
        return PriorityFromArity_(
            node.StaticDeps().size() +
            node.DynamicDeps().size() +
            node.RuntimeDeps().size() );
    }

private:
    FTaskManager& _pool;
    FAggregationPort _global;

    template <typename _Context>
    static void Parallelize_(ITaskContext& task, _Context& action, const TMemoryView<const PBuildNode>& deps, volatile EBuildResult* pResult) {
        STATIC_ASSERT(std::is_base_of_v<FPipelineContext, _Context>);
        Assert(deps.data());
        Assert_NoAssume(not deps.empty());

        const FBuildRevision globalRev = action.Revision();

        FAggregationPort batch;
        for (const PBuildNode& node : deps) {
            FBuildState& st = node->State();

            auto launchBuild = [&]() {
                const PCompletionPort port = NEW_REF(Task, FCompletionPort);

                st.Result = EBuildResult::Unbuilt;
                st.Payload.reset(port);

                task.Run(port.get(),
                    FTaskFunc::Bind< &DispatchNode_<_Context> >(&action, MakeSafePtr(node.get())),
                    PriorityFromNode_(*node) );
            };

            if (not st.Phase.Transition(globalRev, std::move(launchBuild))) {
                PWeakRefCountable payload;
                if (st.Payload.TryLock(&payload))
                    batch.Attach(static_cast<FCompletionPort*>(payload.get()));
            }
        }

        batch.Join(task);

        if (pResult) {
            EBuildResult result = EBuildResult::Unbuilt;

            for (const PBuildNode& node : deps) {
                FBuildState& st = node->State();
                result = Combine(result, st.Result);
            }

            *pResult = result;
        }
    }
};
//----------------------------------------------------------------------------
UBuildExecutor MakeParallelExecutor(FTaskManager& pool) {
    return MakeUnique<FParallelExecutor>(pool);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(SBuildNode) == sizeof(PBuildNode));
//----------------------------------------------------------------------------
void IBuildExecutor::Queue(const FScanContext& scan, const TMemoryView<const SBuildNode>& deps) {
    Queue(scan, TMemoryView<const PBuildNode>{ reinterpret_cast<const PBuildNode*>(deps.data()), deps.size() });
}
void IBuildExecutor::Queue(const FBuildContext& build, const TMemoryView<const SBuildNode>& deps) {
    Queue(build, TMemoryView<const PBuildNode>{ reinterpret_cast<const PBuildNode*>(deps.data()), deps.size() });
}
void IBuildExecutor::Queue(const FCleanContext& clean, const TMemoryView<const SBuildNode>& deps) {
    Queue(clean, TMemoryView<const PBuildNode>{ reinterpret_cast<const PBuildNode*>(deps.data()), deps.size() });
}
//----------------------------------------------------------------------------
EBuildResult IBuildExecutor::QueueAndWaitFor(FScanContext& scan, const TMemoryView<const SBuildNode>& deps) {
    return QueueAndWaitFor(scan, TMemoryView<const PBuildNode>{ reinterpret_cast<const PBuildNode*>(deps.data()), deps.size() });
}
EBuildResult IBuildExecutor::QueueAndWaitFor(FBuildContext& build, const TMemoryView<const SBuildNode>& deps) {
    return QueueAndWaitFor(build, TMemoryView<const PBuildNode>{ reinterpret_cast<const PBuildNode*>(deps.data()), deps.size() });
}
EBuildResult IBuildExecutor::QueueAndWaitFor(FCleanContext& clean, const TMemoryView<const SBuildNode>& deps) {
    return QueueAndWaitFor(clean, TMemoryView<const PBuildNode>{ reinterpret_cast<const PBuildNode*>(deps.data()), deps.size() });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
