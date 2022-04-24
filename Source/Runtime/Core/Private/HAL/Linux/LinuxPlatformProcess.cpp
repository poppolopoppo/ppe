#include "stdafx.h"

#include "HAL/Linux/LinuxPlatformProcess.h"

#ifdef PLATFORM_LINUX

#include "Allocator/TrackingMalloc.h"
#include "Diagnostic/Logger.h"
#include "Container/Vector.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StaticString.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Memory/MemoryProvider.h"
#include "Meta/Utility.h"

#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"

#include "HAL/Linux/Errno.h"
#include "HAL/Linux/LinuxPlatformString.h"
#include "HAL/Linux/LinuxPlatformTime.h"

#include <dlfcn.h>
#include <limits.h>
#include <semaphore.h>
#include <signal.h>
#include <spawn.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

extern "C" char** environ; // provided by libc
namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void XdgOpen_(const wchar_t* path) {
    ::pid_t child = ::fork();
    LOG(HAL, Verbose, L"XdgOpen({0})", MakeCStringView(path));

    if (0 == child) {
        ::exit(::execl("/usr/bin/xdg-open", "xdg-open",
            WCHAR_TO_UTF_8<>(path), (char*)0) );
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLinuxProcessState::FLinuxProcessState(::pid_t pid, bool fireAndForget) NOEXCEPT
:   _pid(pid)
//  assumes process is running
,   _state(kRunning | (fireAndForget ? kFireForget : 0))
,   _exitCode(-1) {
}
//----------------------------------------------------------------------------
FLinuxProcessState::~FLinuxProcessState() {
    if (not (_state & kFireForget)) {
        if (_state & kRunning) {
            LOG(HAL, Warning,
                L"closing a process handle while the process (pid={0}) is still running - we will block until it exits to prevent a zombie",
                Pid() );
        }
        else if (not (_state & kWaitFor)) {
            LOG(HAL, Warning,
                L"closing a process handle of a process (pid={0}) that has not been wait()ed for - will wait now to reap a zombie",
                Pid() );
        }

        Wait();
    }
}
//----------------------------------------------------------------------------
bool FLinuxProcessState::IsRunning() {
    if (_state & kRunning) {
        Assert(not (_state & kWaitFor));

        int killResult = ::kill(Pid(), 0); // no actual signal is sent
        Assert(killResult != -1 || errno != EINVAL);

        if (killResult == 0 || (killResult == -1 && errno == EPERM)) {
            for (;;) { // infinite loop in case w get EINTR and have to repeat
                ::siginfo_t sig;
                sig.si_pid = 0; // if remains 0, treat as child was not waitable (i.e. was running)
                if (::waitid(P_PID, Pid(), &sig, WEXITED | WNOHANG | WNOWAIT)) {
                    if (errno != EINTR) {
                        const FErrno thisErrno;
                        LOG(HAL, Fatal,
                            L"waitid() for pid={0} failed with errno: {1}",
                            Pid(), thisErrno );
                        break; // exit the loop if for some reason fatal log above returns
                    }
                }
                else if (sig.si_pid == Pid()) {
                    _state = _state - kRunning;
                    break;
                }
            }
        }
        else {
            _state = _state - kRunning;
        }

        // if child if a zombie, wait() immediately to free up kernel resources.
        // will avoid higher level code to maintain handles for process no longer running.
        if (not (_state & kRunning)) {
            LOG(HAL, Verbose, L"child with pid={0} is no longer running (zombie), wait()ing immediately", Pid());
            Wait();
        }
    }

    return (_state & kRunning);
}
//----------------------------------------------------------------------------
bool FLinuxProcessState::ExitCode(i32* pExitCode) {
    AssertMessage(L"cannot get exit code of a running process", not (_state & kRunning));

    if (not (_state & kWaitFor))
        Wait();

    if (_exitCode != -1) {
        if (pExitCode)
            *pExitCode = _exitCode;
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
void FLinuxProcessState::Wait() {
    if (_state & kWaitFor)
        return; // bail out if already wait()ed once

    for (;;) {
        ::siginfo_t sig;
        if (::waitid(P_PID, Pid(), &sig, WEXITED)) {
            if (errno != EINTR) {
                const FErrno thisErrno;
                LOG(HAL, Fatal,
                    L"waitid() for pid={0} failed with errno: {1}",
                    Pid(), thisErrno );
                break; // exit the loop if for some reason fatal log above returns
            }
        }
        else {
            Assert(sig.si_pid == Pid());
            Assert(_state & kRunning);

            _exitCode = (sig.si_code == CLD_EXITED ? sig.si_status : -1);
            _state = _state + kWaitFor - kRunning;

            LOG(HAL, Verbose, L"child with pid={0} exited with code {1}", Pid(), _exitCode);
            break;
        }
    }
}
//----------------------------------------------------------------------------
bool FLinuxProcessState::WaitFor(int timeoutMs) {
    if (_state & kWaitFor)
        return true; // bail out if already wait()ed once

    for (;;) {
        ::siginfo_t sig;
        if (::waitid(P_PID, Pid(), &sig, WNOHANG)) {
            if (errno != EINTR) {
                const FErrno thisErrno;
                LOG(HAL, Fatal,
                    L"waitid() for pid={0} failed this errno: {1}",
                    Pid(), thisErrno );
                break; // exit the loop if for some reason fatal log above returns
            }
        }
        else {
            Assert(sig.si_pid == Pid());
            Assert(_state & kRunning);

            if (sig.si_code == CLD_EXITED) {
                _exitCode = sig.si_status;
                _state = _state + kWaitFor - kRunning;

                LOG(HAL, Verbose, L"child with pid={0} exited with code {1}", Pid(), _exitCode);

                return true;
            }

            if (timeoutMs) {
                timeoutMs = 0;
                ::sleep(1);
            }
            else
                break;
        }
    }

    return false;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::OnProcessStart(
    void* appHandle, int nShowCmd,
    const wchar_t* filename, size_t argc, const wchar_t* const* argv ) {
    FGenericPlatformProcess::OnProcessStart(appHandle, nShowCmd, filename, argc, argv);

    FLinuxPlatformTime::InitTiming();
}
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::OnProcessShutdown() {
    FGenericPlatformProcess::OnProcessShutdown();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::Sleep(float seconds) {
    if (0 == seconds)
        std::this_thread::yield();
    else
        ::sleep((int)seconds);
}
//----------------------------------------------------------------------------
// Hybrid spinning
// https://probablydance.com/2019/12/30/measuring-mutexes-spinlocks-and-how-bad-the-linux-scheduler-really-is/
void FLinuxPlatformProcess::SleepForSpinning(i32& backoff) {
    if (backoff < 16) {
        ++backoff;
        ::_mm_pause();
    }
    else {
        backoff = 0;
        std::this_thread::yield();
    }
}
//----------------------------------------------------------------------------
auto FLinuxPlatformProcess::CurrentPID() -> FProcessId {
    return ::getpid();
}
//----------------------------------------------------------------------------
auto FLinuxPlatformProcess::CurrentProcess() -> FProcessHandle {
    return OpenProcess(::getpid());
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::EnableDebugPrivileges() {
    return true;
}
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::Daemonize() {
    if (::daemon(1, 1) == -1) {
        const FErrno thisErrno;
        LOG(HAL, Error, L"daemon(1, 1) failed with errno: {0}",
            thisErrno );
    }
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::IsForeground() {
    // #TODO
    AssertNotImplemented();
    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::IsFirstInstance() {
    // #TODO
    AssertNotImplemented();
    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::IsProcessAlive(FProcessHandle process) {
    Assert(process.Valid());

    bool running = false;
    if (FLinuxProcessState* const procInfo = process.ProcInfo()) {
        running = procInfo->IsRunning();
    }
    else {
        // process opened with OpenProcess() call (we only have pid)

        int killResult = ::kill(process.UnpackedPid(), 0); // no actual signal is sent
        Assert((killResult != -1)|(errno != EINVAL));

        // errno == EPERM: don't have permissions to send signal
        // errno == ESRCH: process doesn't exist
        running = (killResult == 0);
    }

    return running;
}
//----------------------------------------------------------------------------
auto FLinuxPlatformProcess::OpenProcess(FProcessId pid, bool fullAccess/* = true */) -> FProcessHandle {
    Unused(fullAccess);

    // check if actually running
    int killResult = ::kill(pid, 0); // no actual signal is sent
    Assert(killResult != -1 || errno != EINVAL);

    // errno == EPERM: don't have permissions to send signal
    // errno == ESRCH: process doesn't exist
    return FLinuxProcessHandle(killResult == 0 ? pid : -1);
}
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::WaitForProcess(FProcessHandle process) {
    Assert(process.Valid());

    if (FLinuxProcessState* const procInfo = process.ProcInfo()) {
        procInfo->Wait();
    }
    else {
        AssertNotImplemented(); // #TODO support for OpenProcess()
    }
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::WaitForProcess(FProcessHandle process, size_t timeoutMs) {
    Assert(process.Valid());

    if (FLinuxProcessState* const procInfo = process.ProcInfo()) {
        return procInfo->WaitFor(timeoutMs);
    }
    else {
        AssertNotImplemented(); // #TODO support for OpenProcess()
        return false;
    }
}
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::CloseProcess(FProcessHandle process) {
    Assert(process.Valid());
    TRACKING_DELETE(Process, process.ProcInfo());
}
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::TerminateProcess(FProcessHandle process, bool killTree) {
    Assert(process.Valid());

    if (killTree) {
        // #TODO enumerate children
        AssertNotImplemented();
    }

    if (FLinuxProcessState* procInfo = process.ProcInfo()) {
        int killResult = ::kill(procInfo->Pid(), SIGTERM); // graceful
        Assert(killResult != -1 || errno != EINVAL);
    }
    else if (process.Pid() != -1) {
        AssertNotImplemented(); // #TODO support for OpenProcess()
    }
}
//------------------------------------------------------------------------
bool FLinuxPlatformProcess::FindByName(FProcessId* pPid, const FWStringView& name) {
    Assert(pPid);
    Assert(not name.empty());

    FString cmdline = "pidof -s '";
    cmdline += WCHAR_TO_UTF_8<>(name).Str();
    cmdline += "'";

    ::FILE* cmdPipe = ::popen(cmdline.data(), "r");

    char buf[512];
    ::fgets(buf, lengthof(buf), cmdPipe);
    unsigned long parsedPid = ::strtoul(buf, NULL, 10);

    if (pPid)
        *pPid = (FProcessId)parsedPid;

    return (::pclose(cmdPipe) != -1 && parsedPid != ULONG_MAX);
}
//------------------------------------------------------------------------
// process infos
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::ExitCode(int* pExitCode, FProcessHandle process) {
    Assert(pExitCode);
    Assert(process.Valid());

    if (FLinuxProcessState* procInfo = process.ProcInfo()) {
        return procInfo->ExitCode(pExitCode);
    }
    else {
        AssertNotImplemented(); // #TODO support for OpenProcess()
        return false;
    }
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::MemoryStats(FMemoryStats* pStats, FProcessHandle process) {
    Assert(pStats);
    Assert(process.Valid());
    Unused(pStats);
    AssertNotImplemented(); // #TODO popen() ?
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::Name(FString* pName, FProcessHandle process) {
    Assert(pName);
    Assert(process.Valid());

    FString cmdline = "ps -o comm= -p ";
    cmdline += ToString(process.Pid());

    ::FILE* cmdPipe = ::popen(cmdline.data(), "r");

    char buf[512];
    ::fgets(buf, lengthof(buf), cmdPipe);
    pName->assign(MakeCStringView(buf));

    return (::pclose(cmdPipe) != -1);
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::Pid(FProcessId* pPid, FProcessHandle process) {
    Assert(pPid);

    if (process.Valid()) {
        *pPid = process.Pid();
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::Priority(EProcessPriority* pPriority, FProcessHandle process) {
    Assert(pPriority);
    Assert(process.Valid());

    int selfPriority = ::getpriority(PRIO_PROCESS, 0);
    if (errno != 0) {
        const FErrno thisErrno;
        LOG(HAL, Warning, L"could not get current process priority, errno: {0}",
            thisErrno );

        return false;
    }

    int otherPriority = ::getpriority(PRIO_PROCESS, process.Pid());
    if (errno != 0) {
        const FErrno thisErrno;
        LOG(HAL, Warning, L"could not get process priority with pid={0}, errno: {1}",
            process.Pid(), thisErrno );

        return false;
    }

    if (otherPriority == selfPriority)
        *pPriority = EProcessPriority::Normal;
    else if (otherPriority < selfPriority)
        *pPriority = EProcessPriority::AboveNormal;
    else if (otherPriority > selfPriority)
        *pPriority = EProcessPriority::BelowNormal;

    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::SetPriority(FProcessHandle process, EProcessPriority priority) {
    Assert(process.Valid());

    errno = 0;
    int actualPriority = ::getpriority(PRIO_PROCESS, process.Pid());

    if (errno != 0) {
        const FErrno thisErrno;
        LOG(HAL, Warning, L"could not get process priority with pid={0}, errno: {1}",
            process.Pid(), thisErrno );

        return false;
    }

    int priorityMax = 0;
    ::rlimit priorityLimits;
    if (::getrlimit(RLIMIT_NICE, &priorityLimits) == -1) {
        const FErrno thisErrno;
        LOG(HAL, Warning, L"could not get process priority limits, errno: {0}",
            thisErrno );

        return false;
    }
    else {
        priorityMax = 20 - priorityLimits.rlim_cur;
    }

    int newPriority = actualPriority;
    switch (priority) {
    case EProcessPriority::Idle: newPriority += 10; break;
    case EProcessPriority::BelowNormal: newPriority += 5; break;
    case EProcessPriority::Normal: break;
    case EProcessPriority::AboveNormal: newPriority -= 10; break;
    case EProcessPriority::Realtime: newPriority -= 15; break;
    default:
        AssertNotImplemented();
    }

    // cap to [RLIMIT_NICE, 19]
    newPriority = Min(19, newPriority);
    newPriority = Max(priorityMax, newPriority);

    if (::setpriority(PRIO_PROCESS, process.Pid(), newPriority) == -1) {
        const FErrno thisErrno;
        LOG(HAL, Warning, L"failed to set process priority with pid={0}, errno: {1}",
            process.Pid(), thisErrno );

        return false;
    }
    else {
        LOG(HAL, Verbose, L"change process pid={0} priority from {1} to {2} ({3})",
            process.Pid(), actualPriority, newPriority, (int)priority );

        return true;
    }
}
//----------------------------------------------------------------------------
// Use a constref to avoid issues with static variables initialization order !
const FLinuxPlatformProcess::FAffinityMask& FLinuxPlatformProcess::AllCoresAffinity =
    FLinuxPlatformThread::AllCoresAffinity;
//----------------------------------------------------------------------------
auto FLinuxPlatformProcess::AffinityMask(FProcessHandle process) -> FAffinityMask {
    Assert(process.Valid());
    Unused(process);
    AssertNotImplemented();
    return FAffinityMask{ 0 };
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::SetAffinityMask(FProcessHandle process, FAffinityMask mask) {
    Assert(process.Valid());
    Unused(process);
    Unused(mask);
    AssertNotImplemented(); // #TODO ???
    return false;
}
//----------------------------------------------------------------------------
// Pipes
//----------------------------------------------------------------------------
static int PipeFD_(FLinuxPlatformProcess::FPipeHandle pipe) {
    return (int)((intptr_t)pipe);
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::IsPipeBlocked(FPipeHandle pipe) {
    int fd = PipeFD_(pipe);
    int bytesAvailable = 0; // ignored
    return (::ioctl(fd, FIONREAD, &bytesAvailable) == -1);
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::CreatePipe(FPipeHandle* pRead, FPipeHandle* pWrite, bool shareRead) {
    Assert(pRead);
    Assert(pWrite);

    Unused(shareRead);

    int pipeFd[2];
    if (-1 == ::pipe(pipeFd)) {
        const FErrno thisErrno;
        LOG(HAL, Error, L"failed to create pipe with errno: {0}",
            thisErrno );
        return false;
    }

    *pRead =  (FPipeHandle)((intptr_t)pipeFd[0]);
    *pWrite =  (FPipeHandle)((intptr_t)pipeFd[1]);

    Assert(*pRead);
    Assert(*pWrite);

    return true;
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformProcess::PeekPipe(FPipeHandle read) {
    Assert(read);

    int fd = PipeFD_(read);
    int bytesAvailable = 0;
    Verify(::ioctl(fd, FIONREAD, &bytesAvailable) == 0);

    return checked_cast<size_t>(bytesAvailable);
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformProcess::ReadPipe(FPipeHandle read, const FRawMemory& buffer) {
    Assert(read);

    int fd = PipeFD_(read);
    int bytesAvailable = 0;
    if (::ioctl(fd, FIONREAD, &bytesAvailable) == 0) {
        if (bytesAvailable) {
            int bytesRead = ::read(fd, buffer.data(), buffer.size());
            if (bytesRead > 0)
                return checked_cast<size_t>(bytesRead);
        }
    }
    else {
        const FErrno thisErrno;
        LOG(HAL, Fatal, L"ioctl(..., FIONREAD, ...) failed with errno: {0}",
            thisErrno );
    }

    return 0;
}
//----------------------------------------------------------------------------
size_t FLinuxPlatformProcess::WritePipe(FPipeHandle write, const FRawMemoryConst& buffer) {
    Assert(write);

    // If there is not a message or WritePipe is null
    if (buffer.empty())
        return 0;

    // Write to pipe
    int fd = PipeFD_(write);
    return ::write(fd, buffer.data(), buffer.SizeInBytes());
}
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::ClosePipe(FPipeHandle read, FPipeHandle write) {
    if (read)
        ::close(PipeFD_(read));
    if (write)
        ::close(PipeFD_(write));
}
//----------------------------------------------------------------------------
// Spawn process
//----------------------------------------------------------------------------
auto FLinuxPlatformProcess::CreateProcess(
    FProcessId* pPID,
    const wchar_t* executable,
    const wchar_t* parameters,
    const wchar_t* workingDir,
    bool detached, bool hidden, bool inheritHandles, bool noWindow,
    EProcessPriority priority,
    FPipeHandle hStdin/* = nullptr */,
    FPipeHandle hStderr/* = nullptr */,
    FPipeHandle hStdout/* = nullptr */) -> FProcessHandle {
    Assert(pPID);
    Assert(executable);

    Unused(workingDir);
    Unused(hidden);
    Unused(inheritHandles);
    Unused(noWindow);

    CLOG(workingDir != nullptr, HAL, Error,
        L"::posix_spawn() doesn't support working directory yet: {0}",
        MakeCStringView(workingDir) );

    // parse parameters
    VECTORINSITU(Process, TStaticString<>, 4) args;
    args.emplace_back(WCHAR_TO_UTF_8<>(executable));
    for (const wchar_t* p0 = parameters, *p1 = p0; *p1; ++p1) {
        if ((*p1 == L'"') & (p0 != p1)) {
            args.emplace_back(WCHAR_TO_UTF_8<>(MakeView(p0 + 1, p1)));
        }
        else if (*p1 == L' ') {
            if (p0 == p1)
                p0 = p1;
            else
                args.emplace_back(WCHAR_TO_UTF_8<>(MakeView(p0, p1)));
        }
    }

    // construct process command line
    VECTORINSITU(Process, char*, 8) argv;
    argv.reserve_AssumeEmpty(args.size() + 1);
    for (TStaticString<>& arg : args)
        argv.emplace_back(arg.Data);
    argv.emplace_back(nullptr); // must be null terminated

    short int spawnFlags = 0;
    ::posix_spawnattr_t spawnAttr;
    Verify(0 == ::posix_spawnattr_init(&spawnAttr));

    // unmask all signals and set realtime signals to defaut for children
    ::sigset_t emptySignalSet;
    Verify(0 == ::sigemptyset(&emptySignalSet));
    Verify(0 == ::posix_spawnattr_setsigmask(&spawnAttr, &emptySignalSet));
    spawnFlags |= POSIX_SPAWN_SETSIGMASK;

    ::sigset_t setToDefaultSignalSet;
    Verify(0 == ::sigemptyset(&setToDefaultSignalSet));
    for (int sigNum = SIGRTMIN; sigNum <= __SIGRTMAX; ++sigNum)
        Verify(0 == ::sigaddset(&setToDefaultSignalSet, sigNum));

    Verify(0 == ::posix_spawnattr_setsigdefault(&spawnAttr, &setToDefaultSignalSet));
    spawnFlags |= POSIX_SPAWN_SETSIGDEF;

    // make spawned process have its own unique group ip so we can kill the entire group
    // without killing the parent
    spawnFlags |= POSIX_SPAWN_SETPGROUP;

    ::pid_t child = -1;
    int posixSpawnErrno = -1;

    if (!!hStdin | !!hStderr | !!hStdout) {
        ::posix_spawn_file_actions_t fileActions;
        Verify(0 == ::posix_spawn_file_actions_init(&fileActions));

        if (hStdin)
            Verify(0 == ::posix_spawn_file_actions_adddup2(&fileActions, PipeFD_(hStdin), STDIN_FILENO));
        if (hStderr)
            Verify(0 == ::posix_spawn_file_actions_adddup2(&fileActions, PipeFD_(hStderr), STDERR_FILENO));
        if (hStdout)
            Verify(0 == ::posix_spawn_file_actions_adddup2(&fileActions, PipeFD_(hStdout), STDOUT_FILENO));

        Verify(0 == ::posix_spawnattr_setflags(&spawnAttr, spawnFlags));

        posixSpawnErrno = ::posix_spawn(&child, WCHAR_TO_UTF_8<>(executable), &fileActions, &spawnAttr, argv.data(), environ);

        Verify(0 == ::posix_spawn_file_actions_destroy(&fileActions));
    }
    else {
        // faster route using vfork()
        spawnFlags |= POSIX_SPAWN_USEVFORK;

        Verify(0 == ::posix_spawnattr_setflags(&spawnAttr, spawnFlags));

        posixSpawnErrno = ::posix_spawn(&child, WCHAR_TO_UTF_8<>(executable), nullptr, &spawnAttr, argv.data(), environ);
    }

    Verify(0 == ::posix_spawnattr_destroy(&spawnAttr));

    if (posixSpawnErrno != 0) {
        const FErrno thisErrno{ posixSpawnErrno };
        LOG(HAL, Error, L"posix_spawn({0}, {1}) failed with errno: {2}",
            MakeCStringView(executable),
            MakeCStringView(parameters),
            thisErrno );

        return FLinuxProcessHandle{};
    }

    FLinuxProcessHandle process{ TRACKING_NEW(Process, FLinuxProcessState)(child, detached) };

    if (pPID)
        *pPID = process.Pid();

    SetPriority(process, priority);

    return process;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::ExecDetachedProcess(
    const wchar_t* executable,
    const wchar_t* parameters,
    const wchar_t* workingDir) {
    Assert(executable);

    LOG(HAL, Verbose, L"execl(\"{0} {1}\") in {2}",
        MakeCStringView(executable), MakeCStringView(parameters), MakeCStringView(workingDir));

    if (0 == ::fork()) {

        ::exit(::execl(
            WCHAR_TO_UTF_8<>(workingDir),
            WCHAR_TO_UTF_8<>(executable),
            WCHAR_TO_UTF_8<>(parameters),
            (char*)0) );
    }

    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::ExecElevatedProcess(
    int* pReturnCode,
    const wchar_t* executable,
    const wchar_t* parameters,
    const wchar_t* workingDir ) {
    Assert(executable);

    ::pid_t child = ::fork();
    if (0 == child) {
        ::exit(::execl(
            WCHAR_TO_UTF_8<>(workingDir),
            WCHAR_TO_UTF_8<>(executable),
            WCHAR_TO_UTF_8<>(parameters),
            (char*)0) );
    }

    FLinuxProcessState process{ child, false };
    return process.ExitCode(pReturnCode);
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::OpenURL(const wchar_t* url) {
    XdgOpen_(url);
    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::OpenWithDefaultApp(const wchar_t* filename) {
    XdgOpen_(filename);
    return true;
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::EditWithDefaultApp(const wchar_t* filename) {
    XdgOpen_(filename);
    return true;
}
//----------------------------------------------------------------------------
// Semaphores
//----------------------------------------------------------------------------
auto FLinuxPlatformProcess::CreateSemaphore(const char* name, bool create, size_t maxLocks) -> FSemaphore {
    Assert(name);

    ::sem_t* sem;
    if (create)
        sem = ::sem_open(name, O_CREAT | O_EXCL, 0644, maxLocks);
    else
        sem = ::sem_open(name, O_EXCL);

    CLOG(nullptr == sem, HAL, Error,
        L"sem_open({0}) failed with errno: {1}",
        UTF_8_TO_WCHAR<>(name),
        FErrno{} );

    return sem;
}
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::LockSemaphore(FSemaphore semaphore) {
    Assert(semaphore);

    ::sem_t* sem = (::sem_t*)semaphore;
    int waitResult = ::sem_wait(sem);
    CLOG(waitResult != 0, HAL, Error, L"sem_wait() failed with errno: {0}",
        FErrno{} );
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::TryLockSemaphore(FSemaphore semaphore, u64 nanoSeconds) {
    Assert(semaphore);
    Unused(nanoSeconds);

    ::timespec tm;
    tm.tv_sec = 0;;
    tm.tv_nsec = nanoSeconds;

    ::sem_t* sem = (::sem_t*)semaphore;
    int waitResult = ::sem_timedwait(sem, &tm);

    CLOG(waitResult != 0, HAL, Error, L"sem_wait() failed with errno: {0}",
        FErrno{} );

    return (waitResult == 0);
}
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::UnlockSemaphore(FSemaphore semaphore) {
    Assert(semaphore);

    ::sem_t* sem = (::sem_t*)semaphore;
    if (::sem_post(sem) != 0)
        LOG(HAL, Error, L"sem_post() failed with errno: {0}",
            FErrno{} );
}
//----------------------------------------------------------------------------
bool FLinuxPlatformProcess::DestroySemaphore(FSemaphore semaphore) {
    Assert(semaphore);

    ::sem_t* sem = (::sem_t*)semaphore;
    if (::sem_destroy(sem) != 0) {
        LOG(HAL, Error, L"sem_destroy() failed with errno: {0}",
            FErrno{} );

        return false;
    }
    else {
        return true;
    }
}
//----------------------------------------------------------------------------
// Dynamic libraries (DLL)
//----------------------------------------------------------------------------
static void* LoadDynamicLibrary_(const wchar_t* name, bool attach) {
    Assert(name);

    int dlOpenMode = RTLD_LAZY;
    if (attach)
        dlOpenMode |= RTLD_NOLOAD | RTLD_NODELETE;

    void* handle = ::dlopen(WCHAR_TO_UTF_8<>(name), dlOpenMode | RTLD_LOCAL );

    CLOG(not handle, HAL, Error, L"dlopen({0}) failed: {1}",
        MakeCStringView(name),
        UTF_8_TO_WCHAR<>(::dlerror()) );

    return handle;
}
//----------------------------------------------------------------------------
static void UnloadDynamicLibrary_(void* lib) {
    Assert(lib);
    if (::dlclose(lib))
        LOG(HAL, Error, L"dlclose() failed: {0}",
            UTF_8_TO_WCHAR<>(::dlerror()) );
}
//----------------------------------------------------------------------------
auto FLinuxPlatformProcess::AttachToDynamicLibrary(const wchar_t* name) -> FDynamicLibraryHandle {
    return LoadDynamicLibrary_(name, true);
}
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::DetachFromDynamicLibrary(FDynamicLibraryHandle lib) {
    UnloadDynamicLibrary_(lib);
}
//----------------------------------------------------------------------------
auto FLinuxPlatformProcess::OpenDynamicLibrary(const wchar_t* name) -> FDynamicLibraryHandle {
    return LoadDynamicLibrary_(name, false);
}
//----------------------------------------------------------------------------
void FLinuxPlatformProcess::CloseDynamicLibrary(FDynamicLibraryHandle lib) {
    UnloadDynamicLibrary_(lib);
}
//----------------------------------------------------------------------------
FWString FLinuxPlatformProcess::DynamicLibraryFilename(FDynamicLibraryHandle lib) {
    char buf[PATH_MAX + 1];
    if (::dlinfo(lib, RTLD_DI_ORIGIN, &buf) == 0) {
        return ToWString(MakeCStringView(buf));
    }
    else {
        LOG(HAL, Error, L"dlinfo() failed: {0}",
            UTF_8_TO_WCHAR<>(::dlerror()) );
        return FWString();
    }
}
//----------------------------------------------------------------------------
void* FLinuxPlatformProcess::DynamicLibraryFunction(FDynamicLibraryHandle lib, const char* name) {
    Assert(lib);
    Assert(name);

    void* fnAddr = ::dlsym(lib, name);
    CLOG(!fnAddr, HAL, Error,
        L"dlsym({0}) failed: {1}",
        MakeCStringView(name),
        UTF_8_TO_WCHAR<>(::dlerror()) );

    return (void*)fnAddr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_LINUX
