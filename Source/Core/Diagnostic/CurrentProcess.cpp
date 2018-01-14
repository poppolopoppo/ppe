#include "stdafx.h"

#include "CurrentProcess.h"

#include "Diagnostic/Logger.h"
#include "Misc/TargetPlatform.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCurrentProcess::FCurrentProcess(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t **argv)
:   _fileName(MakeStringView(filename, Meta::FForceInit{}))
,   _args(NewArray<FWString>(argc)), _exitCode(0), _appIcon(0)
,   _startedAt(FTimepoint::Now()) {

    for (size_t i = 0; i < argc; ++i) {
        Assert(argv[i]);
        _args[i] = MakeStringView(argv[i], Meta::FForceInit{});
    }

    size_t dirSep = _fileName.size();
    for (; dirSep > 0 && _fileName[dirSep - 1] != L'/' && _fileName[dirSep - 1] != L'\\'; --dirSep);
    _directory.assign(_fileName.begin(), _fileName.begin() + dirSep);

    _applicationHandle = applicationHandle;
    _nShowCmd = nShowCmd;
#ifndef FINAL_RELEASE
    _startedWithDebugger = FPlatformMisc::IsDebuggerAttached();
#else
    _startedWithDebugger = false;
#endif

#ifndef FINAL_RELEASE
    if (_args.end() != _args.FindIf([](const FWString& arg) { return EqualsI(arg, L"-WaitForDebugger"); })) {
        _startedWithDebugger = false; // some parts of the code won't detect that the debugger is attached
        volatile bool bTurnThisOffWhenDebuggerIsAttached = (!FPlatformMisc::IsDebuggerAttached());
        volatile size_t loopCount = 0;
        while (bTurnThisOffWhenDebuggerIsAttached) {
            LOG(Warning, L"[Process] Waiting for debugger to be attached");
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // wait for debugger to be attached
            loopCount++;
        }
    }
#endif

#ifdef USE_DEBUG_LOGGER
    LOG(Info, L"[Process] Started '{0}' with {1} parameters.", _fileName, _args.size());
    LOG(Info, L"[Process] Directory = '{0}'.", _directory);
    LOG(Info, L"[Process] Build configuration = " WIDESTRING(STRINGIZE(BUILDCONFIG)));
    LOG(Info, L"[Process] Compiled at = " WIDESTRING(__DATE__) L"  " WIDESTRING(__TIME__));
    LOG(Info, L"[Process] Application Handle = '{0}', nShowCmd = '{1}'.", _applicationHandle, _nShowCmd);
    forrange(i, 0, _args.size())
        LOG(Info, L"- [{0:2}] '{1}'", i, _args[i]);
#endif
}
//----------------------------------------------------------------------------
FCurrentProcess::~FCurrentProcess() {
    LOG(Info, L"[Process] Exit with code = {0}.", _exitCode.load());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
