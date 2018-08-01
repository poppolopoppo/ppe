#include "stdafx.h"

#include "CurrentProcess.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"
#include "IO/FormatHelpers.h"

namespace PPE {
LOG_CATEGORY(PPE_API, Process)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCurrentProcess::FCurrentProcess(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t **argv)
    : _fileName(MakeCStringView(filename))
    , _args(NewArray<FWString>(argc)), _exitCode(0), _appIcon(0)
    , _startedAt(FTimepoint::Now()) {

    for (size_t i = 0; i < argc; ++i) {
        Assert(argv[i]);
        _args[i] = MakeCStringView(argv[i]);
    }

    const size_t dirSep = _fileName.find_last_of(L"\\/");
    AssertRelease(INDEX_NONE != dirSep);
    _directory.assign(_fileName.begin(), _fileName.begin() + dirSep);

    Verify(FPlatformFile::NormalizePath(_fileName));
    Verify(FPlatformFile::NormalizePath(_directory));

    _applicationHandle = applicationHandle;
    _nShowCmd = nShowCmd;
#ifndef FINAL_RELEASE
    _startedWithDebugger = FPlatformDebug::IsDebuggerPresent();
    if (_args.end() != _args.FindIf([](const FWString& arg) { return EqualsI(arg, L"-IgnoreDebugger"); })) {
        _startedWithDebugger = false;
    }
#else
    _startedWithDebugger = false;
#endif

#ifndef FINAL_RELEASE
    if (_args.end() != _args.FindIf([](const FWString& arg) { return EqualsI(arg, L"-WaitForDebugger"); })) {
        _startedWithDebugger = false; // some parts of the code won't detect that the debugger is attached
        volatile bool bTurnThisOffWhenDebuggerIsAttached = (!FPlatformDebug::IsDebuggerPresent());
        volatile size_t loopCount = 0;
        while (bTurnThisOffWhenDebuggerIsAttached) {
            LOG(Process, Warning, L"waiting for debugger to be attached");
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // wait for debugger to be attached
            loopCount++;
        }
    }
#endif

#ifdef USE_DEBUG_LOGGER
    auto mem = FPlatformMemory::Constants();
    LOG(Process, Info, L"platform name = " WIDESTRING(STRINGIZE(PLATFORM_NAME)));
    LOG(Process, Info, L"build configuration = " WIDESTRING(STRINGIZE(BUILDCONFIG)));
    LOG(Process, Info, L"compiled at = " WIDESTRING(__DATE__) L"  " WIDESTRING(__TIME__));
    LOG(Process, Info, L"cpu = {0}, {1}", FPlatformMisc::CPUVendor(), FPlatformMisc::CPUBrand());
    LOG(Process, Info, L"   {0} physical cores ({1} logical)", FPlatformMisc::NumCores(), FPlatformMisc::NumCoresWHyperThreading());
    LOG(Process, Info, L"   allocation granularity = {0}", Fmt::SizeInBytes(mem.AllocationGranularity));
    LOG(Process, Info, L"   cache line size = {0}", Fmt::SizeInBytes(mem.CacheLineSize));
    LOG(Process, Info, L"   page size = {0}", Fmt::SizeInBytes(mem.PageSize));
    LOG(Process, Info, L"   total physical memory = {0:f2}", Fmt::SizeInBytes(mem.TotalPhysical));
    LOG(Process, Info, L"   total virtual memory = {0:f2}", Fmt::SizeInBytes(mem.TotalVirtual));

    FString userName = FPlatformMisc::UserName();
    FString machineName = FPlatformMisc::MachineName();
    FString osName = FPlatformMisc::OSName();
    LOG(Process, Info, L"localhost = {0} @ {1}", userName, machineName, osName);
    LOG(Process, Info, L"os = {0}", osName);

    u64 totalSize, usedSize;
    Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, _directory.c_str()));
    LOG(Process, Info, L"   start directory = '{0}' : {1:f2}/{2:f2} ({3}).", _directory, Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize));
    FWString workingDir = FPlatformFile::WorkingDirectory();
    Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, workingDir.c_str()));
    LOG(Process, Info, L"   working directory = '{0}' : {1:f2}/{2:f2} ({3}).", workingDir, Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize));
    FWString userDir = FPlatformFile::UserDirectory();
    Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, userDir.c_str()));
    LOG(Process, Info, L"   user directory = '{0}' : {1:f2}/{2:f2} ({3}).", userDir, Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize));
    FWString tempDir = FPlatformFile::TemporaryDirectory();
    Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, tempDir.c_str()));
    LOG(Process, Info, L"   temporary directory = '{0}' : {1:f2}/{2:f2} ({3}).", tempDir, Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize));

    LOG(Process, Info, L"started with debugger = '{0:a}'.", _startedWithDebugger);
    LOG(Process, Info, L"application handle = '{0}', nShowCmd = '{1}'.", _applicationHandle, _nShowCmd);
    LOG(Process, Info, L"started '{0}' with {1} parameters.", _fileName, _args.size());
    forrange(i, 0, _args.size())
        LOG(Process, Info, L"   [{0:2}] '{1}'", i, _args[i]);

    FLUSH_LOG(); // be sure to get those infos in the log
#endif
}
//----------------------------------------------------------------------------
FCurrentProcess::~FCurrentProcess() {
    LOG(Process, Info, L"exit with code = {0}.", _exitCode.load());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
