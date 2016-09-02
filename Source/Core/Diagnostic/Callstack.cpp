#include "stdafx.h"

#include "Callstack.h"

#include "DecodedCallstack.h"
#include "Logger.h"

#include "IO/Stream.h"
#include "IO/String.h"
#include "IO/StringView.h"

#include <mutex>

// TODO: DIA - to support fastlink
// https://blogs.msdn.microsoft.com/vcblog/2010/01/05/dia-based-stack-walking/
// https://msdn.microsoft.com/en-us/library/hd8h6f46.aspx

#ifdef OS_WINDOWS
#   include "DbghelpWrapper.h"
#   include <TlHelp32.h>
#   include <wchar.h>
#endif

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// Retrieve true symbols path from OS environment
//----------------------------------------------------------------------------
static void GetSymbolsPath_(wchar_t* out_symbol_path, size_t max_length) {
    static const wchar_t* kSymbolsPathEnvironmentVariables[3] = {
        L"_NT_SYMBOL_PATH",
        L"_NT_ALTERNATE_SYMBOL_PATH"
    };

    WOCStrStream oss{ out_symbol_path, checked_cast<std::streamsize>(max_length) };
    oss << L".;";

    wchar_t temp_buffer[MAX_PATH];
    DWORD call_result;

    call_result = ::GetCurrentDirectory(MAX_PATH, temp_buffer);

    if (call_result > 0)
        oss << temp_buffer << L';';

    call_result = ::GetModuleFileName(NULL, temp_buffer, MAX_PATH);

    if (call_result > 0) {
        size_t tempBufferLength = Length(temp_buffer);
        wchar_t* searchPointer = temp_buffer + tempBufferLength - 1;
        for (; searchPointer >= temp_buffer; --searchPointer) {
            if (   (*searchPointer  == L'\\')
                    || (*searchPointer  == L'/')
                    || (*searchPointer  == L':') ) {
                *searchPointer = 0;
                break;
            }
        }
        if (Length(temp_buffer) > 0)
            oss << temp_buffer << L';';
    }

    for (int variable_id = 0; variable_id < 2; ++variable_id) {
        call_result = ::GetEnvironmentVariable(
                        kSymbolsPathEnvironmentVariables[variable_id],
                        temp_buffer,
                        MAX_PATH);
        if (call_result > 0)
            oss << temp_buffer << L';';
    }

    call_result = ::GetEnvironmentVariable(
                    L"SYSTEMROOT",
                    temp_buffer,
                    MAX_PATH);

    if (call_result > 0)
        oss << temp_buffer << L"\\System32;";

    call_result = ::GetEnvironmentVariable(
                    L"SYSTEMDRIVE",
                    temp_buffer,
                    MAX_PATH);

    if (call_result > 0)
    {
        wchar_t webSymbolsPath[MAX_PATH];
        {
            const wchar_t* webSymbolsDir = L"\\symcache";
            WOCStrStream tmp{ webSymbolsPath, checked_cast<std::streamsize>(lengthof(webSymbolsPath)) };
            tmp << temp_buffer << webSymbolsDir;
        }

        call_result = ::CreateDirectoryW(webSymbolsPath, NULL);
        if (call_result == 0)
            Assert(ERROR_ALREADY_EXISTS == GetLastError());

        oss << L"cache*" << webSymbolsPath << L';'
            << L"SRV*" << webSymbolsPath
            << L"*http://msdl.microsoft.com/download/symbols;";
    }
}
//----------------------------------------------------------------------------
// Fetch all symbols for modules currently loaded by this process
static void LoadModules_(const DbghelpWrapper::Locked& dbghelp) {
    HANDLE  process = ::GetCurrentProcess();
    DWORD   process_id = ::GetCurrentProcessId();
    HANDLE  snap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);

    MODULEENTRY32 module_entry;
    module_entry.dwSize = sizeof(MODULEENTRY32);

    if (snap == (HANDLE)-1)
        return;

#pragma warning( push )
#pragma warning( disable : 4826 ) // warning C4826: La conversion de 'unsigned char *const ' en 'DWORD64' est de type signe étendu.
    BOOL module_found = ::Module32First(snap, &module_entry);
    while (module_found) {
        DWORD64 succeed = dbghelp.SymLoadModuleExW()(
            process,
            0,
            module_entry.szExePath,
            module_entry.szModule,
            (DWORD64)module_entry.modBaseAddr,
            module_entry.modBaseSize,
            NULL,
            0);
#pragma warning( pop )

        LOG(Info, L"[Symbols] {0} for \"{1}\"",
            succeed ? L"Loaded" : L"Failed to load",
            module_entry.szExePath);
        UNUSED(succeed);

        module_found = ::Module32Next(snap, &module_entry);
    }

    ::CloseHandle(snap);
}
//----------------------------------------------------------------------------
static void InitializeSymbols_(const DbghelpWrapper::Locked& dbghelp) {
    DWORD options = dbghelp.SymGetOptions()();
    options |= SYMOPT_LOAD_LINES;
    options |= SYMOPT_FAIL_CRITICAL_ERRORS;
    options |= SYMOPT_DEFERRED_LOADS;
    options |= SYMOPT_UNDNAME;
    dbghelp.SymSetOptions()(options);

    // Force standard malloc, this is called at a very early stage for custom allocators
    STATIC_CONST_INTEGRAL(size_t, SYMBOL_PATH_CAPACITY, 32 * MAX_PATH);
    wchar_t *const symbol_path = (wchar_t*)std::malloc(SYMBOL_PATH_CAPACITY * sizeof(wchar_t));
    memset(symbol_path, 0x0, SYMBOL_PATH_CAPACITY * sizeof(wchar_t));

    GetSymbolsPath_(symbol_path, SYMBOL_PATH_CAPACITY);

    HANDLE process = ::GetCurrentProcess();
    BOOL succeed = dbghelp.SymInitializeW()(process, symbol_path, FALSE);

    LOG(Info, L"[Symbols] Path = '{0}' -> succeed = {1:A}", symbol_path, (FALSE != succeed));

    std::free(symbol_path);

    if (FALSE == succeed)
        return;

    LoadModules_(dbghelp);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Callstack::Callstack()
: _hash(0), _depth(0) {
    memset(_frames, 0xCD, sizeof(_frames));
}
//----------------------------------------------------------------------------
Callstack::Callstack(size_t framesToSkip, size_t framesToCapture)
: Callstack() {
    Capture(this, framesToSkip, framesToCapture);
}
//----------------------------------------------------------------------------
Callstack::~Callstack() {}
//----------------------------------------------------------------------------
Callstack::Callstack(const Callstack& other)
: _hash(other._hash), _depth(other._depth) {
    memcpy(_frames, other._frames, sizeof(_frames));
}
//----------------------------------------------------------------------------
Callstack& Callstack::operator =(const Callstack& other) {
    if (this != &other) {
        _hash = other._hash;
        _depth = other._depth;
        memcpy(_frames, other._frames, sizeof(_frames));
    }
    return *this;
}
//----------------------------------------------------------------------------
void Callstack::Decode(DecodedCallstack* decoded) const {
    Decode(decoded, _hash, Frames());
}
//----------------------------------------------------------------------------
void Callstack::Decode(DecodedCallstack* decoded, size_t hash, const MemoryView<void* const>& frames) {
    Assert(decoded);

    const auto dbghelp(DbghelpWrapper::Instance().Lock());

    static const wchar_t* kUnknown = L"?????????????????????";

    decoded->_hash = hash;
    decoded->_depth = frames.size();

    HANDLE hProcess = ::GetCurrentProcess();

    char buffer[sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(WCHAR)] = {0};
    PSYMBOL_INFOW pSymbol = (PSYMBOL_INFOW)buffer;
    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
    pSymbol->MaxNameLen = MAX_SYM_NAME;

    IMAGEHLP_LINEW64 line64;
    line64.SizeOfStruct = sizeof(IMAGEHLP_LINEW64);

#pragma warning( push )
#pragma warning( disable : 4826 ) // warning C4826: La conversion de 'unsigned char *const ' en 'DWORD64' est de type signe étendu.
    void* const* address = frames.data();
    auto frame = reinterpret_cast<DecodedCallstack::Frame *>(&decoded->_frames);
    for (size_t i = 0; i < frames.size(); ++i, ++frame, ++address) {
        const wchar_t *symbol = NULL;
        const wchar_t *filename = NULL;
        size_t line(static_cast<size_t>(-1ll));

        {
            DWORD64 dw64Displacement = 0;
            if (TRUE == dbghelp.SymFromAddrW()(hProcess, (DWORD64)*address, &dw64Displacement, pSymbol)) {
                symbol = pSymbol->Name;
            }
            else {
                symbol = kUnknown;
            }
        }
        {
            DWORD dwDisplacement = 0;
            if (TRUE == dbghelp.SymGetLineFromAddrW64()(hProcess, (DWORD64)*address, &dwDisplacement, &line64)) {
                filename = line64.FileName;
                line = line64.LineNumber;
            }
            else {
                filename = kUnknown;
                line = static_cast<size_t>(-1ll);
            }
        }

        ::new ((void*)frame) DecodedCallstack::Frame(*address, symbol, filename, line);
    }
#pragma warning( pop )
}
//----------------------------------------------------------------------------
void Callstack::Capture(Callstack* callstack, size_t framesToSkip, size_t framesToCapture) {
    Assert(callstack);
    callstack->_depth = Capture(
        MakeView(&callstack->_frames[0], &callstack->_frames[MaxDepth]),
        &callstack->_hash,
        framesToSkip,
        framesToCapture);
}
//----------------------------------------------------------------------------
size_t Callstack::Capture(
    const MemoryView<void*>& frames,
    size_t* backtraceHash,
    size_t framesToSkip,
    size_t framesToCapture) {
    Assert(frames.size());
    // backtraceHash is optional and can be null
    Assert(framesToSkip + framesToCapture <= 64 /* see RtlCaptureStackBackTrace() */);
    Assert(framesToCapture <= frames.size());

    if (framesToCapture > MaxDepth)
    {
        Assert(false);
        framesToCapture = MaxDepth;
    }

    DWORD rtlHash = 0;
    const WORD rtlDepth = ::RtlCaptureStackBackTrace(
        checked_cast<DWORD>(framesToSkip),
        checked_cast<DWORD>(framesToCapture),
        frames.data(),
        backtraceHash ? &rtlHash : nullptr
        );

    if (backtraceHash)
        *backtraceHash = checked_cast<size_t>(rtlHash);

    return checked_cast<size_t>(rtlDepth);
}
//----------------------------------------------------------------------------
void Callstack::Start() {
    const DbghelpWrapper::Locked dbghelp = DbghelpWrapper::Instance().Lock();

    InitializeSymbols_(dbghelp);
}
//----------------------------------------------------------------------------
void Callstack::ReloadSymbols() {
    const DbghelpWrapper::Locked dbghelp = DbghelpWrapper::Instance().Lock();

    LoadModules_(dbghelp);
}
//----------------------------------------------------------------------------
void Callstack::Shutdown() {
    const DbghelpWrapper::Locked dbghelp = DbghelpWrapper::Instance().Lock();

    HANDLE hProcess = ::GetCurrentProcess();

    dbghelp.SymCleanup()(hProcess);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
