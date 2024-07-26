// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Diagnostic/CurrentProcess.h"

#include "CoreModule.h"
#include "Diagnostic/BuildVersion.h"
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
#include "Modular/ModuleInfo.h"
#include "Time/DateTime.h"
#include "Time/Timestamp.h"

namespace PPE {
LOG_CATEGORY(PPE_CORE_API, Process)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#if USE_PPE_PLATFORM_DEBUG
static bool GStartedWithDebugger{ FPlatformDebug::IsDebuggerPresent() };
#endif
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FCurrentProcess::StartedWithDebugger() {
#if USE_PPE_PLATFORM_DEBUG
    return GStartedWithDebugger;
#else
    return false;
#endif
}
//----------------------------------------------------------------------------
void* FCurrentProcess::class_singleton_storage() NOEXCEPT {
    return singleton_type::make_singleton_storage(); // for shared lib
}
//----------------------------------------------------------------------------
FCurrentProcess::FCurrentProcess(
    void* appHandle, int nShowCmd,
    const wchar_t* filename, size_t argc, const wchar_t * const* argv )
:   _fileName(MakeCStringView(filename))
,   _args(NEW_ARRAY(Process, FWString, argc))
,   _exitCode(0)
,   _appIcon(0)
,   _startDate(FTimestamp::Now())
,   _startTicks(FTimepoint::Now()) {

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

    _dataPath = FPlatformFile::JoinPath({ _directory, L".."_view, L".."_view, L"Data"_view });
    _savedPath = FPlatformFile::JoinPath({ _directory, L".."_view, L"Saved"_view });

    Verify(FPlatformFile::NormalizePath(_dataPath));
    Verify(FPlatformFile::NormalizePath(_savedPath));

    FPlatformFile::CreateDirectory(*_savedPath, nullptr);

    _appHandle = appHandle;
    _nShowCmd = nShowCmd;
#if USE_PPE_PLATFORM_DEBUG
    if (HasArgument(L"-IgnoreDebugger"_view))
        GStartedWithDebugger = false;
#endif

#if USE_PPE_PLATFORM_DEBUG
    if (HasArgument(L"-WaitForDebugger"_view)) {
        GStartedWithDebugger = false; // some parts of the code won't detect that the debugger is attached
        volatile bool bTurnThisOffWhenDebuggerIsAttached = (!FPlatformDebug::IsDebuggerPresent());
        volatile size_t loopCount = 0;
        while (bTurnThisOffWhenDebuggerIsAttached) {
            PPE_LOG(Process, Warning, "waiting for debugger to be attached");
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // wait for debugger to be attached
            const_cast<size_t&>(loopCount)++;
        }
    }
#endif

#if !USE_PPE_FINAL_RELEASE
    if (HasArgument(L"-AbortOnAssert"_view)) {
        SetAssertionHandler([](const char*, const char*, unsigned, bool isEnsure) -> bool {
            if (isEnsure) // ensures are ignored
                return false;
            PPE_DEBUG_CRASH(),
            ::abort();
        });
        SetAssertionReleaseHandler([](const char*, const char*, unsigned) -> bool {
            PPE_DEBUG_CRASH(),
            ::abort();
        });
    }
#endif

#if !USE_PPE_FINAL_RELEASE && USE_PPE_LOGGER
    if (HasArgument(L"-Unattended"_view))
        FLogger::SetGlobalVerbosity(ELoggerVerbosity::Warning);
    if (HasArgument(L"-Quiet"_view))
        FLogger::SetGlobalVerbosity(ELoggerVerbosity::None);
#endif

    LogAllInfos();
}
//----------------------------------------------------------------------------
FCurrentProcess::~FCurrentProcess() {
    PPE_LOG(Process, Info, "exit with code = {0}.", _exitCode.load());
}
//----------------------------------------------------------------------------
void FCurrentProcess::LogAllInfos() const {
    LogProcessInfos();
    LogPhysicalMemory();
    LogStorageInfos();
    LogMemoryStats();
}
//----------------------------------------------------------------------------
void FCurrentProcess::LogPhysicalMemory() const {
    PPE_LOG_DIRECT(Process, Info, [this](FTextWriter& oss) {
        DumpPhysicalMemory(oss);
    });
}
//----------------------------------------------------------------------------
void FCurrentProcess::LogMemoryStats() const {
    PPE_LOG_DIRECT(Process, Info, [this](FTextWriter& oss) {
        DumpMemoryStats(oss);
    });
}
//----------------------------------------------------------------------------
void FCurrentProcess::LogProcessInfos() const {
    PPE_LOG_DIRECT(Process, Info, [this](FTextWriter& oss) {
        DumpProcessInfos(oss);
    });
}
//----------------------------------------------------------------------------
void FCurrentProcess::LogStorageInfos() const {
    PPE_LOG_DIRECT(Process, Info, [this](FTextWriter& oss) {
        DumpStorageInfos(oss);
    });
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpPhysicalMemory(FTextWriter& oss) const {
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
//----------------------------------------------------------------------------
void FCurrentProcess::DumpMemoryStats(FTextWriter& oss) const {
#if USE_PPE_MEMORYDOMAINS
    // take a snapshot of everything at the same time for consistency
    struct FSnapshot_ {
        const FMemoryTracking* Domain;
        const FMemoryTracking::FSnapshot User;
        const FMemoryTracking::FSnapshot System;
        const FMemoryTracking::FSnapshot Wasted;
        FSnapshot_(const FMemoryTracking& domain)
            : Domain(&domain)
            , User(Domain->User())
            , System(Domain->System())
            , Wasted(Domain->Wasted())
        {}
    } domains[] = {
        FMemoryTracking::GpuMemory(),
        FMemoryTracking::VirtualMemory(),
        FMemoryTracking::ReservedMemory(),
        FMemoryTracking::UsedMemory(),
        FMemoryTracking::PooledMemory(),
        FMemoryTracking::UnaccountedMemory(),
    };

    Format(oss, "memory domains =") << Eol;
    for (const FSnapshot_& snapshot : domains) {
        Format(oss, "{0:-20} : {1:9f2} / {2:9f2} / {3:9f2}",
            snapshot.Domain->Name(),
            Fmt::SizeInBytes(snapshot.User.TotalSize),
            Fmt::SizeInBytes(snapshot.System.TotalSize),
            Fmt::SizeInBytes(snapshot.Wasted.TotalSize)
            ) << Eol;
    }
#else
    Unused(oss);
#endif //!USE_PPE_MEMORYDOMAINS
}
//----------------------------------------------------------------------------
void FCurrentProcess::DumpProcessInfos(FTextWriter& oss) const {
    Format(oss, "Process infos:") << Eol;
    {
        Format(oss, "host = {0} @ {1}", FPlatformMisc::UserName(), FPlatformMisc::MachineName()) << Eol;
        Format(oss, "os = {0}", FPlatformMisc::OSName()) << Eol;
        Format(oss, "started with debugger = '{0:A}'", StartedWithDebugger()) << Eol;
        Format(oss, "application handle = '{0}', nShowCmd = '{1}'", _appHandle, _nShowCmd) << Eol;
        Format(oss, "started '{0}' with {1} parameters", _fileName, _args.size()) << Eol;
        forrange(i, 0, _args.size())
            Format(oss, "   [{0:2}] '{1}'", i, _args[i]) << Eol;
    }
    {
        const FBuildVersion build = CurrentBuildVersion();
        Format(oss, "build version =") << Eol;
        Format(oss, "   branch = {0}", build.Branch) << Eol;
        Format(oss, "   revision = {0}", build.Revision) << Eol;
        Format(oss, "   family = {0}", build.Family) << Eol;
        Format(oss, "   compiler = {0}", build.Compiler) << Eol;
        Format(oss, "   timestamp = {0}", build.Timestamp.ToDateTime()) << Eol;
    }
    {
        const ITargetPlaftorm& platform = CurrentPlatform();
        Format(oss, "platform = {0} ({1})", platform.DisplayName(), CurrentPlatform().FullName()) << Eol;
        Format(oss, "   client supported = {0:A}", platform.SupportsFeature(EPlatformFeature::Client)) << Eol;
        Format(oss, "   server supported = {0:A}", platform.SupportsFeature(EPlatformFeature::Server)) << Eol;
        Format(oss, "   editor supported = {0:A}", platform.SupportsFeature(EPlatformFeature::Editor)) << Eol;
        Format(oss, "   data generation supported = {0:A}", platform.SupportsFeature(EPlatformFeature::DataGeneration)) << Eol;
        Format(oss, "   high quality supported = {0:A}", platform.SupportsFeature(EPlatformFeature::HighQuality)) << Eol;
        Format(oss, "   cooked data supported = {0:A}", platform.SupportsFeature(EPlatformFeature::CookedData)) << Eol;
    }
    {
        const FPlatformMemory::FConstants mem = FPlatformMemory::Constants();
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
            { "start"_view,      _directory  },
            { "data"_view,       _dataPath   },
            { "working"_view,    workingDir  },
            { "user"_view,       userDir     },
            { "temporary"_view,  tempDir     }
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
bool FCurrentProcess::HasArgument(FWStringLiteral literal) const NOEXCEPT {
    return HasArgument(literal.MakeView());
}
//----------------------------------------------------------------------------
bool FCurrentProcess::HasArgument(const FWStringView& arg) const NOEXCEPT {
    return (_args.end() != _args.FindIf(
        [arg](const FWString& it) NOEXCEPT -> bool {
            return EqualsI(it, arg);
        }) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
