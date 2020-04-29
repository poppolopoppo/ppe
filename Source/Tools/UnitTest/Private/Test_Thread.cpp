#include "stdafx.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformAtomics.h"
#include "HAL/PlatformProcess.h"

#include "Container/Deque.h"
#include "Container/HashMap.h"
#include "Container/RawStorage.h"
#include "Container/SparseArray.h"
#include "Container/Vector.h"

#include "IO/BufferedStream.h"
#include "IO/FileStream.h"
#include "IO/FormatHelpers.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Maths/RandomGenerator.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/RefPtr.h"
#include "Memory/WeakPtr.h"
#include "Misc/Event.h"
#include "Misc/Function.h"
#include "Time/DateTime.h"
#include "Time/Timestamp.h"
#include "Time/TimedScope.h"

#include "Thread/ReadWriteLock.h"
#include "Thread/Task.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadPool.h"

namespace PPE {
namespace Test {
LOG_CATEGORY_VERBOSITY(, Test_Thread, NoDebug)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FTask_Member_ : Meta::FNonCopyableNorMovable {
    void Task(ITaskContext&) {}
    void Task_Extra(ITaskContext&, int, bool, float, void*) {}

    void TaskConst(ITaskContext&) const {}
    void TaskConst_Extra(ITaskContext&, int, bool, float, void*) const {}
};

FWD_REFPTR(RefCountableTask_);
class FRefCountableTask_ : public FRefCountable {
public:
    void Task(ITaskContext&) {
        NOOP();
    }
};

void TaskFunc_(ITaskContext&) {}
void TaskFunc_Extra_(ITaskContext&, int, bool, float, void*) {}
void TaskFunc_ExtraRef_(ITaskContext&, FTask_Member_&) {}
void TaskFunc_ExtraConstRef_(ITaskContext&, const FTask_Member_&) {}

NO_INLINE void Test_Function_() {
    FTask_Member_ m;
    auto lambda = [&m, p{ 42 }](ITaskContext&) {
        UNUSED(m);
        UNUSED(p);
    };

    FTaskFunc task;
    task = [](ITaskContext&) {};
    task = [&m](ITaskContext& ctx) { m.Task(ctx); };
    task = lambda;
    task = FTaskFunc::Bind<&TaskFunc_>();
    task = FTaskFunc::Bind<&TaskFunc_Extra_>(42, false, 3.1415f, &task);
    task = FTaskFunc::Bind<&TaskFunc_ExtraRef_>(&m);
    task = FTaskFunc::Bind<&TaskFunc_ExtraConstRef_>(&m);
    task = FTaskFunc::Bind<&FTask_Member_::Task>(&m);
    task = FTaskFunc::Bind<&FTask_Member_::Task_Extra>(&m, 42, false, 3.1415f, &task);
    task = FTaskFunc::Bind<&FTask_Member_::TaskConst>(&m);
    task = FTaskFunc::Bind<&FTask_Member_::TaskConst_Extra>(&m, 42, false, 3.1415f, &task);

    FRefCountableTask_ c;
    task = FTaskFunc::Bind<&FRefCountableTask_::Task>(&c);
    task = FTaskFunc::Bind<&FRefCountableTask_::Task>(MakeSafePtr(&c));

    PRefCountableTask_ a = NEW_REF(Task, FRefCountableTask_);
    task = FTaskFunc::Bind<&FRefCountableTask_::Task>(a);
    task = FTaskFunc::Bind<&FRefCountableTask_::Task>(a.get());
}
//--------------------1--------------------------------------------------------
NO_INLINE void Test_Aggregation_() {
    auto& pool = FGlobalThreadPool::Get();

    FAggregationPort ag;
    pool.Run(ag, TMemoryView<const FTaskFunc>{
        [](ITaskContext&) {},
        [](ITaskContext&) {},
        [](ITaskContext&) {},
        [](ITaskContext&) {}
    });

    pool.RunAndWaitFor([&ag](ITaskContext& ctx) {
        ag.Join(ctx);
    });
}
//--------------------1--------------------------------------------------------
NO_INLINE void Test_Event_() {
    TEvent< TFunction<void(int)> > evt;
    evt.Emplace([](int i) { UNUSED(i); LOG(Test_Thread, Info, L"A = {0}", i); });
    evt.Emplace([](int i) { UNUSED(i); LOG(Test_Thread, Info, L"B = {0}", i); });
    evt.Emplace([](int i) { UNUSED(i); LOG(Test_Thread, Info, L"C = {0}", i); });

    FEventHandle id = evt.Add([](int i) {
        UNUSED(i);
        LOG(Test_Thread, Info, L"DELETED = {0}", i);
    });

    evt(42);

    evt.Remove(id);

    evt(69);

    evt.Clear();

    evt(1337);
}
//----------------------------------------------------------------------------
NO_INLINE void Test_Task_() {
    class FTest : public FRefCountable {
    public:
        FTest() {}
        void Log(ITaskContext&) const {
            LOG(Test_Thread, Info, L"Test task !");
        }
    };

    FTest task;
    AddRef(&task);

    auto& pool = FGlobalThreadPool::Get();
    pool.RunAndWaitFor(MakeFunction<&FTest::Log>(&task)); // should use a TSafePtr<> inside TFunction<>

    RemoveRef_AssertReachZero_NoDelete(&task);
}
//----------------------------------------------------------------------------
NO_INLINE void Test_Async_() {
    // Can't overload with std::function<>: http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-defects.html#2132
    /*
    async([]() {
        STACKLOCAL_OCSTRSTREAM(oss, 512);
        oss << CurrentThreadContext().Name() << ": async fire and forget" << eol;
        std::cout << oss.c_str() << std::flush;
    });
    */

    Async([](ITaskContext&) {
        LOG(Test_Thread, Info, L"{0}: async fire and forget with context",
            MakeCStringView(CurrentThreadContext().Name()));
    });
}
//----------------------------------------------------------------------------
NO_INLINE void Test_Future_() {
    LOG(Test_Thread, Info, L"Start future");

    const PFuture<int> future = Future<int>([]() -> int {
        auto threadName = MakeCStringView(CurrentThreadContext().Name());
        LOG(Test_Thread, Info, L"{0}: future start", threadName);
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
        LOG(Test_Thread, Info, L"{0}: future stop", threadName);
        return 42;
    });

    while (not future->Available()) {
        LOG(Test_Thread, Debug, L"Waiting for future ...");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    LOG(Test_Thread, Info, L"Result from future = {0}", future->Result());

    Assert(42 == future->Result());
}
//----------------------------------------------------------------------------
NO_INLINE void Test_ParallelFor_() {
    const size_t values[] = {
        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
        21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37
    };

    LOG(Test_Thread, Info, L"ParallelFor start");

    ParallelForEach(std::begin(values), std::end(values), [](size_t v) {
        NOOP(v);
        LOG(Test_Thread, Info, L"ParallelFor: {0} -> {1}",
            MakeCStringView(CurrentThreadContext().Name()), v );
        FPlatformProcess::Sleep(0);
    });

    LOG(Test_Thread, Info, L"ParallelFor stop");
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FGraphNode {
    FCompletionPort Port;
    FCompletionPort Port2;

    int Depth{ 0 };

    std::atomic<int> Revision{ 0 };
    void* UserData{ 0 };


    FAtomicPhaseLock Phase;
    WWeakRefCountable Payload;

    FTimestamp Timestamp;
    VECTOR(Task, FGraphNode*) Dependencies;

    template <typename T>
    T* GetUserData() const { return static_cast<T*>(UserData); }

    FGraphNode() = default;

    FGraphNode(const FGraphNode&) = delete;
    FGraphNode& operator =(const FGraphNode&) = delete;

    using FQueue = SPARSEARRAY_INSITU(Task, FGraphNode*);

    void Build(ITaskContext&) {
        //FPlatformProcess::Sleep(0);
        Timestamp = FTimestamp::Now();

        LOG(Test_Thread, Debug, L"Build: {0} -> {1} (depth = {2:3})",
            Fmt::Pointer(this), Timestamp, Depth);
    }

    void Pass(FQueue* q, int revision, int depth = 0) {
        Depth = Max(depth, Depth);
        if (Revision != revision) {
            Revision = revision;

            for (FGraphNode* dep : Dependencies)
                dep->Pass(q, revision, Depth + 1);

            q->Emplace(this);
        }
    }
};
struct FGraph {
    int Revision{ 0 };

    FTimestamp Timestamp;
    FTimedScope Timer;
    std::atomic<size_t> NumBuilt;
    std::atomic<size_t> NumWorkers;
    std::atomic<size_t> MaxWorkers;

    TAllocaBlock<FGraphNode> Storage;
    VECTOR(Task, FGraphNode*) Nodes;

    ~FGraph() {
        Meta::Destroy(Storage.MakeView());
    }

    FGraphNode* Root() const {
        return Storage.data();
    }

    int Reset() {
        Timestamp = 0;
        NumBuilt = 0;
        NumWorkers = 0;
        MaxWorkers = 0;
        FPlatformAtomics::MemoryBarrier();
        Timer = FTimedScope();
        return (++Revision);
    }

    void WorkerBegin() {
        MaxWorkers = Max(MaxWorkers.load(), ++NumWorkers);
    }

    void WorkerEnd() {
        ++NumBuilt;
        --NumWorkers;
    }

    void OnBuildFinished(const FWStringView& executor) {
        UNUSED(executor);

        FPlatformAtomics::MemoryBarrier();

        LOG(Test_Thread, Info, L"executor<{0}>: built {1} nodes in {2} with {3} max workers",
            executor,
            Fmt::CountOfElements(NumBuilt.load()),
            Timer.Elapsed(),
            Fmt::CountOfElements(MaxWorkers.load()) );

        Verify(CheckBuild());
    }

    void Randomize(const size_t n, size_t depth) {
        Storage.RelocateIFP(n);

        FRandomGenerator rnd;

        size_t al = 1;

        DEQUE(Generation, void*) queue;
        queue.emplace_back(Storage.data());

        while (!queue.empty()) {
            FGraphNode* p = INPLACE_NEW(queue.front(), FGraphNode);
            p->Timestamp = Timestamp;

            queue.pop_front();

            const size_t m = 1+rnd.Next(depth + 1);
            forrange(j, 0, m) {
                if (al < n) {
                    FGraphNode* dep = &Storage.RawData[al++];
                    queue.emplace_back(dep);
                    p->Dependencies.emplace_back(dep);
                }
            }
        }

        Nodes.assign(Storage.MakeView().Map([](FGraphNode& n) { return &n; }));
        rnd.Shuffle(Nodes.MakeView());
    }

    bool CheckBuild() const {
        for (const FGraphNode* n : Nodes) {
            if (n->Timestamp.Value() < Timestamp.Value()) {
                return false;
            }
        }

        return true;
    }
};
//----------------------------------------------------------------------------
NO_INLINE void Test_Graph_Preprocess_(FGraph& g) {
    FHighPriorityThreadPool::Get().RunAndWaitFor([&g](ITaskContext& ctx) {
        g.Reset();

        FGraphNode::FQueue q;
        for (FGraphNode* node : g.Nodes)
            node->Pass(&q, g.Revision);

        LOG(Test_Thread, Info, L"Collected {0} tasks to build",
            Fmt::CountOfElements(q.size()));

        FTimedScope time;
        {
            FAggregationPort waitAll;

            for (FGraphNode* dep : q) {
                ctx.Run(&dep->Port, [&g, dep](ITaskContext& c) {
                    g.WorkerBegin();

                    for (FGraphNode* d : dep->Dependencies)
                        c.WaitFor(d->Port);

                    dep->Build(c);

                    g.WorkerEnd();
                });

                waitAll.Attach(&dep->Port);
            }

            waitAll.Join(ctx);
        }

        g.OnBuildFinished(L"Preprocess");
    });
}
//----------------------------------------------------------------------------
static void EachGraphNode_(FAggregationPort* port, ITaskContext& ctx, FGraph& g, FGraphNode* n) {
    Assert(port);

    int old = n->Revision;
    if (old != g.Revision && n->Revision.compare_exchange_strong(old, g.Revision)) {
        ctx.Run(&n->Port2, [&g, n](ITaskContext& subc) {
            g.WorkerBegin();
            {
                FAggregationPort waitDeps;
                for (FGraphNode* dep : n->Dependencies)
                    EachGraphNode_(&waitDeps, subc, g, dep);

                waitDeps.Join(subc);
            }
            {
                n->Build(subc);
            }
            g.WorkerEnd();
        });
    }

    port->Attach(&n->Port2);
}
NO_INLINE void Test_Graph_Incremental_(FGraph& g) {

    FHighPriorityThreadPool::Get().RunAndWaitFor([&g](ITaskContext& ctx) {
        g.Reset();

        FAggregationPort waitAll;
        for (FGraphNode* n : g.Nodes)
            EachGraphNode_(&waitAll, ctx, g, n);

        waitAll.Join(ctx);

        g.OnBuildFinished(L"Incremental");
    });
}
//----------------------------------------------------------------------------
struct FOutOfCoreBuilder_Incremental_ : Meta::FNonCopyableNorMovable {
    FGraph* Graph;

    FOutOfCoreBuilder_Incremental_()
    :   Graph(nullptr)
    {}

    void Build(ITaskContext& ctx, FGraph& g) {
        Graph = &g;
        Graph->Reset();

        BatchBuild(ctx, g.Nodes);

        g.OnBuildFinished(L"OutOfOrder-Incremental");
    }

private:
    static ETaskPriority PriorityFromArity_(const size_t n) NOEXCEPT {
        if (0 == n)
            return ETaskPriority::High;
        else if (4 > n)
            return ETaskPriority::Normal;
        else
            return ETaskPriority::Low;
    }

    static PCompletionPort MakeBuildPort() {
        return NEW_REF(Generation, FCompletionPort);
    }

    void LaunchBuild(ITaskContext& ctx, FAggregationPort* batch, FGraphNode* node) {
        Assert(node);

        PCompletionPort port{ MakeBuildPort() };
        node->Payload.reset(port);

        Verify(batch->Attach(port.get()));

        ctx.Run(port.get(),
            FTaskFunc::Bind<&FOutOfCoreBuilder_Incremental_::AsyncWork>(this, node),
            PriorityFromArity_(node->Dependencies.size()));
    }

    void BatchBuild(ITaskContext& ctx, const TMemoryView<FGraphNode* const>& nodes) {
        Assert(not nodes.empty());

        FAggregationPort batch;

        for (FGraphNode* const node : nodes) {
            auto onTransition = [&, node]() {
                LaunchBuild(ctx, &batch, node);
                ONLY_IF_ASSERT(node->Revision = Graph->Revision);
            };
            if (not node->Phase.Transition(Graph->Revision, std::move(onTransition))) {
                Assert_NoAssume(node->Revision == Graph->Revision);

                PWeakRefCountable payload;
                if (node->Payload.TryLock(&payload))
                    batch.Attach(static_cast<FCompletionPort*>(payload.get()));
            }
        }

        batch.Join(ctx);
    }

    void AsyncWork(ITaskContext& ctx, FGraphNode* node) {
        Assert(node);

        Graph->WorkerBegin();

        if (not node->Dependencies.empty())
            BatchBuild(ctx, node->Dependencies);

        node->Build(ctx);

        Graph->WorkerEnd();
    }

};
NO_INLINE void Test_Graph_OutOfCore_Incremental_(FGraph& g) {
    FHighPriorityThreadPool::Get().RunAndWaitFor([&g](ITaskContext& ctx) {
        FOutOfCoreBuilder_Incremental_ builder;
        builder.Build(ctx, g);
    });
}
//----------------------------------------------------------------------------
NO_INLINE void Test_Graph_ParallelExecution_() {
    {
        FGraph g;
        g.Randomize(2048, 10);

        Test_Graph_Preprocess_(g);
        Test_Graph_Incremental_(g);
        Test_Graph_OutOfCore_Incremental_(g);
    }
#if 0 //%_NOCOMMIT%
    {
        forrange(i, 0, 1000) {
            FGraph g;
            g.Randomize(10000, 20);

            forrange(j, 0, 30)
                Test_Graph_OutOfCore_Incremental_(g);
        }
    }
#endif
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Thread() {
    PPE_DEBUG_NAMEDSCOPE("Test_Thread");

    LOG(Test_Thread, Emphasis, L"starting thread tests ...");

    Test_Function_();
    Test_Aggregation_();
    Test_Event_();
    Test_Async_();
    Test_Future_();
    Test_ParallelFor_();
    Test_Task_();

    Test_Graph_ParallelExecution_();

    ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
