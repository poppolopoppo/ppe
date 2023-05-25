#pragma once

#include "IODetouring.h"

#define IODETOURING_REAL(_FUNC, ...) FIODetouringHooks::Get().Real_##_FUNC(__VA_ARGS__)

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Win32 API IO Hooking
// Based on Microsoft Detours sample `Tracebld`:
// https://github.com/microsoft/Detours/wiki/SampleTracebld
//----------------------------------------------------------------------------
class PPE_IODETOURING_API FIODetouringHooks {
public:
    struct force_init_t {};
    static FIODetouringHooks& Get();
    explicit FIODetouringHooks(force_init_t) NOEXCEPT;

    NO_INLINE void InitializeRealFunctions();

    LONG AttachDetours();
    LONG DetachDetours();

    HMODULE hInstance = NULL;
    HMODULE hKernel32 = NULL;

    HMODULE hMsvcr = NULL;
    PCSTR pszMsvcr = NULL;

    CHAR szDllPath[MAX_PATH]{};

    LONG nPipeCnt{ 0 };

    BOOL bLog = FALSE;

public: // IO hooks:
    static int WINAPI Hook_EntryPoint(VOID);
    int (WINAPI *Real_EntryPoint)(VOID)
        = NULL; // Resolved in DllMain()
    static VOID WINAPI Hook_ExitProcess(UINT uExitCode);
    VOID(WINAPI *Real_ExitProcess)(UINT uExitCode)
        = ::ExitProcess;

    static BOOL WINAPI Hook_CreateDirectoryA(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    BOOL(WINAPI* Real_CreateDirectoryA)(LPCSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
        = ::CreateDirectoryA;
    static BOOL WINAPI Hook_CreateDirectoryW(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    BOOL(WINAPI *Real_CreateDirectoryW)(LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
        = ::CreateDirectoryW;

    static BOOL WINAPI Hook_CreateDirectoryExA(LPCSTR lpTemplateDirectory, LPCSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    BOOL(WINAPI* Real_CreateDirectoryExA)(LPCSTR lpTemplateDirectory, LPCSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
        = ::CreateDirectoryExA;
    static BOOL WINAPI Hook_CreateDirectoryExW(LPCWSTR lpTemplateDirectory, LPCWSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    BOOL(WINAPI *Real_CreateDirectoryExW)(LPCWSTR lpTemplateDirectory, LPCWSTR lpNewDirectory, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
        = ::CreateDirectoryExW;

    static HANDLE WINAPI Hook_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
    HANDLE(WINAPI* Real_CreateFileA)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
        = ::CreateFileA;
    static HANDLE WINAPI Hook_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
    HANDLE(WINAPI *Real_CreateFileW)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
        = ::CreateFileW;

    static HANDLE WINAPI Hook_CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName);
    HANDLE(WINAPI* Real_CreateFileMappingA)(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName)
        = ::CreateFileMappingA;
    static HANDLE WINAPI Hook_CreateFileMappingW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName);
    HANDLE(WINAPI *Real_CreateFileMappingW)(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName)
        = ::CreateFileMappingW;

    static HANDLE WINAPI Hook_CreateFileMappingNumaA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName, DWORD nndPreferred);
    HANDLE(WINAPI* Real_CreateFileMappingNumaA)(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName, DWORD nndPreferred)
        = ::CreateFileMappingNumaA;
    static HANDLE WINAPI Hook_CreateFileMappingNumaW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName, DWORD nndPreferred);
    HANDLE(WINAPI* Real_CreateFileMappingNumaW)(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName, DWORD nndPreferred)
        = ::CreateFileMappingNumaW;

    static BOOL WINAPI Hook_CreatePipe(PHANDLE hReadPipe, PHANDLE hWritePipe, LPSECURITY_ATTRIBUTES lpPipeAttributes, DWORD nSize);
    BOOL(WINAPI *Real_CreatePipe)(PHANDLE hReadPipe, PHANDLE hWritePipe, LPSECURITY_ATTRIBUTES lpPipeAttributes, DWORD nSize)
        = ::CreatePipe;

    static BOOL WINAPI Hook_CloseHandle(HANDLE hObject);
    BOOL(WINAPI *Real_CloseHandle)(HANDLE hObject)
        = ::CloseHandle;

    static BOOL WINAPI Hook_DuplicateHandle(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle, DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwOptions);
    BOOL(WINAPI *Real_DuplicateHandle)(HANDLE hSourceProcessHandle, HANDLE hSourceHandle, HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle, DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwOptions)
        = ::DuplicateHandle;

    static BOOL WINAPI Hook_CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
    BOOL(WINAPI *Real_CreateProcessW)(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
        = ::CreateProcessW;

    static BOOL WINAPI Hook_CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
    BOOL(WINAPI *Real_CreateProcessA)(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
        = ::CreateProcessA;

    static BOOL WINAPI Hook_DeleteFileA(LPCSTR lpFileName);
    BOOL(WINAPI* Real_DeleteFileA)(LPCSTR lpFileName)
        = ::DeleteFileA;
    static BOOL WINAPI Hook_DeleteFileW(LPCWSTR lpFileName);
    BOOL(WINAPI *Real_DeleteFileW)(LPCWSTR lpFileName)
        = ::DeleteFileW;

    static BOOL WINAPI Hook_DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped);
    BOOL(WINAPI *Real_DeviceIoControl)(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
        = ::DeviceIoControl;

    static DWORD WINAPI Hook_GetFileAttributesW(LPCWSTR lpFileName);
    DWORD(WINAPI *Real_GetFileAttributesW)(LPCWSTR lpFileName)
        = ::GetFileAttributesW;

    static LPVOID Hook_MapViewOfFile(HANDLE hFileMappingObject, DWORD  dwDesiredAccess, DWORD  dwFileOffsetHigh, DWORD  dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);
    LPVOID(WINAPI* Real_MapViewOfFile)(HANDLE hFileMappingObject, DWORD  dwDesiredAccess, DWORD  dwFileOffsetHigh, DWORD  dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap)
        = ::MapViewOfFile;

    static LPVOID Hook_MapViewOfFileEx(HANDLE hFileMappingObject, DWORD  dwDesiredAccess, DWORD  dwFileOffsetHigh, DWORD  dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap, LPVOID lpBaseAddress);
    LPVOID(WINAPI* Real_MapViewOfFileEx)(HANDLE hFileMappingObject, DWORD  dwDesiredAccess, DWORD  dwFileOffsetHigh, DWORD  dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap, LPVOID lpBaseAddress)
        = ::MapViewOfFileEx;

    static BOOL WINAPI Hook_MoveFileWithProgressA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags);
    BOOL(WINAPI* Real_MoveFileWithProgressA)(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
        = ::MoveFileWithProgressA;
    static BOOL WINAPI Hook_MoveFileWithProgressW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags);
    BOOL(WINAPI *Real_MoveFileWithProgressW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
        = ::MoveFileWithProgressW;

    static BOOL WINAPI Hook_MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName);
    BOOL(WINAPI *Real_MoveFileA)(LPCSTR lpExistingFileName, LPCSTR lpNewFileName)
        = ::MoveFileA;
    static BOOL WINAPI Hook_MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);
    BOOL(WINAPI *Real_MoveFileW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
        = ::MoveFileW;

    static BOOL WINAPI Hook_MoveFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, DWORD dwFlags);
    BOOL(WINAPI *Real_MoveFileExA)(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, DWORD dwFlags)
        = ::MoveFileExA;
    static BOOL WINAPI Hook_MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags);
    BOOL(WINAPI *Real_MoveFileExW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags) = ::MoveFileExW;

    static BOOL WINAPI Hook_CopyFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
    BOOL(WINAPI *Real_CopyFileExA)(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
        = ::CopyFileExA;
    static BOOL WINAPI Hook_CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
    BOOL(WINAPI *Real_CopyFileExW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
        = ::CopyFileExW;

    static BOOL WINAPI Hook_PrivCopyFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
    BOOL(WINAPI* Real_PrivCopyFileExA)(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
        = NULL;
    static BOOL WINAPI Hook_PrivCopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags);
    BOOL(WINAPI *Real_PrivCopyFileExW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
        = NULL;

    static BOOL WINAPI Hook_CreateHardLinkA(LPCSTR lpFileName, LPCSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    BOOL(WINAPI *Real_CreateHardLinkA)(LPCSTR lpFileName, LPCSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
        = ::CreateHardLinkA;
    static BOOL WINAPI Hook_CreateHardLinkW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    BOOL(WINAPI *Real_CreateHardLinkW)(LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
        = ::CreateHardLinkW;

    static BOOL WINAPI Hook_SetStdHandle(DWORD nStdHandle, HANDLE hHandle);
    BOOL(WINAPI *Real_SetStdHandle)(DWORD nStdHandle, HANDLE hHandle)
        = ::SetStdHandle;

    static HMODULE WINAPI Hook_LoadLibraryA(LPCSTR lpLibFileName);
    HMODULE(WINAPI *Real_LoadLibraryA)(LPCSTR lpLibFileName)
        = ::LoadLibraryA;
    static HMODULE WINAPI Hook_LoadLibraryW(LPCWSTR lpLibFileName);
    HMODULE(WINAPI *Real_LoadLibraryW)(LPCWSTR lpLibFileName)
        = ::LoadLibraryW;

    static HMODULE WINAPI Hook_LoadLibraryExA(LPCSTR lpLibFileName, HANDLE hFile,DWORD dwFlags);
    HMODULE(WINAPI *Real_LoadLibraryExA)(LPCSTR lpLibFileName, HANDLE hFile,DWORD dwFlags)
        = ::LoadLibraryExA;
    static HMODULE WINAPI Hook_LoadLibraryExW(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
    HMODULE(WINAPI *Real_LoadLibraryExW)(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
        = ::LoadLibraryExW;

    static DWORD WINAPI Hook_SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
    DWORD(WINAPI *Real_SetFilePointer)(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod)
        = ::SetFilePointer;

    static BOOL WINAPI Hook_SetFilePointerEx(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod);
    BOOL(WINAPI *Real_SetFilePointerEx)(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod)
        = ::SetFilePointerEx;

    static BOOL WINAPI Hook_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
    BOOL(WINAPI *Real_ReadFile)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
        = ::ReadFile;

    static BOOL WINAPI Hook_ReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpNumberOfBytesRead, LPOVERLAPPED_COMPLETION_ROUTINE lpOverlapped);
    BOOL(WINAPI *Real_ReadFileEx)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpNumberOfBytesRead, LPOVERLAPPED_COMPLETION_ROUTINE lpOverlapped)
        = ::ReadFileEx;

    static BOOL WINAPI Hook_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
    BOOL(WINAPI *Real_WriteFile)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
        = ::WriteFile;

    static BOOL WINAPI Hook_WriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
    BOOL(WINAPI *Real_WriteFileEx)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
        = ::WriteFileEx;

    static BOOL WINAPI Hook_WriteConsoleA(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved);
    BOOL(WINAPI *Real_WriteConsoleA)(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved)
        = ::WriteConsoleA;
    static BOOL WINAPI Hook_WriteConsoleW(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved);
    BOOL(WINAPI *Real_WriteConsoleW)(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved)
        = ::WriteConsoleW;

    static DWORD WINAPI Hook_ExpandEnvironmentStringsA(PCSTR lpSrc, PCHAR lpDst, DWORD nSize);
    DWORD(WINAPI *Real_ExpandEnvironmentStringsA)(PCSTR lpSrc, PCHAR lpDst, DWORD nSize)
        = ::ExpandEnvironmentStringsA;
    static DWORD WINAPI Hook_ExpandEnvironmentStringsW(PCWSTR lpSrc, PWCHAR lpDst, DWORD nSize);
    DWORD(WINAPI *Real_ExpandEnvironmentStringsW)(PCWSTR lpSrc, PWCHAR lpDst, DWORD nSize)
        = ::ExpandEnvironmentStringsW;

    static DWORD WINAPI Hook_GetEnvironmentVariableA(PCSTR lpName, PCHAR lpBuffer, DWORD nSize);
    DWORD(WINAPI *Real_GetEnvironmentVariableA)(PCSTR lpName, PCHAR lpBuffer, DWORD nSize)
        = ::GetEnvironmentVariableA;
    static DWORD WINAPI Hook_GetEnvironmentVariableW(PCWSTR lpName, PWCHAR lpBuffer, DWORD nSize);
    DWORD(WINAPI *Real_GetEnvironmentVariableW)(PCWSTR lpName, PWCHAR lpBuffer, DWORD nSize)
        = ::GetEnvironmentVariableW;

    static PCWSTR CDECL Hook_wgetenv(PCWSTR var);
    PCWSTR(CDECL *Real_wgetenv)(PCWSTR var)
        = NULL; // Resolved in Real_EntryPoint()
    static PCSTR CDECL Hook_getenv(PCSTR var);
    PCSTR(CDECL *Real_getenv)(PCSTR var)
        = NULL; // Resolved in Real_EntryPoint()
    static DWORD CDECL Hook_getenv_s(DWORD *pValue, PCHAR pBuffer, DWORD cBuffer, PCSTR varname);
    DWORD(CDECL *Real_getenv_s)(DWORD *pValue, PCHAR pBuffer, DWORD cBuffer, PCSTR varname)
        = NULL; // Resolved in Real_EntryPoint()
    static DWORD CDECL Hook_wgetenv_s(DWORD *pValue, PWCHAR pBuffer, DWORD cBuffer, PCWSTR varname);
    DWORD(CDECL *Real_wgetenv_s)(DWORD *pValue, PWCHAR pBuffer, DWORD cBuffer, PCWSTR varname)
        = NULL; // Resolved in Real_EntryPoint()
    static DWORD CDECL Hook_dupenv_s(PCHAR *ppBuffer, DWORD *pcBuffer, PCSTR varname);
    DWORD(CDECL *Real_dupenv_s)(PCHAR *ppBuffer, DWORD *pcBuffer, PCSTR varname)
        = NULL; // Resolved in Real_EntryPoint()
    static DWORD CDECL Hook_wdupenv_s(PWCHAR *ppBuffer, DWORD *pcBuffer, PCWSTR varname);
    DWORD(CDECL *Real_wdupenv_s)(PWCHAR *ppBuffer, DWORD *pcBuffer, PCWSTR varname)
        = NULL; // Resolved in Real_EntryPoint()
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
