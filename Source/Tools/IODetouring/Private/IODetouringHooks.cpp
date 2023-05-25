#include "IODetouringHooks.h"

#include "IODetouringDebug.h"
#include "IODetouringFiles.h"
#include "IODetouringTblog.h"

#include "detours-external.h"

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Global struct with original function pointers:
//----------------------------------------------------------------------------
static FIODetouringHooks GIODetouringHooks(FIODetouringHooks::force_init_t{});
//----------------------------------------------------------------------------
FIODetouringHooks::FIODetouringHooks(force_init_t) NOEXCEPT {
    InitializeRealFunctions();
}
//----------------------------------------------------------------------------
FIODetouringHooks& FIODetouringHooks::Get() {
    return GIODetouringHooks;
}
//----------------------------------------------------------------------------
// Detours:
//----------------------------------------------------------------------------
void FIODetouringHooks::InitializeRealFunctions() {
    Real_EntryPoint = NULL ;
    Real_ExitProcess = ::ExitProcess ;
    Real_CreateDirectoryW = ::CreateDirectoryW ;
    Real_CreateDirectoryExW = ::CreateDirectoryExW ;
    Real_CreateFileW = ::CreateFileW ;
    Real_CreateFileMappingW = ::CreateFileMappingW ;
    Real_CreatePipe = ::CreatePipe ;
    Real_CloseHandle = ::CloseHandle ;
    Real_DuplicateHandle = ::DuplicateHandle ;
    Real_CreateProcessW = ::CreateProcessW ;
    Real_CreateProcessA = ::CreateProcessA ;
    Real_DeleteFileW = ::DeleteFileW ;
    Real_DeviceIoControl = ::DeviceIoControl ;
    Real_GetFileAttributesW = ::GetFileAttributesW ;
    Real_MoveFileWithProgressW = ::MoveFileWithProgressW ;
    Real_MoveFileA = ::MoveFileA ;
    Real_MoveFileW = ::MoveFileW ;
    Real_MoveFileExA = ::MoveFileExA ;
    Real_MoveFileExW = ::MoveFileExW ;
    Real_CopyFileExA = ::CopyFileExA ;
    Real_CopyFileExW = ::CopyFileExW ;
    Real_PrivCopyFileExW = NULL ;
    Real_CreateHardLinkA = ::CreateHardLinkA ;
    Real_CreateHardLinkW = ::CreateHardLinkW ;
    Real_SetStdHandle = ::SetStdHandle ;
    Real_LoadLibraryA = ::LoadLibraryA ;
    Real_LoadLibraryW = ::LoadLibraryW ;
    Real_LoadLibraryExA = ::LoadLibraryExA ;
    Real_LoadLibraryExW = ::LoadLibraryExW ;
    Real_SetFilePointer = ::SetFilePointer ;
    Real_SetFilePointerEx = ::SetFilePointerEx ;
    Real_ReadFile = ::ReadFile ;
    Real_ReadFileEx = ::ReadFileEx ;
    Real_WriteFile = ::WriteFile ;
    Real_WriteFileEx = ::WriteFileEx ;
    Real_WriteConsoleA = ::WriteConsoleA ;
    Real_WriteConsoleW = ::WriteConsoleW ;
    Real_ExpandEnvironmentStringsA = ::ExpandEnvironmentStringsA ;
    Real_ExpandEnvironmentStringsW = ::ExpandEnvironmentStringsW ;
    Real_GetEnvironmentVariableA = ::GetEnvironmentVariableA ;
    Real_GetEnvironmentVariableW = ::GetEnvironmentVariableW ;
    Real_wgetenv = NULL; // Resolved in Real_EntryPoint()
    Real_getenv = NULL; // Resolved in Real_EntryPoint()
    Real_getenv_s = NULL; // Resolved in Real_EntryPoint()
    Real_wgetenv_s = NULL; // Resolved in Real_EntryPoint()
    Real_dupenv_s = NULL; // Resolved in Real_EntryPoint()
    Real_wdupenv_s = NULL; // Resolved in Real_EntryPoint()
}
//----------------------------------------------------------------------------
static DWORD StringCompare_(PCSTR pszA, PCSTR pszB) {
    for (;;) {
        CHAR cA = *pszA++;
        CHAR cB = *pszB++;

        if (cA >= 'A' && cA <= 'Z') {
            cA += ('a' - 'A');
        }
        if (cB >= 'A' && cB <= 'Z') {
            cB += ('a' - 'A');
        }

        if (cA == 0 && cB == 0) {
            return 0;
        }
        if (cA != cB) {
            return cA - cB;
        }
    }
}
//----------------------------------------------------------------------------
static BOOL WINAPI ImportFileCallback_(PVOID pContext, HMODULE hFile, PCSTR pszFile) {
    FIODetouringHooks& H = *reinterpret_cast<FIODetouringHooks*>(pContext);

    static const PCSTR s_rpszMsvcrNames[] = {
        "msvcr80.dll",
        "msvcr80d.dll",
        "msvcr71.dll",
        "msvcr71d.dll",
        "msvcr70.dll",
        "msvcr70d.dll",
        NULL
    };

    if (pszFile != NULL) {
        for (int i = 0; s_rpszMsvcrNames[i]; i++) {
            if (StringCompare_(pszFile, s_rpszMsvcrNames[i]) == 0) {
                H.hMsvcr = hFile;
                H.pszMsvcr = s_rpszMsvcrNames[i];
                return FALSE;
            }
        }
    }
    return TRUE;
}
//----------------------------------------------------------------------------
BOOL FindMsvcrModule_(FIODetouringHooks& H) {
    DetourEnumerateImports(NULL, &H, ImportFileCallback_, NULL);

    if (H.hMsvcr != NULL) {
        return TRUE;
    }

    return FALSE;
}
//----------------------------------------------------------------------------
BOOL FindProcInMsvcr_(FIODetouringHooks& H, PVOID * ppvCode, PCSTR pwzFunc) {
    if (PVOID const pv = GetProcAddress(H.hMsvcr, pwzFunc)) {
        *ppvCode = pv;
        return TRUE;
    }
    else {
        *ppvCode = NULL;
        return FALSE;
    }
}
//----------------------------------------------------------------------------
static LONG WINAPI DetourAttachIf_(PVOID *ppPointer, PVOID pDetour) {
    if (*ppPointer == NULL) {
        FIODetouringTblog::Get().Printf("DetourAttachIf failed: %p -->\n", pDetour);
        return NO_ERROR;
    }

    PDETOUR_TRAMPOLINE pRealTrampoline;
    PVOID pRealTarget;
    PVOID pRealDetour;
    return DetourAttachEx(ppPointer, pDetour, &pRealTrampoline, &pRealTarget, &pRealDetour);
}
//----------------------------------------------------------------------------
LONG FIODetouringHooks::AttachDetours() {
    IODETOURING_DEBUGPRINTF("attach IO detours\n");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&Real_EntryPoint, Hook_EntryPoint);
    DetourAttach(&Real_ExitProcess, Hook_ExitProcess);
    DetourAttach(&Real_CreateDirectoryA, Hook_CreateDirectoryA);
    DetourAttach(&Real_CreateDirectoryW, Hook_CreateDirectoryW);
    DetourAttach(&Real_CreateDirectoryExA, Hook_CreateDirectoryExA);
    DetourAttach(&Real_CreateDirectoryExW, Hook_CreateDirectoryExW);
    DetourAttach(&Real_CreateFileA, Hook_CreateFileA);
    DetourAttach(&Real_CreateFileW, Hook_CreateFileW);
    DetourAttach(&Real_CreateFileMappingA, Hook_CreateFileMappingA);
    DetourAttach(&Real_CreateFileMappingW, Hook_CreateFileMappingW);
    DetourAttach(&Real_CreateFileMappingNumaA, Hook_CreateFileMappingNumaA);
    DetourAttach(&Real_CreateFileMappingNumaW, Hook_CreateFileMappingNumaW);
    DetourAttach(&Real_CreatePipe, Hook_CreatePipe);
    DetourAttach(&Real_CloseHandle, Hook_CloseHandle);
    DetourAttach(&Real_DuplicateHandle, Hook_DuplicateHandle);
    DetourAttach(&Real_CreateProcessW, Hook_CreateProcessW);
    DetourAttach(&Real_CreateProcessA, Hook_CreateProcessA);
    DetourAttach(&Real_DeleteFileA, Hook_DeleteFileA);
    DetourAttach(&Real_DeleteFileW, Hook_DeleteFileW);
    DetourAttach(&Real_DeviceIoControl, Hook_DeviceIoControl);
    DetourAttach(&Real_GetFileAttributesW, Hook_GetFileAttributesW);
    DetourAttach(&Real_MapViewOfFile, Hook_MapViewOfFile);
    DetourAttach(&Real_MapViewOfFileEx, Hook_MapViewOfFileEx);
    DetourAttach(&Real_MoveFileWithProgressA, Hook_MoveFileWithProgressA);
    DetourAttach(&Real_MoveFileWithProgressW, Hook_MoveFileWithProgressW);
    DetourAttach(&Real_MoveFileA, Hook_MoveFileA);
    DetourAttach(&Real_MoveFileW, Hook_MoveFileW);
    DetourAttach(&Real_MoveFileExA, Hook_MoveFileExA);
    DetourAttach(&Real_MoveFileExW, Hook_MoveFileExW);
    DetourAttach(&Real_CopyFileExA, Hook_CopyFileExA);
    DetourAttach(&Real_CopyFileExW, Hook_CopyFileExW);
    DetourAttach(&Real_CreateHardLinkA, Hook_CreateHardLinkA);
    DetourAttach(&Real_CreateHardLinkW, Hook_CreateHardLinkW);
    DetourAttach(&Real_SetStdHandle, Hook_SetStdHandle);
    DetourAttach(&Real_LoadLibraryA, Hook_LoadLibraryA);
    DetourAttach(&Real_LoadLibraryW, Hook_LoadLibraryW);
    DetourAttach(&Real_LoadLibraryExA, Hook_LoadLibraryExA);
    DetourAttach(&Real_LoadLibraryExW, Hook_LoadLibraryExW);
    DetourAttach(&Real_SetFilePointer, Hook_SetFilePointer);
    DetourAttach(&Real_SetFilePointerEx, Hook_SetFilePointerEx);
    DetourAttach(&Real_ReadFile, Hook_ReadFile);
    DetourAttach(&Real_ReadFileEx, Hook_ReadFileEx);
    DetourAttach(&Real_WriteFile, Hook_WriteFile);
    DetourAttach(&Real_WriteFileEx, Hook_WriteFileEx);
    DetourAttach(&Real_WriteConsoleA, Hook_WriteConsoleA);
    DetourAttach(&Real_WriteConsoleW, Hook_WriteConsoleW);
    DetourAttach(&Real_ExpandEnvironmentStringsA, Hook_ExpandEnvironmentStringsA);
    DetourAttach(&Real_ExpandEnvironmentStringsW, Hook_ExpandEnvironmentStringsW);
    DetourAttach(&Real_GetEnvironmentVariableA, Hook_GetEnvironmentVariableA);
    DetourAttach(&Real_GetEnvironmentVariableW, Hook_GetEnvironmentVariableW);

    if (!!Real_PrivCopyFileExA)
        DetourAttach(&Real_PrivCopyFileExA, Hook_PrivCopyFileExA);
    if (!!Real_PrivCopyFileExW)
        DetourAttach(&Real_PrivCopyFileExW, Hook_PrivCopyFileExW);

    return DetourTransactionCommit();
}
//----------------------------------------------------------------------------
LONG FIODetouringHooks::DetachDetours() {
    IODETOURING_DEBUGPRINTF("detach IO detours\n");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourDetach(&Real_EntryPoint, Hook_EntryPoint);
    DetourDetach(&Real_ExitProcess, Hook_ExitProcess);
    DetourDetach(&Real_CreateDirectoryA, Hook_CreateDirectoryA);
    DetourDetach(&Real_CreateDirectoryW, Hook_CreateDirectoryW);
    DetourDetach(&Real_CreateDirectoryExA, Hook_CreateDirectoryExA);
    DetourDetach(&Real_CreateDirectoryExW, Hook_CreateDirectoryExW);
    DetourDetach(&Real_CreateFileA, Hook_CreateFileA);
    DetourDetach(&Real_CreateFileW, Hook_CreateFileW);
    DetourDetach(&Real_CreateFileMappingA, Hook_CreateFileMappingA);
    DetourDetach(&Real_CreateFileMappingW, Hook_CreateFileMappingW);
    DetourDetach(&Real_CreateFileMappingNumaA, Hook_CreateFileMappingNumaA);
    DetourDetach(&Real_CreateFileMappingNumaW, Hook_CreateFileMappingNumaW);
    DetourDetach(&Real_CreatePipe, Hook_CreatePipe);
    DetourDetach(&Real_CloseHandle, Hook_CloseHandle);
    DetourDetach(&Real_DuplicateHandle, Hook_DuplicateHandle);
    DetourDetach(&Real_CreateProcessW, Hook_CreateProcessW);
    DetourDetach(&Real_CreateProcessA, Hook_CreateProcessA);
    DetourDetach(&Real_DeleteFileA, Hook_DeleteFileA);
    DetourDetach(&Real_DeleteFileW, Hook_DeleteFileW);
    DetourDetach(&Real_DeviceIoControl, Hook_DeviceIoControl);
    DetourDetach(&Real_GetFileAttributesW, Hook_GetFileAttributesW);
    DetourDetach(&Real_MapViewOfFile, Hook_MapViewOfFile);
    DetourDetach(&Real_MapViewOfFileEx, Hook_MapViewOfFileEx);
    DetourDetach(&Real_MoveFileWithProgressA, Hook_MoveFileWithProgressA);
    DetourDetach(&Real_MoveFileWithProgressW, Hook_MoveFileWithProgressW);
    DetourDetach(&Real_MoveFileA, Hook_MoveFileA);
    DetourDetach(&Real_MoveFileW, Hook_MoveFileW);
    DetourDetach(&Real_MoveFileExA, Hook_MoveFileExA);
    DetourDetach(&Real_MoveFileExW, Hook_MoveFileExW);
    DetourDetach(&Real_CopyFileExA, Hook_CopyFileExA);
    DetourDetach(&Real_CopyFileExW, Hook_CopyFileExW);
    DetourDetach(&Real_CreateHardLinkA, Hook_CreateHardLinkA);
    DetourDetach(&Real_CreateHardLinkW, Hook_CreateHardLinkW);
    DetourDetach(&Real_SetStdHandle, Hook_SetStdHandle);
    DetourDetach(&Real_LoadLibraryA, Hook_LoadLibraryA);
    DetourDetach(&Real_LoadLibraryW, Hook_LoadLibraryW);
    DetourDetach(&Real_LoadLibraryExA, Hook_LoadLibraryExA);
    DetourDetach(&Real_LoadLibraryExW, Hook_LoadLibraryExW);
    DetourDetach(&Real_SetFilePointer, Hook_SetFilePointer);
    DetourDetach(&Real_SetFilePointerEx, Hook_SetFilePointerEx);
    DetourDetach(&Real_ReadFile, Hook_ReadFile);
    DetourDetach(&Real_ReadFileEx, Hook_ReadFileEx);
    DetourDetach(&Real_WriteFile, Hook_WriteFile);
    DetourDetach(&Real_WriteFileEx, Hook_WriteFileEx);
    DetourDetach(&Real_WriteConsoleA, Hook_WriteConsoleA);
    DetourDetach(&Real_WriteConsoleW, Hook_WriteConsoleW);
    DetourDetach(&Real_ExpandEnvironmentStringsA, Hook_ExpandEnvironmentStringsA);
    DetourDetach(&Real_ExpandEnvironmentStringsW, Hook_ExpandEnvironmentStringsW);
    DetourDetach(&Real_GetEnvironmentVariableA, Hook_GetEnvironmentVariableA);
    DetourDetach(&Real_GetEnvironmentVariableW, Hook_GetEnvironmentVariableW);

    if (!!Real_PrivCopyFileExA)
        DetourDetach(&Real_PrivCopyFileExA, Hook_PrivCopyFileExA);
    if (!!Real_PrivCopyFileExW)
        DetourDetach(&Real_PrivCopyFileExW, Hook_PrivCopyFileExW);

    if (Real_getenv)    DetourDetach(&(PVOID&)Real_getenv, Hook_getenv);
    if (Real_getenv_s)  DetourDetach(&(PVOID&)Real_getenv_s, Hook_getenv_s);
    if (Real_wgetenv)   DetourDetach(&(PVOID&)Real_wgetenv, Hook_wgetenv);
    if (Real_wgetenv_s) DetourDetach(&(PVOID&)Real_wgetenv, Hook_wgetenv_s);
    if (Real_dupenv_s)  DetourDetach(&(PVOID&)Real_dupenv_s, Hook_dupenv_s);
    if (Real_wdupenv_s) DetourDetach(&(PVOID&)Real_wdupenv_s, Hook_wdupenv_s);

    return DetourTransactionCommit();
}
//----------------------------------------------------------------------------
// Hooks:
//----------------------------------------------------------------------------
int WINAPI FIODetouringHooks::Hook_EntryPoint(VOID) {
    FIODetouringTblog::Get().Open();

    FIODetouringHooks& H = FIODetouringHooks::Get();
    if (FindMsvcrModule_(H)) {
        FindProcInMsvcr_(H, &(PVOID&)H.Real_getenv, "getenv");
        FindProcInMsvcr_(H, &(PVOID&)H.Real_wgetenv, "_wgetenv");
        FindProcInMsvcr_(H, &(PVOID&)H.Real_getenv_s, "getenv_s");
        FindProcInMsvcr_(H, &(PVOID&)H.Real_wgetenv_s, "_wgetenv_s");
        FindProcInMsvcr_(H, &(PVOID&)H.Real_dupenv_s, "_dupenv_s");
        FindProcInMsvcr_(H, &(PVOID&)H.Real_wdupenv_s, "_wdupenv_s");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttachIf_(&(PVOID&)H.Real_getenv, &Hook_getenv);
        DetourAttachIf_(&(PVOID&)H.Real_getenv_s, &Hook_getenv_s);
        DetourAttachIf_(&(PVOID&)H.Real_wgetenv, &Hook_wgetenv);
        DetourAttachIf_(&(PVOID&)H.Real_wgetenv, &Hook_wgetenv_s);
        DetourAttachIf_(&(PVOID&)H.Real_dupenv_s, &Hook_dupenv_s);
        DetourAttachIf_(&(PVOID&)H.Real_wdupenv_s, &Hook_wdupenv_s);

        DetourTransactionCommit();
    }

    return H.Real_EntryPoint();
}
//----------------------------------------------------------------------------
VOID WINAPI FIODetouringHooks::Hook_ExitProcess(UINT uExitCode) {
    IODETOURING_DEBUGPRINTF("call Hook_ExitProcess\n");
    return Get().Real_ExitProcess(uExitCode);
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateDirectoryA\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_CreateDirectoryA(lpPathName, lpSecurityAttributes);
    }
    __finally {
        if (rv) {
            if (FIODetouringFiles::PFileInfo const pInfo = FIODetouringFiles::Get().FindPartial(lpPathName))
                pInfo->bDirectory = TRUE;
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateDirectoryW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_CreateDirectoryW(lpPathName, lpSecurityAttributes);
    } __finally {
        if (rv) {
            if (FIODetouringFiles::PFileInfo const pInfo = FIODetouringFiles::Get().FindPartial(lpPathName))
                pInfo->bDirectory = TRUE;
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CreateDirectoryExA(LPCSTR lpTemplateDirectory, LPCSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateDirectoryExA\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_CreateDirectoryExA(lpTemplateDirectory, lpNewDirectory, lpSecurityAttributes);
    }
    __finally {
        if (rv) {
            if (FIODetouringFiles::PFileInfo const pInfo = FIODetouringFiles::Get().FindPartial(lpNewDirectory))
                pInfo->bDirectory = TRUE;
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CreateDirectoryExW(LPCWSTR lpTemplateDirectory, LPCWSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateDirectoryExW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_CreateDirectoryExW(lpTemplateDirectory, lpNewDirectory, lpSecurityAttributes);
    } __finally {
        if (rv) {
            if (FIODetouringFiles::PFileInfo const pInfo = FIODetouringFiles::Get().FindPartial(lpNewDirectory))
                pInfo->bDirectory = TRUE;
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
HANDLE WINAPI FIODetouringHooks::Hook_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateFileA\n");
    HANDLE rv = 0;
    __try {
        rv = Get().Real_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    __finally {
        if (dwDesiredAccess != 0 && rv != NULL && rv != INVALID_HANDLE_VALUE) {

            FIODetouringFiles& files = FIODetouringFiles::Get();
            FIODetouringFiles::PFileInfo const pInfo = files.FindPartial(lpFileName);

            // FILE_FLAG_WRITE_THROUGH              0x80000000
            // FILE_FLAG_OVERLAPPED                 0x40000000
            // FILE_FLAG_NO_BUFFERING               0x20000000
            // FILE_FLAG_RANDOM_ACCESS              0x10000000
            // FILE_FLAG_SEQUENTIAL_SCAN            0x08000000
            // FILE_FLAG_DELETE_ON_CLOSE            0x04000000
            // FILE_FLAG_BACKUP_SEMANTICS           0x02000000
            // FILE_FLAG_POSIX_SEMANTICS            0x01000000
            // FILE_FLAG_OPEN_REPARSE_POINT         0x00200000
            // FILE_FLAG_OPEN_NO_RECALL             0x00100000
            // FILE_FLAG_FIRST_PIPE_INSTANCE        0x00080000
            // FILE_ATTRIBUTE_ENCRYPTED             0x00004000
            // FILE_ATTRIBUTE_NOT_CONTENT_INDEXED   0x00002000
            // FILE_ATTRIBUTE_OFFLINE               0x00001000
            // FILE_ATTRIBUTE_COMPRESSED            0x00000800
            // FILE_ATTRIBUTE_REPARSE_POINT         0x00000400
            // FILE_ATTRIBUTE_SPARSE_FILE           0x00000200
            // FILE_ATTRIBUTE_TEMPORARY             0x00000100
            // FILE_ATTRIBUTE_NORMAL                0x00000080
            // FILE_ATTRIBUTE_DEVICE                0x00000040
            // FILE_ATTRIBUTE_ARCHIVE               0x00000020
            // FILE_ATTRIBUTE_DIRECTORY             0x00000010
            // FILE_ATTRIBUTE_SYSTEM                0x00000004
            // FILE_ATTRIBUTE_HIDDEN                0x00000002
            // FILE_ATTRIBUTE_READONLY              0x00000001

            // CREATE_NEW          1
            // CREATE_ALWAYS       2
            // OPEN_EXISTING       3
            // OPEN_ALWAYS         4
            // TRUNCATE_EXISTING   5

            if (dwCreationDisposition == CREATE_NEW ||
                dwCreationDisposition == CREATE_ALWAYS ||
                dwCreationDisposition == TRUNCATE_EXISTING) {

                if (!pInfo->bRead)
                    pInfo->bCantRead = TRUE;
            }
            else if (dwCreationDisposition == OPEN_EXISTING) {
            }
            else if (dwCreationDisposition == OPEN_ALWAYS) {
                // pInfo->bAppend = TRUE;    // !!!
            }

            if ((dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE) == FILE_FLAG_DELETE_ON_CLOSE)
                pInfo->bCleanup = TRUE;
            if ((dwFlagsAndAttributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)
                pInfo->bCantWrite = TRUE;
            if ((dwFlagsAndAttributes & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
                pInfo->bSystemPath = TRUE;
            if ((dwFlagsAndAttributes & FILE_ATTRIBUTE_TEMPORARY) == FILE_ATTRIBUTE_TEMPORARY)
                pInfo->bTemporaryFile = TRUE;

            files.Remember(rv, pInfo);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
HANDLE WINAPI FIODetouringHooks::Hook_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateFileW\n");
    HANDLE rv = 0;
    __try {
        rv = Get().Real_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    } __finally {
        if (dwDesiredAccess != 0 && rv != NULL && rv != INVALID_HANDLE_VALUE) {

            FIODetouringFiles& files = FIODetouringFiles::Get();
            FIODetouringFiles::PFileInfo const pInfo = files.FindPartial(lpFileName);

            // FILE_FLAG_WRITE_THROUGH              0x80000000
            // FILE_FLAG_OVERLAPPED                 0x40000000
            // FILE_FLAG_NO_BUFFERING               0x20000000
            // FILE_FLAG_RANDOM_ACCESS              0x10000000
            // FILE_FLAG_SEQUENTIAL_SCAN            0x08000000
            // FILE_FLAG_DELETE_ON_CLOSE            0x04000000
            // FILE_FLAG_BACKUP_SEMANTICS           0x02000000
            // FILE_FLAG_POSIX_SEMANTICS            0x01000000
            // FILE_FLAG_OPEN_REPARSE_POINT         0x00200000
            // FILE_FLAG_OPEN_NO_RECALL             0x00100000
            // FILE_FLAG_FIRST_PIPE_INSTANCE        0x00080000
            // FILE_ATTRIBUTE_ENCRYPTED             0x00004000
            // FILE_ATTRIBUTE_NOT_CONTENT_INDEXED   0x00002000
            // FILE_ATTRIBUTE_OFFLINE               0x00001000
            // FILE_ATTRIBUTE_COMPRESSED            0x00000800
            // FILE_ATTRIBUTE_REPARSE_POINT         0x00000400
            // FILE_ATTRIBUTE_SPARSE_FILE           0x00000200
            // FILE_ATTRIBUTE_TEMPORARY             0x00000100
            // FILE_ATTRIBUTE_NORMAL                0x00000080
            // FILE_ATTRIBUTE_DEVICE                0x00000040
            // FILE_ATTRIBUTE_ARCHIVE               0x00000020
            // FILE_ATTRIBUTE_DIRECTORY             0x00000010
            // FILE_ATTRIBUTE_SYSTEM                0x00000004
            // FILE_ATTRIBUTE_HIDDEN                0x00000002
            // FILE_ATTRIBUTE_READONLY              0x00000001

            // CREATE_NEW          1
            // CREATE_ALWAYS       2
            // OPEN_EXISTING       3
            // OPEN_ALWAYS         4
            // TRUNCATE_EXISTING   5

            if (dwCreationDisposition == CREATE_NEW ||
                dwCreationDisposition == CREATE_ALWAYS ||
                dwCreationDisposition == TRUNCATE_EXISTING) {

                if (!pInfo->bRead)
                    pInfo->bCantRead = TRUE;
            }
            else if (dwCreationDisposition == OPEN_EXISTING) {
            }
            else if (dwCreationDisposition == OPEN_ALWAYS) {
                // pInfo->bAppend = TRUE;    // !!!
            }

            if ((dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE) == FILE_FLAG_DELETE_ON_CLOSE)
                pInfo->bCleanup = TRUE;
            if ((dwFlagsAndAttributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)
                pInfo->bCantWrite = TRUE;
            if ((dwFlagsAndAttributes & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
                pInfo->bSystemPath = TRUE;
            if ((dwFlagsAndAttributes & FILE_ATTRIBUTE_TEMPORARY) == FILE_ATTRIBUTE_TEMPORARY)
                pInfo->bTemporaryFile = TRUE;

            files.Remember(rv, pInfo);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
HANDLE WINAPI FIODetouringHooks::Hook_CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateFileMappingA\n");
    HANDLE rv = 0;
    __try {
        rv = Get().Real_CreateFileMappingA(hFile, lpAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
    }
    __finally {
        if (rv != NULL && rv != INVALID_HANDLE_VALUE) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            if (FIODetouringFiles::PFileInfo const pInfo = files.RecallFile(hFile)) {
                switch (flProtect) {
                case PAGE_READONLY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-only mapping\n", pInfo->pwzPath);
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_READWRITE:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-write mapping\n", pInfo->pwzPath);
                    break;
                case PAGE_WRITECOPY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create copy-on-write mapping\n", pInfo->pwzPath);
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_EXECUTE_READ:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_EXECUTE_READWRITE:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-write-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    break;
                case PAGE_EXECUTE_WRITECOPY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-copy-on-write-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    pInfo->bCantWrite = TRUE;
                    break;
                }

                files.Remember(rv, pInfo);
            }
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
HANDLE WINAPI FIODetouringHooks::Hook_CreateFileMappingW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateFileMappingW\n");
    HANDLE rv = 0;
    __try {
        rv = Get().Real_CreateFileMappingW(hFile, lpAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName);
    } __finally {
        if (rv != NULL && rv != INVALID_HANDLE_VALUE) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            if (FIODetouringFiles::PFileInfo const pInfo = files.RecallFile(hFile)) {
                switch (flProtect) {
                case PAGE_READONLY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-only mapping\n", pInfo->pwzPath);
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_READWRITE:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-write mapping\n", pInfo->pwzPath);
                    break;
                case PAGE_WRITECOPY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create copy-on-write mapping\n", pInfo->pwzPath);
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_EXECUTE_READ:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_EXECUTE_READWRITE:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-write-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    break;
                case PAGE_EXECUTE_WRITECOPY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-copy-on-write-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    pInfo->bCantWrite = TRUE;
                    break;
                }

                files.Remember(rv, pInfo);
            }
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
HANDLE WINAPI FIODetouringHooks::Hook_CreateFileMappingNumaA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName, DWORD nndPreferred) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateFileMappingNumaA\n");
    HANDLE rv = 0;
    __try {
        rv = Get().Real_CreateFileMappingNumaA(hFile, lpAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName, nndPreferred);
    }
    __finally {
        if (rv != NULL && rv != INVALID_HANDLE_VALUE) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            if (FIODetouringFiles::PFileInfo const pInfo = files.RecallFile(hFile)) {
                switch (flProtect) {
                case PAGE_READONLY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-only mapping\n", pInfo->pwzPath);
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_READWRITE:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-write mapping\n", pInfo->pwzPath);
                    break;
                case PAGE_WRITECOPY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create copy-on-write mapping\n", pInfo->pwzPath);
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_EXECUTE_READ:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_EXECUTE_READWRITE:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-write-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    break;
                case PAGE_EXECUTE_WRITECOPY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-copy-on-write-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    pInfo->bCantWrite = TRUE;
                    break;
                }

                files.Remember(rv, pInfo);
            }
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
HANDLE WINAPI FIODetouringHooks::Hook_CreateFileMappingNumaW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName, DWORD nndPreferred) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateFileMappingNumaW\n");
    HANDLE rv = 0;
    __try {
        rv = Get().Real_CreateFileMappingNumaW(hFile, lpAttributes, flProtect, dwMaximumSizeHigh, dwMaximumSizeLow, lpName, nndPreferred);
    }
    __finally {
        if (rv != NULL && rv != INVALID_HANDLE_VALUE) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            if (FIODetouringFiles::PFileInfo const pInfo = files.RecallFile(hFile)) {
                switch (flProtect) {
                case PAGE_READONLY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-only mapping\n", pInfo->pwzPath);
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_READWRITE:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-write mapping\n", pInfo->pwzPath);
                    break;
                case PAGE_WRITECOPY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create copy-on-write mapping\n", pInfo->pwzPath);
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_EXECUTE_READ:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    pInfo->bCantWrite = TRUE;
                    break;
                case PAGE_EXECUTE_READWRITE:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-write-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    break;
                case PAGE_EXECUTE_WRITECOPY:
                    IODETOURING_DEBUGPRINTF("file '%ls' create read-copy-on-write-execute mapping\n", pInfo->pwzPath);
                    pInfo->bExecute = TRUE;
                    pInfo->bCantWrite = TRUE;
                    break;
                }

                files.Remember(rv, pInfo);
            }
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CreatePipe(PHANDLE hReadPipe, PHANDLE hWritePipe, LPSECURITY_ATTRIBUTES lpPipeAttributes, DWORD nSize) {
    IODETOURING_DEBUGPRINTF("call Hook_CreatePipe\n");
    HANDLE hRead = INVALID_HANDLE_VALUE;
    HANDLE hWrite = INVALID_HANDLE_VALUE;

    if (hReadPipe == NULL)
        hReadPipe = &hRead;
    if (hWritePipe == NULL)
        hWritePipe = &hWrite;

    BOOL rv = 0;
    __try {
        rv = Get().Real_CreatePipe(hReadPipe, hWritePipe, lpPipeAttributes, nSize);
    } __finally {
        if (rv) {
            CHAR szPipe[128];

            SafePrintf(szPipe, ARRAYSIZE(szPipe), "\\\\.\\PIPE\\Temp.%d.%d",
                       GetCurrentProcessId(),
                       InterlockedIncrement(&Get().nPipeCnt));

            FIODetouringFiles& files = FIODetouringFiles::Get();

            FIODetouringFiles::PFileInfo const pInfo = files.FindPartial(szPipe);
            pInfo->bPipe = TRUE;

            files.Remember(*hReadPipe, pInfo);
            files.Remember(*hWritePipe, pInfo);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CloseHandle(HANDLE hObject) {
    IODETOURING_DEBUGPRINTF("call Hook_CloseHandle\n");
    BOOL rv = 0;
    __try {
        FIODetouringFiles& files = FIODetouringFiles::Get();
        if (FIODetouringFiles::PProcInfo const pProc = files.RecallProc(hObject))
            files.Close(pProc->hProc);

        if (FIODetouringFiles::PFileInfo const pFile = files.RecallFile(hObject)) {
            DWORD dwErr = GetLastError();
            pFile->cbContent = GetFileSize(hObject, NULL);
            if (pFile->cbContent == INVALID_FILE_SIZE)
                pFile->cbContent = 0;

            if (pFile->bCantRead)
                pFile->bRead = FALSE;
            if (pFile->bCantWrite)
                pFile->bWrite = FALSE;

            SetLastError(dwErr);
        }
        rv = Get().Real_CloseHandle(hObject);
    } __finally {
        if (rv)
            FIODetouringFiles::Get().Forget(hObject);
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_DuplicateHandle(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle, DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwOptions) {
    IODETOURING_DEBUGPRINTF("call Hook_DuplicateHandle\n");
    HANDLE hTemp = INVALID_HANDLE_VALUE;

    BOOL rv = 0;
    __try {
        if (lpTargetHandle == NULL) {
            lpTargetHandle = &hTemp;
        }
        *lpTargetHandle = INVALID_HANDLE_VALUE;

        rv = Get().Real_DuplicateHandle(hSourceProcessHandle, hSourceHandle, hTargetProcessHandle, lpTargetHandle, dwDesiredAccess, bInheritHandle, dwOptions);

    } __finally {
        if (*lpTargetHandle != INVALID_HANDLE_VALUE) {
            FIODetouringFiles::Get().Duplicate(*lpTargetHandle, hSourceHandle);
        }
    };
    return rv;
}

//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateProcessA\n");
    if (lpCommandLine == NULL) {
        lpCommandLine = (LPSTR)lpApplicationName;
    }

    CHAR szProc[MAX_PATH];
    BOOL rv = FALSE;

    // retrieve application name either from user param or from command-line
    CHAR szPath[MAX_PATH];
    if (lpApplicationName == NULL) {
        PCHAR pszDst = szPath;
        PCHAR pszSrc = lpCommandLine;

        if (*pszSrc == '\"') {
            CHAR cQuote = *pszSrc++;

            while (*pszSrc && *pszSrc != cQuote) {
                *pszDst++ = *pszSrc++;
            }
            *pszDst++ = '\0';
        }
        else {
            while (*pszSrc && *pszSrc != ' ' && *pszSrc != '\t') {
                if (*pszSrc == '\t') {
                    *pszSrc = ' ';
                }
                *pszDst++ = *pszSrc++;
            }
            *pszDst++ = '\0';
        }
    }
    else {
        FIODetouringFiles::StringCopy(szPath, lpApplicationName);
    }

    __try {
        FIODetouringFiles& files = FIODetouringFiles::Get();

        if (files.ShouldDetourApplication(szPath)) {
            // use IO detouring on child process by copying parent's payload

            LPPROCESS_INFORMATION ppi = lpProcessInformation;
            PROCESS_INFORMATION pi;
            if (ppi == NULL) {
                ZeroMemory(&pi, sizeof(pi));
                ppi = &pi;
            }

            FIODetouringHooks& hooks = Get();
            rv = DetourCreateProcessWithDllExA(lpApplicationName,
                lpCommandLine,
                lpProcessAttributes,
                lpThreadAttributes,
                bInheritHandles,
                dwCreationFlags | CREATE_SUSPENDED,
                lpEnvironment,
                lpCurrentDirectory,
                lpStartupInfo,
                ppi,
                hooks.szDllPath,
                hooks.Real_CreateProcessA);

            if (rv) {
                HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
                HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
                HANDLE hStderr = GetStdHandle(STD_ERROR_HANDLE);

                if (lpStartupInfo != NULL && (lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) != 0) {
                    hStdin = lpStartupInfo->hStdInput;
                    hStdout = lpStartupInfo->hStdOutput;
                    hStderr = lpStartupInfo->hStdError;
                }

                FIODetouringTblog::Get().ChildPayload(ppi->hProcess, ppi->dwProcessId, szProc, hStdin, hStdout, hStderr);

                if (!(dwCreationFlags & CREATE_SUSPENDED))
                    ResumeThread(ppi->hThread);

                if (ppi == &pi) {
                    hooks.Real_CloseHandle(ppi->hThread);
                    hooks.Real_CloseHandle(ppi->hProcess);
                }
            }
        }
        else {
            // this application is explicitly ignored for IO detouring, use regular Real_CreateProcessX()

            rv = Get().Real_CreateProcessA(
                lpApplicationName, lpCommandLine,
                lpProcessAttributes, lpThreadAttributes,
                bInheritHandles, dwCreationFlags,
                lpEnvironment, lpCurrentDirectory,
                lpStartupInfo, lpProcessInformation);
        }
    }
    __finally {
        // always record access for executable path, weather using IO detouring or not
        if (rv) {
            if (FIODetouringFiles::PFileInfo pInfo = FIODetouringFiles::Get().FindPartial(szPath))
                pInfo->bExecute = true;
        }
    }
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateProcessW\n");
    if (lpCommandLine == NULL) {
        lpCommandLine = (LPWSTR)lpApplicationName;
    }

    CHAR szProc[MAX_PATH];
    BOOL rv = 0;

    // retrieve application name either from user param or from command-line
    WCHAR wzPath[MAX_PATH];
    if (lpApplicationName == NULL) {
        PWCHAR pwzDst = wzPath;
        PWCHAR pwzSrc = lpCommandLine;

        if (*pwzSrc == '\"') {
            WCHAR cQuote = *pwzSrc++;

            while (*pwzSrc && *pwzSrc != cQuote) {
                *pwzDst++ = *pwzSrc++;
            }
            *pwzDst++ = '\0';
        }
        else {
            while (*pwzSrc && *pwzSrc != ' ' && *pwzSrc != '\t') {
                if (*pwzSrc == '\t') {
                    *pwzSrc = ' ';
                }
                *pwzDst++ = *pwzSrc++;
            }
            *pwzDst++ = '\0';
        }
    }
    else {
        FIODetouringFiles::StringCopy(wzPath, lpApplicationName);
    }

    __try {
        FIODetouringFiles& files = FIODetouringFiles::Get();

        if (files.ShouldDetourApplication(wzPath)) {
            // use IO detouring on child process by copying parent's payload

            LPPROCESS_INFORMATION ppi = lpProcessInformation;
            PROCESS_INFORMATION pi;
            if (ppi == NULL) {
                ZeroMemory(&pi, sizeof(pi));
                ppi = &pi;
            }

            FIODetouringHooks& hooks = Get();
            rv = DetourCreateProcessWithDllExW(lpApplicationName,
                lpCommandLine,
                lpProcessAttributes,
                lpThreadAttributes,
                bInheritHandles,
                dwCreationFlags | CREATE_SUSPENDED,
                lpEnvironment,
                lpCurrentDirectory,
                lpStartupInfo,
                ppi,
                hooks.szDllPath,
                hooks.Real_CreateProcessW);

            if (rv) {
                HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
                HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
                HANDLE hStderr = GetStdHandle(STD_ERROR_HANDLE);

                if (lpStartupInfo != NULL && (lpStartupInfo->dwFlags & STARTF_USESTDHANDLES) != 0) {
                    hStdin = lpStartupInfo->hStdInput;
                    hStdout = lpStartupInfo->hStdOutput;
                    hStderr = lpStartupInfo->hStdError;
                }

                FIODetouringTblog::Get().ChildPayload(ppi->hProcess, ppi->dwProcessId, szProc, hStdin, hStdout, hStderr);

                if (!(dwCreationFlags & CREATE_SUSPENDED))
                    ResumeThread(ppi->hThread);

                if (ppi == &pi) {
                    hooks.Real_CloseHandle(ppi->hThread);
                    hooks.Real_CloseHandle(ppi->hProcess);
                }
            }
        }
        else {
            // this application is explicitly ignored for IO detouring, use regular Real_CreateProcessX()

            rv = Get().Real_CreateProcessW(
                lpApplicationName, lpCommandLine,
                lpProcessAttributes, lpThreadAttributes,
                bInheritHandles, dwCreationFlags,
                lpEnvironment, lpCurrentDirectory,
                lpStartupInfo, lpProcessInformation);
        }

    } __finally {
        // always record access for executable path, weather using IO detouring or not
        if (rv) {
            if (FIODetouringFiles::PFileInfo pInfo = FIODetouringFiles::Get().FindPartial(wzPath))
                pInfo->bExecute = true;
        }
    }
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_DeleteFileA(LPCSTR lpFileName) {
    IODETOURING_DEBUGPRINTF("call Hook_DeleteFileA\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_DeleteFileA(lpFileName);
    }
    __finally {
        FIODetouringFiles::Get().NoteDelete(lpFileName);
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_DeleteFileW(LPCWSTR lpFileName) {
    IODETOURING_DEBUGPRINTF("call Hook_DeleteFileW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_DeleteFileW(lpFileName);
    } __finally {
        FIODetouringFiles::Get().NoteDelete(lpFileName);
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped) {
    IODETOURING_DEBUGPRINTF("call Hook_DeviceIoControl\n");
    DWORD bytesReturned = 0;
    if (lpBytesReturned == NULL)
        lpBytesReturned = &bytesReturned;

    BOOL rv = 0;
    __try {
        rv = Get().Real_DeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
    } __finally {
        FIODetouringFiles& files = FIODetouringFiles::Get();
        files.SetRead(hDevice, 0);
        files.SetWrite(hDevice, 0);

        if (rv && dwIoControlCode != 0x390008 && dwIoControlCode != 0x4d0008 && dwIoControlCode != 0x6d0008) {
            files.RecallFile(hDevice);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
DWORD WINAPI FIODetouringHooks::Hook_GetFileAttributesW(LPCWSTR lpFileName) {
    IODETOURING_DEBUGPRINTF("call Hook_GetFileAttributesW\n");
    DWORD rv = 0;
    __try {
        rv = Get().Real_GetFileAttributesW(lpFileName);
    } __finally {
    };
    return rv;
}
//----------------------------------------------------------------------------
LPVOID WINAPI FIODetouringHooks::Hook_MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD  dwFileOffsetHigh, DWORD  dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap) {
    IODETOURING_DEBUGPRINTF("call Hook_MapViewOfFile\n");
    LPVOID rv = NULL;
    __try {
        rv = Get().Real_MapViewOfFile(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
    } __finally {
        if (dwDesiredAccess != 0 && dwNumberOfBytesToMap > 0 && rv != NULL) {
            if (FIODetouringFiles::PFileInfo const pInfo = FIODetouringFiles::Get().RecallFile(hFileMappingObject)) {
                if ((dwDesiredAccess & FILE_MAP_READ) == FILE_MAP_READ) {
                    pInfo->bRead = TRUE;
                    pInfo->cbRead += dwNumberOfBytesToMap;
                }
                if ((dwDesiredAccess & FILE_MAP_WRITE) == FILE_MAP_WRITE) {
                    pInfo->bWrite = TRUE;
                    pInfo->cbWrite += dwNumberOfBytesToMap;
                }
                if ((dwDesiredAccess & FILE_MAP_EXECUTE) == FILE_MAP_EXECUTE)
                    pInfo->bExecute = TRUE;
            }
        }
    }
    return rv;
}
//----------------------------------------------------------------------------
LPVOID WINAPI FIODetouringHooks::Hook_MapViewOfFileEx(HANDLE hFileMappingObject, DWORD  dwDesiredAccess, DWORD  dwFileOffsetHigh, DWORD  dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap, LPVOID lpBaseAddress) {
    IODETOURING_DEBUGPRINTF("call Hook_MapViewOfFileEx\n");
    LPVOID rv = NULL;
    __try {
        rv = Get().Real_MapViewOfFileEx(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap, lpBaseAddress);
    }
    __finally {
        if (dwDesiredAccess != 0 && dwNumberOfBytesToMap > 0 && rv != NULL) {
            if (FIODetouringFiles::PFileInfo const pInfo = FIODetouringFiles::Get().RecallFile(hFileMappingObject)) {
                if ((dwDesiredAccess & FILE_MAP_READ) == FILE_MAP_READ) {
                    pInfo->bRead = TRUE;
                    pInfo->cbRead += dwNumberOfBytesToMap;
                }
                if ((dwDesiredAccess & FILE_MAP_WRITE) == FILE_MAP_WRITE) {
                    pInfo->bWrite = TRUE;
                    pInfo->cbWrite += dwNumberOfBytesToMap;
                }
                if ((dwDesiredAccess & FILE_MAP_EXECUTE) == FILE_MAP_EXECUTE)
                    pInfo->bExecute = TRUE;
            }
        }
    }
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_MoveFileWithProgressA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags) {
    IODETOURING_DEBUGPRINTF("call Hook_MoveFileWithProgressA\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_MoveFileWithProgressA(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
    }
    __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteWrite(lpNewFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_MoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags) {
    IODETOURING_DEBUGPRINTF("call Hook_MoveFileWithProgressW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_MoveFileWithProgressW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
    } __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteWrite(lpNewFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName) {
    IODETOURING_DEBUGPRINTF("call Hook_MoveFileA\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_MoveFileA(lpExistingFileName, lpNewFileName);
    } __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteCleanup(lpExistingFileName);
            files.NoteWrite(lpNewFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName) {
    IODETOURING_DEBUGPRINTF("call Hook_MoveFileW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_MoveFileW(lpExistingFileName, lpNewFileName);
    } __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteCleanup(lpExistingFileName);
            files.NoteWrite(lpNewFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_MoveFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, DWORD dwFlags) {
    IODETOURING_DEBUGPRINTF("call Hook_MoveFileExA\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_MoveFileExA(lpExistingFileName, lpNewFileName, dwFlags);
    } __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteCleanup(lpExistingFileName);
            files.NoteWrite(lpNewFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags) {
    IODETOURING_DEBUGPRINTF("call Hook_MoveFileExW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
    } __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteCleanup(lpExistingFileName);
            files.NoteWrite(lpNewFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CopyFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags) {
    IODETOURING_DEBUGPRINTF("call Hook_CopyFileExA\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_CopyFileExA(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    } __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteWrite(lpNewFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags) {
    IODETOURING_DEBUGPRINTF("call Hook_CopyFileExW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_CopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    } __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteWrite(lpNewFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_PrivCopyFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags) {
    IODETOURING_DEBUGPRINTF("call Hook_PrivCopyFileExW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_PrivCopyFileExA(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    }
    __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteWrite(lpNewFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_PrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags) {
    IODETOURING_DEBUGPRINTF("call Hook_PrivCopyFileExW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_PrivCopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
    } __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteWrite(lpNewFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CreateHardLinkA(LPCSTR lpFileName, LPCSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateHardLinkA\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_CreateHardLinkA(lpFileName, lpExistingFileName, lpSecurityAttributes);
    } __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteWrite(lpFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_CreateHardLinkW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes) {
    IODETOURING_DEBUGPRINTF("call Hook_CreateHardLinkW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_CreateHardLinkW(lpFileName, lpExistingFileName, lpSecurityAttributes);
    } __finally {
        if (rv) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.NoteRead(lpExistingFileName);
            files.NoteWrite(lpFileName);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_SetStdHandle(DWORD nStdHandle, HANDLE hHandle) {
    IODETOURING_DEBUGPRINTF("call Hook_SetStdHandle\n");
     BOOL rv = 0;
    __try {
        rv = Get().Real_SetStdHandle(nStdHandle, hHandle);
    } __finally {
        if (rv)
            FIODetouringFiles::Get().SetStdio(hHandle);
    };
    return rv;}
//----------------------------------------------------------------------------
HMODULE WINAPI FIODetouringHooks::Hook_LoadLibraryA(LPCSTR lpLibFileName) {
    IODETOURING_DEBUGPRINTF("call Hook_LoadLibraryA\n");
    HMODULE rv = 0;
    __try {
        rv = Get().Real_LoadLibraryA(lpLibFileName);
    } __finally {
        if (rv != NULL && rv != INVALID_HANDLE_VALUE)
            FIODetouringFiles::Get().NoteExecute(rv);
    };
    return rv;
}
//----------------------------------------------------------------------------
HMODULE WINAPI FIODetouringHooks::Hook_LoadLibraryW(LPCWSTR lpLibFileName) {
    IODETOURING_DEBUGPRINTF("call Hook_LoadLibraryW\n");
    HMODULE rv = 0;
    __try {
        rv = Get().Real_LoadLibraryW(lpLibFileName);
    } __finally {
        if (rv != NULL && rv != INVALID_HANDLE_VALUE)
            FIODetouringFiles::Get().NoteExecute(rv);
    };
    return rv;
}
//----------------------------------------------------------------------------
HMODULE WINAPI FIODetouringHooks::Hook_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile,DWORD dwFlags) {
    IODETOURING_DEBUGPRINTF("call Hook_LoadLibraryExA\n");
    HMODULE rv = 0;
    __try {
        rv = Get().Real_LoadLibraryExA(lpLibFileName, hFile, dwFlags);
    } __finally {
        if (rv != NULL && rv != INVALID_HANDLE_VALUE) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.SetExecute(hFile);
            files.NoteExecute(rv);
        }

    };
    return rv;
}
//----------------------------------------------------------------------------
HMODULE WINAPI FIODetouringHooks::Hook_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags) {
    IODETOURING_DEBUGPRINTF("call Hook_LoadLibraryExW\n");
    HMODULE rv = 0;
    __try {
        rv = Get().Real_LoadLibraryExW(lpLibFileName, hFile, dwFlags);
    } __finally {
        if (rv != NULL && rv != INVALID_HANDLE_VALUE) {
            FIODetouringFiles& files = FIODetouringFiles::Get();
            files.SetExecute(hFile);
            files.NoteExecute(rv);
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
DWORD WINAPI FIODetouringHooks::Hook_SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) {
    IODETOURING_DEBUGPRINTF("call Hook_SetFilePointer\n");
    DWORD rv = 0;
    __try {
        rv = Get().Real_SetFilePointer(hFile, lDistanceToMove, lpDistanceToMoveHigh, dwMoveMethod);
    } __finally {
        LONG high = 0;
        if (lpDistanceToMoveHigh == NULL)
            lpDistanceToMoveHigh = &high;

        FIODetouringFiles::PFileInfo const pInfo = FIODetouringFiles::Get().RecallFile(hFile);
        if (pInfo != NULL) {
            if (dwMoveMethod == FILE_END && lDistanceToMove == 0xffffffff)
                pInfo->bAppend = TRUE;
        }
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod) {
    IODETOURING_DEBUGPRINTF("call Hook_SetFilePointerEx\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_SetFilePointerEx(hFile, liDistanceToMove, lpNewFilePointer, dwMoveMethod);
    } __finally {
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {
    IODETOURING_DEBUGPRINTF("call Hook_ReadFile\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    } __finally {
        if (rv)
            FIODetouringFiles::Get().SetRead(hFile, nNumberOfBytesToRead);
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_ReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpNumberOfBytesRead, LPOVERLAPPED_COMPLETION_ROUTINE lpOverlapped) {
    IODETOURING_DEBUGPRINTF("call Hook_ReadFileEx\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_ReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    } __finally {
        if (rv)
            FIODetouringFiles::Get().SetRead(hFile, nNumberOfBytesToRead);
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
    IODETOURING_DEBUGPRINTF("call Hook_WriteFile\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
    } __finally {
        if (rv)
            FIODetouringFiles::Get().SetWrite(hFile, nNumberOfBytesToWrite);
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_WriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) {
    IODETOURING_DEBUGPRINTF("call Hook_WriteFileEx\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_WriteFileEx(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine);
    } __finally {
        if (rv)
            FIODetouringFiles::Get().SetWrite(hFile, nNumberOfBytesToWrite);
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_WriteConsoleA(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved) {
    IODETOURING_DEBUGPRINTF("call Hook_WriteConsoleA\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_WriteConsoleA(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved);
    } __finally {
        if (rv)
            FIODetouringFiles::Get().SetWrite(hConsoleOutput, nNumberOfCharsToWrite);
    };
    return rv;
}
//----------------------------------------------------------------------------
BOOL WINAPI FIODetouringHooks::Hook_WriteConsoleW(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved) {
    IODETOURING_DEBUGPRINTF("call Hook_WriteConsoleW\n");
    BOOL rv = 0;
    __try {
        rv = Get().Real_WriteConsoleW(hConsoleOutput, lpBuffer, nNumberOfCharsToWrite, lpNumberOfCharsWritten, lpReserved);
    } __finally {
        if (rv)
            FIODetouringFiles::Get().SetWrite(hConsoleOutput, nNumberOfCharsToWrite);
    };
    return rv;
}
//----------------------------------------------------------------------------
DWORD WINAPI FIODetouringHooks::Hook_ExpandEnvironmentStringsA(PCSTR lpSrc, PCHAR lpDst, DWORD nSize) {
    IODETOURING_DEBUGPRINTF("call Hook_ExpandEnvironmentStringsA\n");
    return Get().Real_ExpandEnvironmentStringsA(lpSrc, lpDst, nSize);
}
//----------------------------------------------------------------------------
DWORD WINAPI FIODetouringHooks::Hook_ExpandEnvironmentStringsW(PCWSTR lpSrc, PWCHAR lpDst, DWORD nSize) {
    IODETOURING_DEBUGPRINTF("call Hook_ExpandEnvironmentStringsW\n");
    return Get().Real_ExpandEnvironmentStringsW(lpSrc, lpDst, nSize);
}
//----------------------------------------------------------------------------
DWORD WINAPI FIODetouringHooks::Hook_GetEnvironmentVariableA(PCSTR lpName, PCHAR lpBuffer, DWORD nSize) {
    IODETOURING_DEBUGPRINTF("call Hook_GetEnvironmentVariableA\n");
    DWORD rv = 0;
    __try {
        rv = Get().Real_GetEnvironmentVariableA(lpName, lpBuffer, nSize);
    }
    __finally {
        FIODetouringFiles::Get().UseEnvVar(lpName);
    };
    return rv;
}
//----------------------------------------------------------------------------
DWORD WINAPI FIODetouringHooks::Hook_GetEnvironmentVariableW(PCWSTR lpName, PWCHAR lpBuffer, DWORD nSize) {
    IODETOURING_DEBUGPRINTF("call Hook_GetEnvironmentVariableW\n");
    DWORD rv = 0;
    __try {
        rv = Get().Real_GetEnvironmentVariableW(lpName, lpBuffer, nSize);
    }
    __finally {
        FIODetouringFiles::Get().UseEnvVar(lpName);
    };
    return rv;
}
//----------------------------------------------------------------------------
PCWSTR CDECL FIODetouringHooks::Hook_wgetenv(PCWSTR var) {
    IODETOURING_DEBUGPRINTF("call Hook_wgetenv\n");
    PCWSTR rv = 0;
    __try {
        rv = Get().Real_wgetenv(var);
    }
    __finally {
        FIODetouringFiles::Get().UseEnvVar(var);
    };
    return rv;
}
//----------------------------------------------------------------------------
PCSTR CDECL FIODetouringHooks::Hook_getenv(PCSTR var) {
    IODETOURING_DEBUGPRINTF("call Hook_getenv\n");
    PCSTR rv = 0;
    __try {
        rv = Get().Real_getenv(var);
    }
    __finally {
        FIODetouringFiles::Get().UseEnvVar(var);
    };
    return rv;
}
//----------------------------------------------------------------------------
DWORD CDECL FIODetouringHooks::Hook_getenv_s(DWORD *pValue, PCHAR pBuffer, DWORD cBuffer, PCSTR varname) {
    IODETOURING_DEBUGPRINTF("call Hook_getenv_s\n");
    DWORD rv = 0;
    __try {
        DWORD value;
        if (pValue == NULL)
            pValue = &value;
        rv = Get().Real_getenv_s(pValue, pBuffer, cBuffer, varname);
    }
    __finally {
        FIODetouringFiles::Get().UseEnvVar(varname);
    }
    return rv;
}
//----------------------------------------------------------------------------
DWORD CDECL FIODetouringHooks::Hook_wgetenv_s(DWORD *pValue, PWCHAR pBuffer, DWORD cBuffer, PCWSTR varname) {
    IODETOURING_DEBUGPRINTF("call Hook_wgetenv_s\n");
    DWORD rv = 0;
    __try {
        DWORD value;
        if (pValue == NULL)
            pValue = &value;
        rv = Get().Real_wgetenv_s(pValue, pBuffer, cBuffer, varname);
    }
    __finally {
        FIODetouringFiles::Get().UseEnvVar(varname);
    }
    return rv;
}
//----------------------------------------------------------------------------
DWORD CDECL FIODetouringHooks::Hook_dupenv_s(PCHAR *ppBuffer, DWORD *pcBuffer, PCSTR varname) {
    IODETOURING_DEBUGPRINTF("call Hook_dupenv_s\n");
    DWORD rv = 0;
    __try {
        PCHAR pb;
        DWORD cb;
        if (ppBuffer == NULL)
            ppBuffer = &pb;
        if (pcBuffer == NULL)
            pcBuffer = &cb;
        rv = Get().Real_dupenv_s(ppBuffer, pcBuffer, varname);
    }
    __finally {
        FIODetouringFiles::Get().UseEnvVar(varname);
    }
    return rv;
}
//----------------------------------------------------------------------------
DWORD CDECL FIODetouringHooks::Hook_wdupenv_s(PWCHAR *ppBuffer, DWORD *pcBuffer, PCWSTR varname) {
    IODETOURING_DEBUGPRINTF("call Hook_wdupenv_s\n");
    DWORD rv = 0;
    __try {
        PWCHAR pb;
        DWORD cb;
        if (ppBuffer == NULL)
            ppBuffer = &pb;
        if (pcBuffer == NULL)
            pcBuffer = &cb;
        rv = Get().Real_wdupenv_s(ppBuffer, pcBuffer, varname);
    }
    __finally {
        FIODetouringFiles::Get().UseEnvVar(varname);
    }
    return rv;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
