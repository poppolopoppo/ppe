#include "stdafx.h"

#include "Container/RawStorage.h"
#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "Memory/MemoryStream.h"
#include "Misc/Process.h"

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Process)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if defined(PLATFORM_WINDOWS)
#   ifdef ARCH_X64
#       define PPE_TESTPROCESS_IMAGENAME_ARGS L"c:\\Windows\\syswow64\\getmac.exe", {}
#   else
#       define PPE_TESTPROCESS_IMAGENAME_ARGS L"c:\\Windows\\system32\\getmac.exe", {}
#   endif
#elif defined(PLATFORM_LINUX)
#   define PPE_TESTPROCESS_IMAGENAME_ARGS L"/bin/ls", {}
#else
#   error "unsupported OS"
#endif
//----------------------------------------------------------------------------
static void Test_ProcessCapture_(const FWString& executable, const FWString& params) {

    FProcess::FRawStorage outb, errb;
    const int exitCode = FProcess::CaptureOutput(
        &outb, &errb,
        executable, params, {},
        FProcess::NoWindow );

    const FStringView errA = errb.MakeConstView().Cast<const char>();
    const FStringView outA = outb.MakeConstView().Cast<const char>();

    UNUSED(errA);
    UNUSED(outA);

    LOG_DIRECT(Test_Process, Info, ToWString(outA));

    if (not errA.empty())
        LOG_DIRECT(Test_Process, Error, ToWString(errA));

    FLUSH_LOG();

    AssertRelease(0 == exitCode);
}
//----------------------------------------------------------------------------
static void Test_ProcessLog_(const FWString& executable, const FWString& params) {
    MEMORYSTREAM(Process) log;
    log.reserve(GBufferedStreamDefaultBufferSize);

    const int exitCode = FProcess::CaptureOutput(
        &log, &log,
        executable, params, {},
        FProcess::NoWindow );

    const FStringView logA = log.MakeView().Cast<const char>();

    UNUSED(logA);

    LOG_DIRECT(Test_Process, Info, ToWString(logA));
    FLUSH_LOG();

    AssertRelease(0 == exitCode);
}
//----------------------------------------------------------------------------
static void Test_ProcessCapture_() {
    Test_ProcessCapture_(PPE_TESTPROCESS_IMAGENAME_ARGS);
    Test_ProcessLog_(PPE_TESTPROCESS_IMAGENAME_ARGS);
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
