#include "stdafx.h"

#include "Diagnostic/CurrentProcess.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformDebug.h"
#include "HAL/PlatformFile.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"
#include "HAL/TargetPlatform.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, Process)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void* FCurrentProcess::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared lib
}
//----------------------------------------------------------------------------
FCurrentProcess::FCurrentProcess(
    void* appHandle, int nShowCmd,
    const wchar_t* filename, size_t argc, const wchar_t * const* argv )
:   _fileName(MakeCStringView(filename))
,   _args(NewArray<FWString>(argc))
,   _exitCode(0)
,   _appIcon(0)
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
            LOG_DIRECT(Process, Warning, L"waiting for debugger to be attached");
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // wait for debugger to be attached
            loopCount++;
        }
    }
#endif

#if !USE_PPE_FINAL_RELEASE
    if (_args.end() != _args.FindIf([](const FWString& arg) { return EqualsI(arg, L"-AbortOnAssert"); })) {
        const FAssertHandler abortHandler = [](const wchar_t*, const wchar_t*, unsigned) -> bool { abort(); };
        SetAssertionHandler(abortHandler);
        SetAssertionReleaseHandler(abortHandler);
    }
#endif

    LogProcessInfos();
    LogMemoryStats();
    LogStorageInfos();
}
//----------------------------------------------------------------------------
FCurrentProcess::~FCurrentProcess() {
    LOG(Process, Info, L"exit with code = {0}.", _exitCode.load());
}
//----------------------------------------------------------------------------
void FCurrentProcess::LogMemoryStats() const {
#if USE_PPE_LOGGER
    FStringBuilder sb;
    DumpMemoryStats(sb);
    LOG_DIRECT(Process, Info, ToWString(sb.Written()).MakeView());
#endif
}
//----------------------------------------------------------------------------
void FCurrentProcess::LogProcessInfos() const {
#if USE_PPE_LOGGER
    FStringBuilder sb;
    DumpProcessInfos(sb);
    LOG_DIRECT(Process, Info, ToWString(sb.Written()).MakeView());
#endif
}
//----------------------------------------------------------------------------
void FCurrentProcess::LogStorageInfos() const {
#if USE_PPE_LOGGER
    FStringBuilder sb;
    DumpStorageInfos(sb);
    LOG_DIRECT(Process, Info, ToWString(sb.Written()).MakeView());
#endif
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpPhysicalMemory(FTextWriter& oss) const {
    {
        auto mem = FPlatformMemory::Stats();
        Format(oss, "process memory =") << Eol;
        Format(oss, "   physical mem   : {0:10f3} / {1:10f3} / {2:10f3} : {3}",
            Fmt::SizeInBytes(mem.UsedPhysical),
            Fmt::SizeInBytes(mem.PeakUsedPhysical),
            Fmt::SizeInBytes(mem.AvailablePhysical),
            Fmt::Percentage(mem.UsedPhysical, mem.AvailablePhysical)) << Eol;
        Format(oss, "   virtual mem    : {0:10f3} / {1:10f3} / {2:10f3} : {3}",
            Fmt::SizeInBytes(mem.UsedVirtual),
            Fmt::SizeInBytes(mem.PeakUsedVirtual),
            Fmt::SizeInBytes(mem.AvailableVirtual),
            Fmt::Percentage(mem.UsedVirtual, mem.AvailableVirtual)) << Eol;
        auto stk = FPlatformMemory::StackUsage();
        Format(oss, "   stackLocal mem : {0:10f3} / {1:10f3} / {2:10f3} : {3}",
            Fmt::SizeInBytes(stk.Committed),
            Fmt::SizeInBytes(stk.Guard),
            Fmt::SizeInBytes(stk.Reserved),
            Fmt::Percentage(stk.Committed + stk.Guard, stk.Reserved)) << Eol;
    }
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpMemoryStats(FTextWriter& oss) const {
#if USE_PPE_MEMORYDOMAINS
    {
        Format(oss, "memory domains =") << Eol;

        const FMemoryTracking* domains[] = {
            &MemoryDomain::FReservedMemory::TrackingData(),
            &MemoryDomain::FUsedMemory::TrackingData(),
            &MemoryDomain::FPooledMemory::TrackingData()
        };

        for (const FMemoryTracking* domain : domains) {
            const auto usr = domain->User();
            const auto sys = domain->System();

            Format(oss,
                "   [{0}]", domain->Name()) << Eol;
            Format(oss,
                "       $usr\n"
                "           num allocs          = {0}\n"
                "           min / max size      = {1:8f3} / {2:8f3}\n"
                "           total size          = {3:8f3}\n"
                "           peak allocs / size  = {4:8f3} / {5:8f3}\n"
                "           accum allocs / size = {6:8f3} / {7:8f3}",
                Fmt::CountOfElements(usr.NumAllocs),
                Fmt::SizeInBytes(usr.MinSize),
                Fmt::SizeInBytes(usr.MaxSize),
                Fmt::SizeInBytes(usr.TotalSize),
                Fmt::CountOfElements(usr.PeakAllocs),
                Fmt::SizeInBytes(usr.PeakSize),
                Fmt::CountOfElements(usr.AccumulatedAllocs),
                Fmt::SizeInBytes(usr.AccumulatedSize)) << Eol;
            Format(oss,
                "       $sys\n"
                "           num allocs          = {0}\n"
                "           min / max size      = {1:8f3} / {2:8f3}\n"
                "           total size          = {3:8f3}\n"
                "           peak allocs / size  = {4:8f3} / {5:8f3}\n"
                "           accum allocs / size = {6:8f3} / {7:8f3}",
                Fmt::CountOfElements(sys.NumAllocs),
                Fmt::SizeInBytes(sys.MinSize),
                Fmt::SizeInBytes(sys.MaxSize),
                Fmt::SizeInBytes(sys.TotalSize),
                Fmt::CountOfElements(sys.PeakAllocs),
                Fmt::SizeInBytes(sys.PeakSize),
                Fmt::CountOfElements(sys.AccumulatedAllocs),
                Fmt::SizeInBytes(sys.AccumulatedSize)) << Eol;
        }

    }
#else
    UNUSED(oss);
#endif //!USE_PPE_MEMORYDOMAINS
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpProcessInfos(FTextWriter& oss) const {
    {
        Format(oss, "host = {0} @ {1}", FPlatformMisc::UserName(), FPlatformMisc::MachineName()) << Eol;
        Format(oss, "os = {0}", FPlatformMisc::OSName()) << Eol;
        Format(oss, "started with debugger = '{0:A}'", _startedWithDebugger) << Eol;
        Format(oss, "application handle = '{0}', nShowCmd = '{1}'", _appHandle, _nShowCmd) << Eol;
        Format(oss, "started '{0}' with {1} parameters", _fileName, _args.size()) << Eol;
        forrange(i, 0, _args.size())
            Format(oss, "   [{0:2}] '{1}'", i, _args[i]) << Eol;
    }
    {
        auto& platform = CurrentPlatform();
        Format(oss, "platform = {0} ({1})", platform.DisplayName(), CurrentPlatform().FullName()) << Eol;
        Format(oss, "build configuration = " STRINGIZE(BUILD_FAMILY)) << Eol;
        Format(oss, "compiled at = " __DATE__ "  " __TIME__) << Eol;
        Format(oss, "   client supported = {0:A}", platform.SupportsFeature(EPlatformFeature::Client)) << Eol;
        Format(oss, "   server supported = {0:A}", platform.SupportsFeature(EPlatformFeature::Server)) << Eol;
        Format(oss, "   editor supported = {0:A}", platform.SupportsFeature(EPlatformFeature::Editor)) << Eol;
        Format(oss, "   data generation supported = {0:A}", platform.SupportsFeature(EPlatformFeature::DataGeneration)) << Eol;
        Format(oss, "   high quality supported = {0:A}", platform.SupportsFeature(EPlatformFeature::HighQuality)) << Eol;
        Format(oss, "   cooked data supported = {0:A}", platform.SupportsFeature(EPlatformFeature::CookedData)) << Eol;
    }
    {
        auto mem = FPlatformMemory::Constants();
        Format(oss, "cpu = {0}, {1}", FPlatformMisc::CPUVendor(), FPlatformMisc::CPUBrand()) << Eol;
        Format(oss, "   {0} physical cores ({1} logical)", FPlatformMisc::NumCores(), FPlatformMisc::NumCoresWithSMT()) << Eol;
        Format(oss, "   allocation granularity = {0}", Fmt::SizeInBytes(mem.AllocationGranularity)) << Eol;
        Format(oss, "   cache line size = {0}", Fmt::SizeInBytes(mem.CacheLineSize)) << Eol;
        Format(oss, "   page size = {0}", Fmt::SizeInBytes(mem.PageSize)) << Eol;
        Format(oss, "   total physical memory = {0:f2}", Fmt::SizeInBytes(mem.TotalPhysical)) << Eol;
        Format(oss, "   total virtual memory = {0:f2}", Fmt::SizeInBytes(mem.TotalVirtual)) << Eol;
    }
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpStorageInfos(FTextWriter& oss) const {
    {
        Format(oss, "storage =") << Eol;

        const FWString workingDir = FPlatformFile::WorkingDirectory();
        const FWString userDir = FPlatformFile::UserDirectory();
        const FWString tempDir = FPlatformFile::TemporaryDirectory();

        struct FDirectory {
            FStringView Name;
            FWStringView Path;
        };

        const FDirectory directories[] = {
            { "start",      _directory  },
            { "data",       _dataPath   },
            { "working",    workingDir  },
            { "user",       userDir     },
            { "temporary",  tempDir     }
        };

        for (const FDirectory& dir : directories) {
            u64 totalSize, usedSize;
            Verify(FPlatformFile::TotalSizeAndUsage(&totalSize, &usedSize, dir.Path.data()));
            Format(oss, "   {0:9} dir = '{1:50}' : {2:f2}/{3:f2} ({4})",
                dir.Name, ToString(dir.Path),
                Fmt::SizeInBytes(usedSize), Fmt::SizeInBytes(totalSize), Fmt::Percentage(usedSize, totalSize))
                << Eol;
        }
    }
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpPhysicalMemory(FWTextWriter& oss) const {
    FStringBuilder sb;
    DumpPhysicalMemory(sb);
    oss << ToWString(sb.Written());
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpMemoryStats(FWTextWriter& oss) const{
    FStringBuilder sb;
    DumpMemoryStats(sb);
    oss << ToWString(sb.Written());
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpProcessInfos(FWTextWriter& oss) const {
    FStringBuilder sb;
    DumpProcessInfos(sb);
    oss << ToWString(sb.Written());
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpStorageInfos(FWTextWriter& oss) const {
    FStringBuilder sb;
    DumpStorageInfos(sb);
    oss << ToWString(sb.Written());
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpCrashInfos(FTextWriter& oss) const {
    DumpProcessInfos(oss);
    DumpPhysicalMemory(oss);
    DumpMemoryStats(oss);
    DumpStorageInfos(oss);
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpCrashInfos(FWTextWriter& oss) const {
    DumpProcessInfos(oss);
    DumpPhysicalMemory(oss);
    DumpMemoryStats(oss);
    DumpStorageInfos(oss);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
