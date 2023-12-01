// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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

    Unused(errA);
    Unused(outA);

    PPE_LOG_DIRECT(Test_Process, Info, [&](FTextWriter& oss) { oss << outA; });

    if (not errA.empty())
        PPE_LOG_DIRECT(Test_Process, Info, [&](FTextWriter& oss) { oss << errA; });

    PPE_LOG_FLUSH();

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

    Unused(logA);

    PPE_LOG_DIRECT(Test_Process, Info, [&](FTextWriter& oss) { oss << logA; });
    PPE_LOG_FLUSH();

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
