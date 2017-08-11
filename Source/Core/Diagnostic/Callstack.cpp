#include "stdafx.h"

#include "Callstack.h"

#include "DecodedCallstack.h"
#include "LastError.h"
#include "Logger.h"

#include "IO/Stream.h"
#include "IO/String.h"
#include "IO/StringView.h"

#include <mutex>

#ifdef PLATFORM_WINDOWS
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

    FWOCStrStream oss{ out_symbol_path, checked_cast<std::streamsize>(max_length) };
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
            FWOCStrStream tmp{ webSymbolsPath, checked_cast<std::streamsize>(lengthof(webSymbolsPath)) };
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
static void LoadModules_(const FDbghelpWrapper::FLocked& dbghelp) {
    HANDLE  process = ::GetCurrentProcess();
    DWORD   process_id = ::GetCurrentProcessId();
    HANDLE  snap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process_id);

    MODULEENTRY32 module_entry;
    module_entry.dwSize = sizeof(MODULEENTRY32);

    if (snap == (HANDLE)-1)
        return;

#pragma warning( push )
#pragma warning( disable : 4826 ) // warning C4826: La conversion de 'unsigned char *const ' en 'DWORD64' est de type signe �tendu.
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

        LOG(Info, L"[PDB] {0} for \"{1}\"",
            succeed ? L"Loaded" : L"Failed to load",
            module_entry.szExePath);
        UNUSED(succeed);

        module_found = ::Module32Next(snap, &module_entry);
    }

    ::CloseHandle(snap);
}
//----------------------------------------------------------------------------
static void InitializeSymbols_(const FDbghelpWrapper::FLocked& dbghelp) {
    DWORD options = dbghelp.SymGetOptions()();
    options |= SYMOPT_LOAD_LINES;
    options |= SYMOPT_FAIL_CRITICAL_ERRORS;
    options |= SYMOPT_DEFERRED_LOADS;
    options |= SYMOPT_UNDNAME;
    dbghelp.SymSetOptions()(options);

    // Force standard malloc, this is called at a very early stage for custom allocators
    STATIC_CONST_INTEGRAL(size_t, SYMBOL_PATH_CAPACITY, 8 * MAX_PATH);
    wchar_t symbol_path[SYMBOL_PATH_CAPACITY] = { 0 };

    GetSymbolsPath_(symbol_path, SYMBOL_PATH_CAPACITY);

    HANDLE process = ::GetCurrentProcess();
    BOOL succeed = dbghelp.SymInitializeW()(process, symbol_path, FALSE);

    LOG(Info, L"[PDB] Path = '{0}' -> succeed = {1:A}", symbol_path, (FALSE != succeed));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCallstack::FCallstack()
: _hash(0), _depth(0) {
#ifdef WITH_CORE_ASSERT
    ::memset(_frames, 0xCD, sizeof(_frames));
#endif
}
//----------------------------------------------------------------------------
FCallstack::FCallstack(size_t framesToSkip, size_t framesToCapture)
: FCallstack() {
    Capture(this, framesToSkip, framesToCapture);
}
//----------------------------------------------------------------------------
FCallstack::~FCallstack() {}
//----------------------------------------------------------------------------
FCallstack::FCallstack(const FCallstack& other)
: _hash(other._hash), _depth(other._depth) {
    memcpy(_frames, other._frames, sizeof(_frames));
}
//----------------------------------------------------------------------------
FCallstack& FCallstack::operator =(const FCallstack& other) {
    if (this != &other) {
        _hash = other._hash;
        _depth = other._depth;
        memcpy(_frames, other._frames, sizeof(_frames));
    }
    return *this;
}
//----------------------------------------------------------------------------
void FCallstack::Decode(FDecodedCallstack* decoded) const {
    Decode(decoded, _hash, Frames());
}
//----------------------------------------------------------------------------
void FCallstack::Decode(FDecodedCallstack* decoded, size_t hash, const TMemoryView<void* const>& frames) {
    Assert(decoded);

    const auto dbghelp(FDbghelpWrapper::Instance().Lock());

    LoadModules_(dbghelp);

    static const wchar_t* kUnknown = L"????????";

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
#pragma warning( disable : 4826 ) // warning C4826: La conversion de 'unsigned char *const ' en 'DWORD64' est de type signe �tendu.
    void* const* address = frames.data();
    auto frame = reinterpret_cast<FDecodedCallstack::FFrame *>(&decoded->_frames);
    for (size_t i = 0; i < frames.size(); ++i, ++frame, ++address) {
        FWString symbol;
        FWString filename;
        size_t line(static_cast<size_t>(-1ll));

        {
            DWORD64 dw64Displacement = 0;
            if (TRUE == dbghelp.SymFromAddrW()(hProcess, (DWORD64)*address, &dw64Displacement, pSymbol)) {
                symbol = pSymbol->Name;
            }
            else {
                const long lastErrorCode = ::GetLastError();
                symbol = GetLastErrorToWString(lastErrorCode);
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
                line = 0;
            }
        }

        ::new ((void*)frame) FDecodedCallstack::FFrame(*address, std::move(symbol), std::move(filename), line);
    }
#pragma warning( pop )
}
//----------------------------------------------------------------------------
void FCallstack::Capture(FCallstack* callstack, size_t framesToSkip, size_t framesToCapture) {
    Assert(callstack);
    callstack->_depth = Capture(
        MakeView(&callstack->_frames[0], &callstack->_frames[MaxDepth]),
        &callstack->_hash,
        framesToSkip,
        framesToCapture);
}
//----------------------------------------------------------------------------
size_t FCallstack::Capture(
    const TMemoryView<void*>& frames,
    size_t* backtraceHash,
    size_t framesToSkip,
    size_t framesToCapture) {
    Assert(frames.size());
    // backtraceHash is optional and can be null
    Assert(framesToSkip + framesToCapture <= 64 /* see RtlCaptureStackBackTrace() */);
    Assert(framesToCapture <= frames.size());

    if (framesToCapture > MaxDepth) {
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
void FCallstack::SetFrames(const TMemoryView<void* const>& frames) {
    _hash = 0;
    const size_t n = Min(MaxDepth, frames.size());
    for (_depth = 0; _depth < n; ++_depth) {
        if (nullptr == frames[_depth]) break;
        _frames[_depth] = frames[_depth];
    }
}
//----------------------------------------------------------------------------
void FCallstack::Start() {
    const FDbghelpWrapper& dbghelp = FDbghelpWrapper::Instance();
    if (dbghelp.Available())
        InitializeSymbols_(dbghelp.Lock());
}
//----------------------------------------------------------------------------
void FCallstack::ReloadSymbols() {
    const FDbghelpWrapper& dbghelp = FDbghelpWrapper::Instance();
    if (dbghelp.Available())
        LoadModules_(dbghelp.Lock());
}
//----------------------------------------------------------------------------
void FCallstack::Shutdown() {
    const FDbghelpWrapper& dbghelp = FDbghelpWrapper::Instance();
    if (dbghelp.Available()) {
        HANDLE hProcess = ::GetCurrentProcess();
        dbghelp.Lock().SymCleanup()(hProcess);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
