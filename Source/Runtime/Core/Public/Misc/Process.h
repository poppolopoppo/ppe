#pragma once

#include "Core_fwd.h"

#include "HAL/PlatformProcess.h"
#include "Container/RawStorage_fwd.h"
#include "IO/String_fwd.h"
#include "Meta/enum.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FProcess : public Meta::FNonCopyable {
public:
    using FPipeHandle = FPlatformProcess::FPipeHandle;
    using FProcessId = FPlatformProcess::FProcessId;
    using FProcessHandle = FPlatformProcess::FProcessHandle;
    using FProcessMemory = FPlatformProcess::FMemoryStats;

    enum EProcessFlags {
        None            = 0,

        Detached        = 1<<0,
        Hidden          = 1<<1,
        InheritHandles  = 1<<2,
        NoWindow        = 1<<3,

        RedirectStdin   = 1<<4,
        RedirectStderr  = 1<<5,
        RedirectStdout  = 1<<6,
    };

    FProcess() NOEXCEPT;
    ~FProcess();

    FProcess(
        FProcessId pid,
        FProcessHandle hProc,
        FPipeHandle hStdinWrite = nullptr,
        FPipeHandle hStderrRead = nullptr,
        FPipeHandle hStdoutRead = nullptr ) NOEXCEPT;

    FProcess(FProcess&& rvalue) NOEXCEPT;
    FProcess& operator =(FProcess&& rvalue) NOEXCEPT;

    FProcessId Pid() const { return _pid; }
    FProcessHandle Handle() const { return _hProc; }

    bool IsAlive() const;
    bool IsValid() const { return (0 != _pid); }

    FString Name() const;
    int ExitCode() const;
    FProcessMemory MemoryUsage() const;

    EProcessPriority Priority() const;
    bool SetPriority(EProcessPriority priority);

    size_t WriteStdin(const FRawMemoryConst& buffer);

    size_t ReadStderr(const FRawMemory& buffer);
    size_t ReadStdout(const FRawMemory& buffer);

    using FRawStorage = RAWSTORAGE(Process, u8);

    bool ReadStderr(FRawStorage* outp);
    bool ReadStdout(FRawStorage* outp);

    void WaitFor();
    bool WaitFor(size_t timeoutMs);

    void Close();
    void Terminate(bool killtree = true);

    void Swap(FProcess& other) NOEXCEPT;

public:
    static int CaptureOutput(
        FRawStorage* pStdout,
        FRawStorage* pStderr,
        const FWStringView& url,
        const TMemoryView<const FWStringView>& args,
        const FWStringView& optionalWorkingDir,
        EProcessFlags flags = None,
        EProcessPriority priority = EProcessPriority::Normal );

    static FProcess Current();

    static FProcess Create(
        const FWStringView& url,
        const TMemoryView<const FWStringView>& args,
        const FWStringView& optionalWorkingDir,
        EProcessFlags flags = None,
        EProcessPriority priority = EProcessPriority::Normal );

    static FProcess Open(FProcessId pid, bool fullAccess = false);
    static FProcess Open(const FWStringView& name, bool fullAccess = false);

private:
    FProcessId _pid;
    FProcessHandle _hProc;

    FPipeHandle _hStdinWrite;
    FPipeHandle _hStderrRead;
    FPipeHandle _hStdoutRead;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
