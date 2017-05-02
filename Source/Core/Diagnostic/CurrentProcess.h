#pragma once

#include "Core/Core.h"

#include "Core/IO/String.h"
#include "Core/Memory/UniqueView.h"
#include "Core/Meta/Singleton.h"
#include "Core/Time/Timepoint.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FCurrentProcess : Meta::TSingleton<FCurrentProcess> {
public:
    typedef Meta::TSingleton<FCurrentProcess> parent_type;

    ~FCurrentProcess();

    const FWString& FileName() const { return _fileName; }
    const FWString& Directory() const { return _directory; }
    TMemoryView<const FWString> Args() const { return MakeView(_args); }

    void *ApplicationHandle() const { return _applicationHandle; }
    int nShowCmd() const { return _nShowCmd; }

    int ExitCode() const { return _exitCode; }
    void SetExitCode(int value) { _exitCode = value; }

    size_t AppIcon() const { return _appIcon; }
    void SetAppIcon(size_t value) { _appIcon = value; }

    const FTimepoint& StartedAt() const { return _startedAt; }

    using parent_type::Instance;
#ifdef WITH_CORE_ASSERT
    using parent_type::HasInstance;
#endif
    using parent_type::Destroy;

    static void Create(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t **argv) {
        parent_type::Create(applicationHandle, nShowCmd, argc, argv);
    }

    static FSeconds ElapsedSeconds() {
        return FSeconds(FTimepoint::ElapsedSince(Instance().StartedAt()));
    }

private:
    friend class Meta::TSingleton<FCurrentProcess>;
    FCurrentProcess(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t **argv);

    FWString _fileName;
    FWString _directory;
    TUniqueArray<FWString> _args;

    void *_applicationHandle;
    int _nShowCmd;

    std::atomic<int> _exitCode;

    size_t _appIcon;

    FTimepoint _startedAt;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
