#pragma once

#include "Core/Core.h"

#include "Core/IO/String.h"
#include "Core/Memory/UniqueView.h"
#include "Core/Meta/Singleton.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class CurrentProcess;
//----------------------------------------------------------------------------
class CurrentProcessData {
public:
    CurrentProcessData(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t **argv);
    ~CurrentProcessData();

    CurrentProcessData(const CurrentProcessData&) = delete;
    CurrentProcessData& operator =(const CurrentProcessData&) = delete;

    const WString& FileName() const { return _fileName; }
    const WString& Directory() const { return _directory; }
    MemoryView<const WString> Args() const { return MakeView(_args); }

    void *ApplicationHandle() const { return _applicationHandle; }
    int nShowCmd() const { return _nShowCmd; }

    int ExitCode() const { return _exitCode; }
    void SetExitCode(int value) { _exitCode = value; }

    size_t AppIcon() const { return _appIcon; }
    void SetAppIcon(size_t value) { _appIcon = value; }

private:
    friend struct Meta::Activator<CurrentProcessData>;

    WString _fileName;
    WString _directory;
    UniqueArray<WString> _args;

    void *_applicationHandle;
    int _nShowCmd;

    std::atomic<int> _exitCode;

    size_t _appIcon;
};
//----------------------------------------------------------------------------
class CurrentProcess : Meta::Singleton<CurrentProcessData, CurrentProcess> {
public:
    typedef Meta::Singleton<CurrentProcessData, CurrentProcess> parent_type;

    using parent_type::Instance;
    using parent_type::HasInstance;
    using parent_type::Destroy;

    static void Create(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t **argv) {
        parent_type::Create(applicationHandle, nShowCmd, argc, argv);
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
