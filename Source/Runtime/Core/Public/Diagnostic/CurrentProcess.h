#pragma once

#include "Core.h"

#include "IO/String.h"
#include "IO/TextWriter_fwd.h"
#include "Memory/UniqueView.h"
#include "Meta/Singleton.h"
#include "Time/Timepoint.h"
#include "Time/Timestamp.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FCurrentProcess : Meta::TSingleton<FCurrentProcess> {
    friend class Meta::TSingleton<FCurrentProcess>;
    using singleton_type = Meta::TSingleton<FCurrentProcess>;
    static DLL_NOINLINE void* class_singleton_storage() NOEXCEPT; // for shared lib

public:
    ~FCurrentProcess();

    const FWString& ExecutableName() const { return _executableName; }
    const FWString& FileName() const { return _fileName; }
    const FWString& Directory() const { return _directory; }
    TMemoryView<const FWString> Args() const { return MakeView(_args); }

    const FWString& DataPath() const { return _dataPath; }
    const FWString& SavedPath() const { return _savedPath; }

    void *AppHandle() const { return _appHandle; }
    int nShowCmd() const { return _nShowCmd; }

    int ExitCode() const { return _exitCode; }
    void SetExitCode(int value) { _exitCode = value; }

    size_t AppIcon() const { return _appIcon; }
    void SetAppIcon(size_t value) { _appIcon = value; }

    bool StartedWithDebugger() const { return _startedWithDebugger; }

    void DumpPhysicalMemory(FTextWriter& oss) const;
    void DumpMemoryStats(FTextWriter& oss) const;
    void DumpProcessInfos(FTextWriter& oss) const;
    void DumpStorageInfos(FTextWriter& oss) const;

    void DumpPhysicalMemory(FWTextWriter& oss) const;
    void DumpMemoryStats(FWTextWriter& oss) const;
    void DumpProcessInfos(FWTextWriter& oss) const;
    void DumpStorageInfos(FWTextWriter& oss) const;

    void DumpCrashInfos(FTextWriter& oss) const;
    void DumpCrashInfos(FWTextWriter& oss) const;

    void LogAllInfos() const;
    void LogMemoryStats() const;
    void LogPhysicalMemory() const;
    void LogProcessInfos() const;
    void LogStorageInfos() const;

    const FTimestamp& StartDate() const { return _startDate; }
    const FTimepoint& StartTicks() const { return _startTicks; }

    bool HasArgument(const FWStringView& arg) const NOEXCEPT;

    using singleton_type::Get;
#if USE_PPE_ASSERT
    using singleton_type::HasInstance;
#endif
    using singleton_type::Destroy;

    static void Create(void* appHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t * const* argv) {
        singleton_type::Create(appHandle, nShowCmd, filename, argc, argv);
    }

    static FSeconds ElapsedSeconds() {
        return FSeconds(FTimepoint::ElapsedSince(Get().StartTicks()));
    }

private:
    FCurrentProcess(
        void* appHandle, int nShowCmd,
        const wchar_t* filename, size_t argc, const wchar_t * const* argv );

    FWString _executableName;
    FWString _fileName;
    FWString _directory;
    TUniqueArray<FWString> _args;

    FWString _dataPath;
    FWString _savedPath;

    void* _appHandle;
    int _nShowCmd;

    std::atomic<int> _exitCode;

    size_t _appIcon;
    bool _startedWithDebugger;

    const FTimestamp _startDate;
    const FTimepoint _startTicks;
};
//----------------------------------------------------------------------------
inline const FCurrentProcess& CurrentProcess() {
    return FCurrentProcess::Get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
