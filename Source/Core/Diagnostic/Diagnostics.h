#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDiagnosticsStartup {
public:
    FDiagnosticsStartup(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv) { Start(applicationHandle, nShowCmd, argc, argv); }
    ~FDiagnosticsStartup() { Shutdown(); }

    static void Start(void *applicationHandle, int nShowCmd, size_t argc, const wchar_t** argv);
    static void Shutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
