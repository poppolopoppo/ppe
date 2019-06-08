#include "stdafx.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformProcess.h"

#include "Container/RawStorage.h"
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
#include "Misc/Event.h"
#include "Misc/Function.h"
#include "Time/DateTime.h"
#include "Time/Timestamp.h"
#include "Time/TimedScope.h"
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
static void Test_Event() {
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
static void Test_Task_() {
    class FTest : public FRefCountable {
    public:
        FTest() {}
        void Log(ITaskContext&) const {
            LOG(Test_Thread, Info, L"Test task !");
        }
    };

    TRefPtr<FTest> Ptr(NEW_REF(Task, FTest));

    auto& pool = FGlobalThreadPool::Get();
    pool.RunAndWaitFor(MakeFunction<&FTest::Log>(Ptr.get())); // should use a TSafePtr<> inside TFunction<>
}
//----------------------------------------------------------------------------
static void Test_Async_() {
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
static void Test_Future_() {
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
static void Test_ParallelFor_() {
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
    int Depth{ 0 };
    std::atomic<int> Revision{ 0 };
    FTimestamp Timestamp;
    FTaskWaitHandle Status;
    VECTOR(Task, FGraphNode*) Dependencies;

    FGraphNode() = default;

    FGraphNode(const FGraphNode&) = delete;
    FGraphNode& operator =(const FGraphNode&) = delete;

    FGraphNode(FGraphNode&&) = default;
    FGraphNode& operator =(FGraphNode&&) = default;

    using FQueue = SPARSEARRAY_INSITU(Task, FGraphNode*);

    void Build(ITaskContext&) {
        FPlatformProcess::Sleep(0.01f);
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
    TAllocaBlock<FGraphNode> Storage;
    VECTOR(Task, FGraphNode*) Nodes;

    void Randomize(size_t n, size_t depth) {
        Storage.RelocateIFP(n);

        FRandomGenerator rnd;

        forrange(i, 0, n) {
            FGraphNode* const p = INPLACE_NEW(&Storage.RawData[i], FGraphNode);

            if (i) {
                const size_t m = rnd.Next(depth + 1);

                p->Dependencies.reserve_AssumeEmpty(m);
                forrange(j, 0, m)
                    Add_Unique(p->Dependencies, &Storage.RawData[rnd.Next(i)]);
            }
        }

        Nodes.assign(Storage.MakeView().Map([](FGraphNode& n) { return &n; }));
        rnd.Shuffle(Nodes.MakeView());
    }
};
//----------------------------------------------------------------------------
static void Test_Graph_Preprocess_() {
    FGraph g;
    g.Randomize(1024, 10);
    FHighPriorityThreadPool::Get().RunAndWaitFor([&g](ITaskContext& ctx) {
        g.Revision++;

        FGraphNode::FQueue q;
        for (FGraphNode* node : g.Nodes)
            node->Pass(&q, g.Revision);

        LOG(Test_Thread, Info, L"Collected {0} tasks to build",
            Fmt::CountOfElements(q.size()));

        FTimedScope time;

        FTaskWaitHandle waitAll;
        ctx.CreateWaitHandle(&waitAll);

        for (FGraphNode* dep : q) {
            ctx.Run(&dep->Status, [dep](ITaskContext& c) {
                for (FGraphNode* d : dep->Dependencies)
                    c.WaitFor(d->Status);

                dep->Build(c);
                });
            dep->Status.AttachTo(waitAll);
        }

        ctx.WaitFor(waitAll);

        LOG(Test_Thread, Info, L"Collected {0} tasks to build in {1}",
            Fmt::CountOfElements(q.size()), time.Elapsed());

        for (FGraphNode* dep : q)
            dep->Status.Reset();
    });
}
//----------------------------------------------------------------------------
static void EachGraphNode_(ITaskContext& ctx, int rev, FGraphNode* n) {
    int old = n->Revision;
    if (old == rev || not n->Revision.compare_exchange_strong(old, rev)) {
        ctx.WaitFor(n->Status);
    }
    else {
        for (FGraphNode* dep : n->Dependencies)
            ctx.Run(&n->Status, [rev, dep](ITaskContext& c) {
                EachGraphNode_(c, rev, dep);
                });

        if (n->Status.Valid()) {
            ctx.WaitFor(n->Status);
            //n->Status.Reset();
        }

        n->Build(ctx);
    }
}
static void Test_Graph_Incremental_() {
    FGraph g;
    g.Randomize(1024, 10);
    FHighPriorityThreadPool::Get().RunAndWaitFor([&g](ITaskContext& ctx) {
        g.Revision++;
        for (FGraphNode* n : g.Nodes)
            EachGraphNode_(ctx, g.Revision, n);
        });
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Thread() {
    LOG(Test_Thread, Emphasis, L"starting thread tests ...");

    Test_Event();
    Test_Async_();
    Test_Future_();
    Test_ParallelFor_();
    Test_Task_();
    Test_Graph_Preprocess_();
    Test_Graph_Incremental_();

    ReleaseMemoryInModules();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
