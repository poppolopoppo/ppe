#include "stdafx.h"

#include "Diagnostic/CurrentProcess.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"
#include "HAL/TargetPlatform.h"
#include "IO/FormatHelpers.h"

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, Process)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCurrentProcess::FCurrentProcess(void* appHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t **argv)
:   _fileName(MakeCStringView(filename))
,   _args(NewArray<FWString>(argc)), _exitCode(0), _appIcon(0)
,   _startedAt(FTimepoint::Now()) {

    for (size_t i = 0; i < argc; ++i) {
        Assert(argv[i]);
        _args[i] = MakeCStringView(argv[i]);
    }

    const size_t dirSep = _fileName.find_last_of(L"\\/");
    AssertRelease(INDEX_NONE != dirSep);
    _directory.assign(_fileName.begin(), _fileName.begin() + dirSep);
    _executableName.assign(_fileName.begin() + dirSep + 1, _fileName.end());

    Verify(FPlatformFile::NormalizePath(_fileName));
    Verify(FPlatformFile::NormalizePath(_directory));

    _dataPath = FPlatformFile::JoinPath({ _directory, L"..", L"..", L"Data" });
    _savedPath = FPlatformFile::JoinPath({ _directory, L"..", L"Saved" });

    Verify(FPlatformFile::NormalizePath(_dataPath));
    Verify(FPlatformFile::NormalizePath(_savedPath));

    FPlatformFile::CreateDirectory(*_savedPath, nullptr);

    _appHandle = appHandle;
    _nShowCmd = nShowCmd;
#if !USE_PPE_FINAL_RELEASE
    _startedWithDebugger = FPlatformDebug::IsDebuggerPresent();
    if (_args.end() != _args.FindIf([](const FWString& arg) { return EqualsI(arg, L"-IgnoreDebugger"); })) {
        _startedWithDebugger = false;
    }
#else
    _startedWithDebugger = false;
#endif

#if !USE_PPE_FINAL_RELEASE
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

    DumpProcessInfos();
}
//----------------------------------------------------------------------------
FCurrentProcess::~FCurrentProcess() {
    LOG(Process, Info, L"exit with code = {0}.", _exitCode.load());
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpMemoryStats() const {
#if USE_PPE_LOGGER
    auto mem = FPlatformMemory::Stats();
    LOG(Process, Info, L"Physical mem   : {0:10f3} / {1:10f3} / {2:10f3} : {3}",
        Fmt::SizeInBytes(mem.UsedPhysical),
        Fmt::SizeInBytes(mem.PeakUsedPhysical),
        Fmt::SizeInBytes(mem.AvailablePhysical),
        Fmt::Percentage(mem.UsedPhysical, mem.AvailablePhysical) );
    LOG(Process, Info, L"Virtual mem    : {0:10f3} / {1:10f3} / {2:10f3} : {3}",
        Fmt::SizeInBytes(mem.UsedVirtual),
        Fmt::SizeInBytes(mem.PeakUsedVirtual),
        Fmt::SizeInBytes(mem.AvailableVirtual),
        Fmt::Percentage(mem.UsedVirtual, mem.AvailableVirtual) );
    auto stk = FPlatformMemory::StackUsage();
    LOG(Process, Info, L"StackLocal mem : {0:10f3} / {1:10f3} / {2:10f3} : {3}",
        Fmt::SizeInBytes(stk.Committed),
        Fmt::SizeInBytes(stk.Guard),
        Fmt::SizeInBytes(stk.Reserved),
        Fmt::Percentage(stk.Committed + stk.Guard, stk.Reserved) );
#endif
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpProcessInfos() const {
#if USE_PPE_LOGGER
    auto mem = FPlatformMemory::Constants();
    auto& platform = CurrentPlatform();
    LOG(Process, Info, L"platform name = {0} ({1})", platform.DisplayName(), CurrentPlatform().FullName());
    LOG(Process, Info, L"build configuration = " WIDESTRING(STRINGIZE(BUILDCONFIG)));
    LOG(Process, Info, L"compiled at = " WIDESTRING(__DATE__) L"  " WIDESTRING(__TIME__));
    LOG(Process, Info, L"   Client supported = {0:A}", platform.SupportsFeature(EPlatformFeature::Client));
    LOG(Process, Info, L"   Server supported = {0:A}", platform.SupportsFeature(EPlatformFeature::Server));
    LOG(Process, Info, L"   Editor supported = {0:A}", platform.SupportsFeature(EPlatformFeature::Editor));
    LOG(Process, Info, L"   DataGeneration supported = {0:A}", platform.SupportsFeature(EPlatformFeature::DataGeneration));
    LOG(Process, Info, L"   HighQuality supported = {0:A}", platform.SupportsFeature(EPlatformFeature::HighQuality));
    LOG(Process, Info, L"   CookedData supported = {0:A}", platform.SupportsFeature(EPlatformFeature::CookedData));

    LOG(Process, Info, L"cpu = {0}, {1}", FPlatformMisc::CPUVendor(), FPlatformMisc::CPUBrand());
    LOG(Process, Info, L"   {0} physical cores ({1} logical)", FPlatformMisc::NumCores(), FPlatformMisc::NumCoresWithSMT());
    LOG(Process, Info, L"   allocation granularity = {0}", Fmt::SizeInBytes(mem.AllocationGranularity));
    LOG(Process, Info, L"   cache line size = {0}", Fmt::SizeInBytes(mem.CacheLineSize));
    LOG(Process, Info, L"   page size = {0}", Fmt::SizeInBytes(mem.PageSize));
    LOG(Process, Info, L"   total physical memory = {0:f2}", Fmt::SizeInBytes(mem.TotalPhysical));
    LOG(Process, Info, L"   total virtual memory = {0:f2}", Fmt::SizeInBytes(mem.TotalVirtual));

    FString userName = FPlatformMisc::UserName();
    FString machineName = FPlatformMisc::MachineName();
    FString osName = FPlatformMisc::OSName();
    LOG(Process, Info, L"localhost = {0} @ {1}", userName, machineName);
    LOG(Process, Info, L"os = {0}", osName);

    u64 totalSize, usedSize;
    Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, _directory.c_str()));
    LOG(Process, Info, L"   start directory = '{0}' : {1:f2}/{2:f2} ({3})", _directory, Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize));

    Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, _dataPath.c_str()));
    LOG(Process, Info, L"   data directory = '{0}' : {1:f2}/{2:f2} ({3})", _dataPath, Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize));

    Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, _savedPath.c_str()));
    LOG(Process, Info, L"   saved directory = '{0}' : {1:f2}/{2:f2} ({3})", _savedPath, Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize));

    FWString workingDir = FPlatformFile::WorkingDirectory();
    Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, workingDir.c_str()));
    LOG(Process, Info, L"   working directory = '{0}' : {1:f2}/{2:f2} ({3})", workingDir, Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize));

    FWString userDir = FPlatformFile::UserDirectory();
    Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, userDir.c_str()));
    LOG(Process, Info, L"   user directory = '{0}' : {1:f2}/{2:f2} ({3})", userDir, Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize));

    FWString tempDir = FPlatformFile::TemporaryDirectory();
    Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, tempDir.c_str()));
    LOG(Process, Info, L"   temporary directory = '{0}' : {1:f2}/{2:f2} ({3})", tempDir, Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize));

    LOG(Process, Info, L"started with debugger = '{0:A}'", _startedWithDebugger);
    LOG(Process, Info, L"application handle = '{0}', nShowCmd = '{1}'", _appHandle, _nShowCmd);
    LOG(Process, Info, L"started '{0}' with {1} parameters", _fileName, _args.size());
    forrange(i, 0, _args.size())
        LOG(Process, Info, L"   [{0:2}] '{1}'", i, _args[i]);

    FLUSH_LOG(); // be sure to get those infos in the log
#endif
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
