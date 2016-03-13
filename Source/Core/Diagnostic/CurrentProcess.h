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
class CurrentProcess : Meta::Singleton<CurrentProcess> {
public:
    typedef Meta::Singleton<CurrentProcess> parent_type;

    ~CurrentProcess();

    const WString& FileName() const { return _fileName; }
    const WString& Directory() const { return _directory; }
    MemoryView<const WString> Args() const { return MakeView(_args); }

    void *ApplicationHandle() const { return _applicationHandle; }
    int nShowCmd() const { return _nShowCmd; }

    int ExitCode() const { return _exitCode; }
    void SetExitCode(int value) { _exitCode = value; }

    size_t AppIcon() const { return _appIcon; }
    void SetAppIcon(size_t value) { _appIcon = value; }

    const Timepoint& StartedAt() const { return _startedAt; }

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t **argv) {
        parent_type::Create(applicationHandle, nShowCmd, argc, argv);
    }

    static Units::Time::Seconds ElapsedSeconds() {
        return HasInstance()
            ? Timepoint::ElapsedSince(Instance().StartedAt())
            : Units::Time::Seconds(0);
    }

private:
    friend class Meta::Singleton<CurrentProcess>;
    CurrentProcess(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t **argv);

    WString _fileName;
    WString _directory;
    UniqueArray<WString> _args;

    void *_applicationHandle;
    int _nShowCmd;

    std::atomic<int> _exitCode;

    size_t _appIcon;

    Timepoint _startedAt;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
