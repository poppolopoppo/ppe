#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DiagnosticsStartup {
public:
    DiagnosticsStartup(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv) { Start(applicationHandle, nShowCmd, argc, argv); }
    ~DiagnosticsStartup() { Shutdown(); }

    static void Start(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv);
    static void Shutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
