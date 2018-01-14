#include "stdafx.h"

#include "Core/IO/BufferedStream.h"
#include "Core/IO/FileStream.h"
#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter.h"
#include "Core/Thread/Task.h"
#include "Core/Thread/ThreadContext.h"

namespace Core {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
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

    Async([](ITaskContext& ctx) {
        GStdout
            << MakeStringView(CurrentThreadContext().Name(), Meta::FForceInit{})
            << ": async fire and forget with context" << Endl;
    });
}
//----------------------------------------------------------------------------
static void Test_Future_() {
    auto oss = GStdout;

    oss << "Start future" << Endl;

    const PFuture<int> future = Future<int>([]() -> int {
        auto oss = GStdout;
        auto threadName = MakeStringView(CurrentThreadContext().Name(), Meta::FForceInit{});
        oss << threadName << ": future start" << Endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
        oss << threadName << ": future stop" << Endl;
        return 42;
    });

    while (not future->Available())
        oss << "Waiting for future...\n";

    oss << "Result from future : " << future->Result() << Endl;

    Assert(42 == future->Result());
}
//----------------------------------------------------------------------------
static void Test_ParallelFor_() {
    const size_t values[] = {
        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,
        21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37
    };

    GStdout << "ParallelFor start" << Endl;

    ParallelFor(std::begin(values), std::end(values), [](size_t v) {
        GStdout << MakeStringView(CurrentThreadContext().Name(), Meta::FForceInit{}) << ": " << v << Endl;
    });

    GStdout << "ParallelFor Stop" << Endl;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Thread() {
    Test_Async_();
    Test_Future_();
    Test_ParallelFor_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace Core
