#pragma once

#include "Core.h"

#include "IO/String.h"
#include "Memory/UniqueView.h"
#include "Meta/Singleton.h"
#include "Time/Timepoint.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FCurrentProcess : Meta::TSingleton<FCurrentProcess> {
public:
    typedef Meta::TSingleton<FCurrentProcess> parent_type;

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

    void DumpMemoryStats() const;
    void DumpProcessInfos() const;

    const FTimepoint& StartedAt() const { return _startedAt; }

    using parent_type::Get;
#ifdef WITH_PPE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create(void* appHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t **argv) {
        parent_type::Create(appHandle, nShowCmd, filename, argc, argv);
    }

    static FSeconds ElapsedSeconds() {
        return FSeconds(FTimepoint::ElapsedSince(Get().StartedAt()));
    }

private:
    friend class Meta::TSingleton<FCurrentProcess>;
    FCurrentProcess(void* appHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t **argv);

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

    FTimepoint _startedAt;
};
//----------------------------------------------------------------------------
inline const FCurrentProcess& CurrentProcess() {
    return FCurrentProcess::Get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
