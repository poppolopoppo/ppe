#include "stdafx.h"

#include "Container/RawStorage.h"
#include "Diagnostic/Logger.h"
#include "IO/StringBuilder.h"
#include "Misc/Process.h"

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Process)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#ifdef ARCH_X64
#   define PPE_TESTPROCESS_IMAGENAME_ARGS L"c:\\Windows\\syswow64\\getmac.exe", {}
#else
#   define PPE_TESTPROCESS_IMAGENAME_ARGS L"c:\\Windows\\system32\\getmac.exe", {}
#endif
//----------------------------------------------------------------------------
static void Test_ProcessCapture_() {
    FProcess::FRawStorage outb, errb;
    const int exitCode = FProcess::CaptureOutput(
        &outb, &errb,
        PPE_TESTPROCESS_IMAGENAME_ARGS, {},
        FProcess::NoWindow );

    const FStringView errA = errb.MakeConstView().Cast<const char>();
    const FStringView outA = outb.MakeConstView().Cast<const char>();

    Assert(errA.size() == 2);

    LOG_DIRECT(Test_Process, Info, ToWString(outA));

    if (not errA.empty())
        LOG_DIRECT(Test_Process, Error, ToWString(errA));

    FLUSH_LOG();

    VerifyRelease(0 == exitCode);
}
//----------------------------------------------------------------------------
#undef PPE_TESTPROCESS_IMAGENAME_ARGS
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Process() {
    Test_ProcessCapture_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
