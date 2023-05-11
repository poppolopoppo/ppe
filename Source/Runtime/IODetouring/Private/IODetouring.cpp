// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "IODetouring.h"

#include "IODetouringFiles.h"
#include "IODetouringHooks.h"
#include "IODetouringTblog.h"

#include "detours-external.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Installing/uninstalling the hooks when DLL is loaded/unloaded:
//----------------------------------------------------------------------------
static BOOL OnProcessAttach_(FIODetouringHooks& H, HMODULE hDll);
static BOOL OnProcessDettach_(FIODetouringHooks& H, HMODULE hDll);
//----------------------------------------------------------------------------
static BOOL OnThreadAttach_(FIODetouringHooks& H, HMODULE hDll);
static BOOL OnThreadDetach_(FIODetouringHooks& H, HMODULE hDll);
//----------------------------------------------------------------------------
extern "C" BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, PVOID lpReserved) {
    PPE::Unused(hModule);
    PPE::Unused(lpReserved);

    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    FIODetouringHooks& H = FIODetouringHooks::Get();

    switch (dwReason) {
    case DLL_PROCESS_ATTACH:
        DetourRestoreAfterWith();
        H.Real_EntryPoint =  (int (WINAPI *)(VOID))DetourGetEntryPoint(NULL);
        return OnProcessAttach_(H, hModule);

    case DLL_PROCESS_DETACH:
        return OnProcessDettach_(H, hModule);

    case DLL_THREAD_ATTACH:
        return OnThreadAttach_(H, hModule);

    case DLL_THREAD_DETACH:
        return OnThreadDetach_(H, hModule);
    }

    return TRUE;
}
//----------------------------------------------------------------------------
static BOOL OnProcessAttach_(FIODetouringHooks& H, HMODULE hDll) {
    FIODetouringTblog::Create();

    H.hInstance = hDll;
    H.hKernel32 = NULL;
    H.bLog = FALSE;

    PBYTE xCreate = (PBYTE)DetourCodeFromPointer((PVOID)H.Real_CreateProcessW, NULL);
    FIODetouringTblog::PPayload pPayload = NULL;

    for (HMODULE hMod = NULL; (hMod = DetourEnumerateModules(hMod)) != NULL;) {
        ULONG cbData;
        PVOID pvData = DetourFindPayload(hMod, GIODetouringGuid, &cbData);

        if (pvData != NULL && pPayload == NULL) {
            pPayload = (FIODetouringTblog::PPayload)pvData;
        }

        ULONG cbMod = DetourGetModuleSize(hMod);

        if (((PBYTE)hMod) < xCreate && ((PBYTE)hMod + cbMod) > xCreate) {
            H.hKernel32 = hMod;
        }
    }

    if (pPayload == NULL || H.hKernel32 == NULL) {
        return FALSE;
    }

    FIODetouringFiles::Create(pPayload->wzStdin, pPayload->wzStdout, pPayload->wzStderr);

    FIODetouringTblog& Tblog = FIODetouringTblog::Get();
    Tblog.CopyPayload(pPayload);

    GetModuleFileNameA(H.hInstance, H.szDllPath, ARRAYSIZE(H.szDllPath));

    // Find hidden functions.
    H.Real_PrivCopyFileExW = (BOOL (WINAPI *)(LPCWSTR, LPCWSTR, LPPROGRESS_ROUTINE, LPVOID, LPBOOL, DWORD))
        GetProcAddress(H.hKernel32, "PrivCopyFileExW");
    if (H.Real_PrivCopyFileExW == NULL) {
        PPE_DEBUG_BREAK();
    }

    LONG const error = H.AttachDetours();
    if (error != NO_ERROR) {
        PPE_DEBUG_BREAK();
        Tblog.Printf("Error attaching detours: %d\n", error);
    }

    OnThreadAttach_(H, hDll);

    H.bLog = TRUE;
    return TRUE;
}
//----------------------------------------------------------------------------
static BOOL OnProcessDettach_(FIODetouringHooks& H, HMODULE hDll) {
    OnThreadDetach_(H, hDll);
    H.bLog = FALSE;

    FIODetouringFiles::Get().Dump();
    FIODetouringFiles::Destroy();

    FIODetouringTblog& Tblog = FIODetouringTblog::Get();

    LONG const error = H.DetachDetours();
    if (error != NO_ERROR) {
        PPE_DEBUG_BREAK();
        Tblog.Printf("Error detaching detours: %d\n", error);
    }

    Tblog.Close();
    return TRUE;
}
//----------------------------------------------------------------------------
static BOOL OnThreadAttach_(FIODetouringHooks& H, HMODULE hDll) {
    PPE::Unused(H, hDll);
    return TRUE;
}
//----------------------------------------------------------------------------
static BOOL OnThreadDetach_(FIODetouringHooks& H, HMODULE hDll) {
    PPE::Unused(H, hDll);
    return TRUE;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
