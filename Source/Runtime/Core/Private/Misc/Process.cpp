#include "stdafx.h"

#include "Misc/Process.h"

#include "Container/RawStorage.h"
#include "IO/String.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool ReadPipe_AppendOutp_(FProcess::FRawStorage& outp, FProcess::FPipeHandle pipe) {
    size_t total = 0;

    while (size_t toRead = FPlatformProcess::PeekPipe(pipe)) {
        const size_t off = outp.size();
        outp.Resize_KeepData(off + toRead);

        const size_t read = FPlatformProcess::ReadPipe(pipe, outp.MakeView().CutStartingAt(off));
        if (read != toRead)
            outp.Resize_KeepData(off + read);

        total += read;
    }

    return (total > 0);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FProcess::FProcess() NOEXCEPT
:   _pid(0)
,   _hProc(nullptr)
,   _hStdinWrite(nullptr)
,   _hStderrRead(nullptr)
,   _hStdoutRead(nullptr)
{}
//----------------------------------------------------------------------------
FProcess::~FProcess() {
    if (_hProc)
        Close();

    Assert_NoAssume(nullptr == _hProc);
    Assert_NoAssume(nullptr == _hStdinWrite);
    Assert_NoAssume(nullptr == _hStderrRead);
    Assert_NoAssume(nullptr == _hStdoutRead);
}
//----------------------------------------------------------------------------
FProcess::FProcess(
    FProcessId pid,
    FProcessHandle hProc,
    FPipeHandle hStdinWrite /* = nullptr */,
    FPipeHandle hStderrRead /* = nullptr */,
    FPipeHandle hStdoutRead /* = nullptr */ ) NOEXCEPT
:   _pid(pid)
,   _hProc(hProc)
,   _hStdinWrite(hStdinWrite)
,   _hStderrRead(hStderrRead)
,   _hStdoutRead(hStdoutRead) {
    Assert_NoAssume(IsValid());
    Assert_NoAssume(not (_hStdinWrite && FPlatformProcess::IsPipeBlocked(_hStdinWrite)));
    Assert_NoAssume(not (_hStderrRead && FPlatformProcess::IsPipeBlocked(_hStderrRead)));
    Assert_NoAssume(not (_hStdoutRead && FPlatformProcess::IsPipeBlocked(_hStdoutRead)));
}
//----------------------------------------------------------------------------
FProcess::FProcess(FProcess&& rvalue) NOEXCEPT : FProcess() {
    Swap(rvalue);
}
//----------------------------------------------------------------------------
auto FProcess::operator =(FProcess&& rvalue) NOEXCEPT -> FProcess& {
    if (_hProc)
        Close();

    Assert_NoAssume(nullptr == _hProc);
    Assert_NoAssume(nullptr == _hStdinWrite);
    Assert_NoAssume(nullptr == _hStderrRead);
    Assert_NoAssume(nullptr == _hStdoutRead);

    Swap(rvalue);
    return (*this);
}
//----------------------------------------------------------------------------
bool FProcess::IsAlive() const {
    return FPlatformProcess::IsProcessAlive(_hProc);
}
//----------------------------------------------------------------------------
FString FProcess::Name() const {
    FString processName;
    Verify(FPlatformProcess::Name(&processName, _hProc));
    return processName;
}
//----------------------------------------------------------------------------
int FProcess::ExitCode() const {
    int exitcode;
    Verify(FPlatformProcess::ExitCode(&exitcode, _hProc));
    return exitcode;
}
//----------------------------------------------------------------------------
auto FProcess::MemoryUsage() const -> FProcessMemory {
    FProcessMemory mem;
    Verify(FPlatformProcess::MemoryStats(&mem, _hProc));
    return mem;
}
//----------------------------------------------------------------------------
EProcessPriority FProcess::Priority() const {
    EProcessPriority priority = EProcessPriority::Normal;
    Verify(FPlatformProcess::Priority(&priority, _hProc));
    return priority;
}
//----------------------------------------------------------------------------
bool FProcess::SetPriority(EProcessPriority priority) {
    return FPlatformProcess::SetPriority(_hProc, priority);
}
//----------------------------------------------------------------------------
size_t FProcess::WriteStdin(const FRawMemoryConst& buffer) {
    Assert(_hStdinWrite);
    return FPlatformProcess::WritePipe(_hStdinWrite, buffer);
}
//----------------------------------------------------------------------------
size_t FProcess::ReadStderr(const FRawMemory& buffer) {
    Assert(_hStderrRead);
    return FPlatformProcess::ReadPipe(_hStderrRead, buffer);
}
//----------------------------------------------------------------------------
size_t FProcess::ReadStdout(const FRawMemory& buffer) {
    Assert(_hStdoutRead);
    return FPlatformProcess::ReadPipe(_hStdoutRead, buffer);
}
//----------------------------------------------------------------------------
bool FProcess::ReadStderr(FRawStorage* outp) {
    Assert(outp);
    Assert(_hStderrRead);
    return ReadPipe_AppendOutp_(*outp, _hStderrRead);
}
//----------------------------------------------------------------------------
bool FProcess::ReadStdout(FRawStorage* outp) {
    Assert(outp);
    Assert(_hStdoutRead);
    return ReadPipe_AppendOutp_(*outp, _hStdoutRead);
}
//----------------------------------------------------------------------------
void FProcess::WaitFor() {
    FPlatformProcess::WaitForProcess(_hProc);
}
//----------------------------------------------------------------------------
bool FProcess::WaitFor(size_t timeoutMs) {
    return FPlatformProcess::WaitForProcess(_hProc, timeoutMs);
}
//----------------------------------------------------------------------------
void FProcess::Close() {
    FPlatformProcess::ClosePipe(nullptr, _hStdinWrite);
    FPlatformProcess::ClosePipe(_hStderrRead, nullptr);
    FPlatformProcess::ClosePipe(_hStdoutRead, nullptr);

    FPlatformProcess::CloseProcess(_hProc);

    _hProc = nullptr;
    _hStdinWrite = _hStderrRead = _hStdoutRead = nullptr;
}
//----------------------------------------------------------------------------
void FProcess::Terminate(bool killtree/* = true */) {
    FPlatformProcess::TerminateProcess(_hProc, killtree);
    Close();
}
//----------------------------------------------------------------------------
void FProcess::Swap(FProcess& other) NOEXCEPT {
    std::swap(_pid, other._pid);
    std::swap(_hProc, other._hProc);

    std::swap(_hStdinWrite, other._hStdinWrite);
    std::swap(_hStderrRead, other._hStderrRead);
    std::swap(_hStdoutRead, other._hStdoutRead);
}
//----------------------------------------------------------------------------
int FProcess::CaptureOutput(
    FRawStorage* pStdout,
    FRawStorage* pStderr,
    const FWStringView& url,
    const TMemoryView<const FWStringView>& args,
    const FWStringView& optionalWorkingDir /* = FWStringView{} */,
    EProcessFlags flags /* = None */,
    EProcessPriority priority /* = EProcessPriority::Normal */) {
    FProcess proc = Create(url, args, optionalWorkingDir,
        static_cast<EProcessFlags>(flags |
            (pStderr ? RedirectStderr : 0) |
            (pStdout ? RedirectStdout : 0) |
            (!!pStdout|!!pStderr ? InheritHandles : 0) ),
        priority );

    if (not proc.IsValid())
        return -1;

    do {
        if (pStderr) proc.ReadStderr(pStderr);
        if (pStdout) proc.ReadStdout(pStdout);

        FPlatformProcess::Sleep(0);

    } while (proc.IsAlive());

    if (pStderr) proc.ReadStderr(pStderr);
    if (pStdout) proc.ReadStdout(pStdout);

    return proc.ExitCode();
}
//----------------------------------------------------------------------------
FProcess FProcess::Current() {
    return FProcess(FPlatformProcess::CurrentPID(), FPlatformProcess::CurrentProcess());
}
//----------------------------------------------------------------------------
FProcess FProcess::Create(
    const FWStringView& url,
    const TMemoryView<const FWStringView>& args,
    const FWStringView& optionalWorkingDir /* = FWStringView{} */,
    EProcessFlags flags /* = None */,
    EProcessPriority priority /* = EProcessPriority::Normal */) {

    FPipeHandle hStdinRead{ 0 }, hStdinWrite{ 0 };
    if (flags & RedirectStdin)
        VerifyRelease(FPlatformProcess::CreatePipe(&hStdinRead, &hStdinWrite, true));

    FPipeHandle hStderrRead{ 0 }, hStderrWrite{ 0 };
    if (flags & RedirectStderr)
        VerifyRelease(FPlatformProcess::CreatePipe(&hStderrRead, &hStderrWrite, false));

    FPipeHandle hStdoutRead{ 0 }, hStdoutWrite{ 0 };
    if (flags & RedirectStdout)
        VerifyRelease(FPlatformProcess::CreatePipe(&hStdoutRead, &hStdoutWrite, false));

    if (!!hStdinRead | !!hStderrWrite | !!hStdoutWrite)
        flags = static_cast<EProcessFlags>(flags | InheritHandles);

    FProcessId pid = 0;
    const FProcessHandle hProc = FPlatformProcess::CreateProcess(
        &pid,
        url, args, optionalWorkingDir,
        !!(flags & Detached),
        !!(flags & Hidden),
        !!(flags & InheritHandles),
        !!(flags & NoWindow),
        priority,
        hStdinRead, hStderrWrite, hStdoutWrite );

    if (hProc) {
        FPlatformProcess::ClosePipe(hStdinRead, nullptr);
        FPlatformProcess::ClosePipe(nullptr, hStderrWrite);
        FPlatformProcess::ClosePipe(nullptr, hStdoutWrite);

        return FProcess{ pid, hProc, hStdinWrite, hStderrRead, hStdoutRead };
    }
    else {
        FPlatformProcess::ClosePipe(hStdinRead, hStdinWrite);
        FPlatformProcess::ClosePipe(hStderrRead, hStderrWrite);
        FPlatformProcess::ClosePipe(hStdoutRead, hStdoutWrite);

        return FProcess{};
    }
}
//----------------------------------------------------------------------------
FProcess FProcess::Open(FProcessId pid, bool fullAccess/* = false */) {
    return FProcess{ pid, FPlatformProcess::OpenProcess(pid, fullAccess) };
}
//----------------------------------------------------------------------------
FProcess FProcess::Open(const FWStringView& name, bool fullAccess/* = false */) {
    FProcessId pid;
    if (FPlatformProcess::FindByName(&pid, name))
        return Open(pid, fullAccess);
    else
        return FProcess{};
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
