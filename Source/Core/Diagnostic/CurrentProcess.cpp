#include "stdafx.h"

#include "CurrentProcess.h"

#include "Logger.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
CurrentProcessData::CurrentProcessData(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t **argv)
:   _args(NewArray<WString>(argc - 1)), _exitCode(0), _appIcon(0) {
    Assert(argc); // current process name at least

    for (size_t i = 1; i < argc; ++i) {
        Assert(argv[i]);
        _args[i] = argv[i];
    }

    _fileName = argv[0];

    size_t dirSep = _fileName.size();
    for (; dirSep > 0 && _fileName[dirSep - 1] != L'/' && _fileName[dirSep - 1] != L'\\'; --dirSep);
    _directory = WString(_fileName.begin(), _fileName.begin() + dirSep);

    _applicationHandle = applicationHandle;
    _nShowCmd = nShowCmd;

#ifdef USE_LOGGER
    LOG(Information, L"[Process] Started '{0}' with {1} parameters.", _fileName, _args.size());
    LOG(Information, L"[Process] Directory = '{0}'.", _directory);
    LOG(Information, L"[Process] Application Handle = '{0}', nShowCmd = '{1}'.", _applicationHandle, _nShowCmd);
    for (size_t i = 0; i < _args.size(); ++i)
        LOG(Information, L"- [{0:2}] '{1}'", i, _args[i]);
#endif
}
//----------------------------------------------------------------------------
CurrentProcessData::~CurrentProcessData() {
    LOG(Information, L"[Process] Exit with code = {0}.", _exitCode);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
