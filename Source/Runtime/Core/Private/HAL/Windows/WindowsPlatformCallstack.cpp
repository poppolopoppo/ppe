#include "stdafx.h"

#include "HAL/Windows/WindowsPlatformCallstack.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/StringBuilder.h"

#include "HAL/Windows/DbgHelpWrapper.h"
#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"

#include <TlHelp32.h>
#include <wchar.h>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool GWindowsPlatformSymbolsInitialized = false;
static bool GWindowsPlatformSymbolsLoaded = false;
//----------------------------------------------------------------------------
// Retrieve true symbols path from OS environment
//----------------------------------------------------------------------------
static void GetSymbolsPath_(wchar_t* out_symbol_path, size_t max_length) {
    static const wchar_t* kSymbolsPathEnvironmentVariables[3] = {
        L"_NT_SYMBOL_PATH",
        L"_NT_ALTERNATE_SYMBOL_PATH"
    };

    FWFixedSizeTextWriter oss(out_symbol_path, max_length);
    oss << L".;";

    wchar_t temp_buffer[MAX_PATH];
    ::DWORD call_result;

    call_result = ::GetCurrentDirectory(MAX_PATH, temp_buffer);

    if (call_result > 0)
        oss << MakeCStringView(temp_buffer) << L';';

    call_result = ::GetModuleFileName(NULL, temp_buffer, MAX_PATH);

    if (call_result > 0) {
        size_t tempBufferLength = Length(temp_buffer);
        wchar_t* searchPointer = temp_buffer + tempBufferLength - 1;
        for (; searchPointer >= temp_buffer; --searchPointer) {
            if ((*searchPointer == L'\\')
                || (*searchPointer == L'/')
                || (*searchPointer == L':')) {
                *searchPointer = 0;
                break;
            }
        }

        FWStringView modulePath = MakeCStringView(temp_buffer);
        if (not modulePath.empty())
            oss << modulePath << L';';
    }

    for (int variable_id = 0; variable_id < 2; ++variable_id) {
        call_result = ::GetEnvironmentVariable(
            kSymbolsPathEnvironmentVariables[variable_id],
            temp_buffer,
            MAX_PATH);
        if (call_result > 0)
            oss << MakeCStringView(temp_buffer) << L';';
    }

    call_result = ::GetEnvironmentVariable(
        L"SYSTEMROOT",
        temp_buffer,
        MAX_PATH);

    if (call_result > 0)
        oss << MakeCStringView(temp_buffer) << L"\\System32;";

    call_result = ::GetEnvironmentVariable(
        L"SYSTEMDRIVE",
        temp_buffer,
        MAX_PATH);

    if (call_result > 0)
    {
        wchar_t webSymbolsBuffer[MAX_PATH];
        {
            FWFixedSizeTextWriter tmp(webSymbolsBuffer);
            tmp << MakeCStringView(temp_buffer) << L"\\symcache"
                << Eos;
        }

        const FWStringView webSymbolsPath = MakeCStringView(webSymbolsBuffer);

        call_result = ::CreateDirectoryW(webSymbolsPath.data(), NULL);
        if (call_result == 0)
            Assert(ERROR_ALREADY_EXISTS == GetLastError());

        oss << L"cache*" << webSymbolsPath << L';'
            << L"SRV*" << webSymbolsPath
            << L"*http://msdl.microsoft.com/download/symbols;";
    }
}
//----------------------------------------------------------------------------
// Fetch all symbols for modules currently loaded by this process
static void LoadModules_(const FDbghelpWrapper::FLocked& dbghelp) {
    ::HANDLE process = ::GetCurrentProcess();
    ::DWORD process_id = ::GetCurrentProcessId();
    ::HANDLE snap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);

    ::MODULEENTRY32 module_entry;
    module_entry.dwSize = sizeof(::MODULEENTRY32);

    if (snap == (::HANDLE)-1)
        return;

    PRAGMA_MSVC_WARNING_PUSH()
    PRAGMA_MSVC_WARNING_DISABLE(4826) // warning C4826: convert unsigned char* to DWORD64
    ::BOOL module_found = ::Module32First(snap, &module_entry);
    while (module_found) {
        ::DWORD64 succeed = dbghelp.SymLoadModuleExW()(
            process,
            0,
            module_entry.szExePath,
            module_entry.szModule,
            (::DWORD64)module_entry.modBaseAddr,
            module_entry.modBaseSize,
            NULL,
            0);
        PRAGMA_MSVC_WARNING_POP()

            LOG(HAL, Info, L"{0} for \"{1}\"",
                succeed ? L"Loaded" : L"Failed to load",
                module_entry.szExePath);
        UNUSED(succeed);

        module_found = ::Module32Next(snap, &module_entry);
    }

    ::CloseHandle(snap);
}
//----------------------------------------------------------------------------
// Must be called once per run before any symbol query
static void InitializeSymbols_(const FDbghelpWrapper::FLocked& dbghelp) {
    ::DWORD options = dbghelp.SymGetOptions()();
    options |= SYMOPT_LOAD_LINES;
    options |= SYMOPT_FAIL_CRITICAL_ERRORS;
    options |= SYMOPT_DEFERRED_LOADS;
    options |= SYMOPT_UNDNAME;
    dbghelp.SymSetOptions()(options);

    // Force standard malloc, this is called at a very early stage for custom allocators
    STATIC_CONST_INTEGRAL(size_t, SYMBOL_PATH_CAPACITY, 8 * MAX_PATH);
    wchar_t symbol_path[SYMBOL_PATH_CAPACITY] = { 0 };

    GetSymbolsPath_(symbol_path, SYMBOL_PATH_CAPACITY);

    ::HANDLE process = ::GetCurrentProcess();
    ::BOOL succeed = dbghelp.SymInitializeW()(process, symbol_path, FALSE);
    UNUSED(succeed);

    LOG(HAL, Info, L"path = '{0}' -> succeed = {1:A}", symbol_path, (FALSE != succeed));
}
//----------------------------------------------------------------------------
} //!namepsace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FWindowsPlatformCallstack::OnLoadModule() {
    GWindowsPlatformSymbolsLoaded = false;
}
//----------------------------------------------------------------------------
void FWindowsPlatformCallstack::LoadSymbolInfos() {
    if (GWindowsPlatformSymbolsLoaded) {
        GWindowsPlatformSymbolsInitialized = true;
        return;
    }

    const FDbghelpWrapper::FLocked dbghelp;

    if (not GWindowsPlatformSymbolsInitialized) {
        InitializeSymbols_(dbghelp);
        GWindowsPlatformSymbolsInitialized = true;
    }

    LoadModules_(dbghelp);

    GWindowsPlatformSymbolsLoaded = true;
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformCallstack::CaptureCallstack(const TMemoryView<FProgramCounter>& backtrace, size_t framesToSkip) {
    Assert(not backtrace.empty());

    if (not GWindowsPlatformSymbolsLoaded)
        LoadSymbolInfos();

    const ::WORD depth = ::RtlCaptureStackBackTrace(
        checked_cast<::DWORD>(framesToSkip),
        checked_cast<::DWORD>(backtrace.size()),
        backtrace.data(),
        nullptr );

    return checked_cast<size_t>(depth);
}
//----------------------------------------------------------------------------
bool FWindowsPlatformCallstack::ProgramCounterToModuleName(FWString* moduleName, FProgramCounter pc) {
    Assert(GWindowsPlatformSymbolsLoaded); // LoadSymbolInfos() before
    Assert(moduleName);
    Assert(pc);

    const ::HANDLE hProcess = ::GetCurrentProcess();

    ::IMAGEHLP_MODULEW64 moduleInfo; // print name of module + address if PDBs are not available
    ::ZeroMemory(&moduleInfo, sizeof(moduleInfo));
    moduleInfo.SizeOfStruct = sizeof(moduleInfo);

    const FDbghelpWrapper::FLocked dbghelp;

    if (not dbghelp.SymGetModuleInfoW64()(hProcess, ::DWORD64(uintptr_t(pc)), &moduleInfo))
        return false;

    *moduleName = MakeCStringView(moduleInfo.ImageName);

    return true;
}
//----------------------------------------------------------------------------
bool FWindowsPlatformCallstack::ProgramCounterToSymbolInfo(FProgramCounterSymbolInfo* symbolInfo, FProgramCounter pc) {
    Assert(GWindowsPlatformSymbolsLoaded); // LoadSymbolInfos() before
    Assert(symbolInfo);
    Assert(pc);

    const ::HANDLE hProcess = ::GetCurrentProcess();

    char buffer[sizeof(::SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(WCHAR)] = { 0 };
    ::PSYMBOL_INFOW pSymbol = (::PSYMBOL_INFOW)buffer;
    pSymbol->SizeOfStruct = sizeof(::SYMBOL_INFOW);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    ::IMAGEHLP_LINEW64 line64;
    line64.SizeOfStruct = sizeof(::IMAGEHLP_LINEW64);

    ::IMAGEHLP_MODULEW64 moduleInfo; // print name of module + address if PDBs are not available
    ::ZeroMemory(&moduleInfo, sizeof(moduleInfo));
    moduleInfo.SizeOfStruct = sizeof(moduleInfo);

    const FDbghelpWrapper::FLocked dbghelp;
    {
        ::DWORD64 dw64Displacement = 0;
        if (dbghelp.SymFromAddrW()(hProcess, ::DWORD64(uintptr_t(pc)), &dw64Displacement, pSymbol))
            symbolInfo->Function = FWStringView(pSymbol->Name, checked_cast<size_t>(pSymbol->NameLen));
        else
            symbolInfo->Function = ToWString(FLastError());
    }
    {
        ::DWORD dwDisplacement = 0;
        if (dbghelp.SymGetLineFromAddrW64()(hProcess, ::DWORD64(uintptr_t(pc)), &dwDisplacement, &line64)) {
            symbolInfo->Filename = MakeCStringView(line64.FileName);
            symbolInfo->Line = checked_cast<size_t>(line64.LineNumber);
        }
        else if (dbghelp.SymGetModuleInfoW64()(hProcess, ::DWORD64(uintptr_t(pc)), &moduleInfo)) {
            symbolInfo->Filename = MakeCStringView(moduleInfo.ImageName);
            symbolInfo->Line = 0;
        }
        else {
            static const wchar_t kUnknown[] = L"????????";
            symbolInfo->Filename = MakeStringView(kUnknown);
            symbolInfo->Line = size_t(pc);
        }
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS