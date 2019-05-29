#include "stdafx.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformProcess.h"
#include "IO/BufferedStream.h"
#include "IO/FileStream.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/RefPtr.h"
#include "Misc/Event.h"
#include "Misc/Function.h"
#include "Thread/Task.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadPool.h"

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Thread)
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
void Test_Thread() {
    LOG(Test_Thread, Emphasis, L"starting thread tests ...");

    Test_Event();
    Test_Async_();
    Test_Future_();
    Test_ParallelFor_();
    Test_Task_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
