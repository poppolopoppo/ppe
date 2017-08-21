#include "stdafx.h"

#include "Core/Thread/Task.h"
#include "Core/Thread/ThreadContext.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void Test_Async_() {
    async([]() {
        STACKLOCAL_OCSTRSTREAM(oss, 512);
        oss << CurrentThreadContext().Name() << ": async fire and forget" << eol;
        std::cout << oss.c_str() << std::flush;
    });

    async([](ITaskContext& ctx) {
        STACKLOCAL_OCSTRSTREAM(oss, 512);
        oss << CurrentThreadContext().Name() << ": async fire and forget with context" << eol;
        std::cout << oss.c_str() << std::flush;
    });
}
//----------------------------------------------------------------------------
static void Test_Future_() {
    const PFuture<int> future = future([]() -> int {
        STACKLOCAL_OCSTRSTREAM(oss, 512);
        oss << CurrentThreadContext().Name() << ": future" << eol;
        std::cout << oss.c_str() << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
        return 42;
    });

    while (not future->Available())
        std::cout << "Waiting for future...\n";

    std::cout << "EResult from future : " << future->Result() << eol;

    Assert(42 == future->Result());
}
//----------------------------------------------------------------------------
static void Test_ParallelFor_() {
    const size_t values[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

    parallel_for(std::begin(values), std::end(values), [](size_t v) {
        STACKLOCAL_OCSTRSTREAM(oss, 512);
        oss << CurrentThreadContext().Name() << ": " << v << eol;
        std::cout << oss.c_str() << std::flush;
    });

    std::cout << "Hurrah!" << eol;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Thread() {
    std::cout << std::nounitbuf;

    Test_Async_();
    Test_Future_();
    Test_ParallelFor_();

    std::cout << std::unitbuf;
    std::cout << std::flush;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
