#pragma once

#include "Core.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDiagnosticsStartup {
public:
    FDiagnosticsStartup(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t** argv) { Start(applicationHandle, nShowCmd, filename, argc, argv); }
    ~FDiagnosticsStartup() { Shutdown(); }

    static void Start(void *applicationHandle, int nShowCmd, const wchar_t* filename, size_t argc, const wchar_t** argv);
    static void Shutdown();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE