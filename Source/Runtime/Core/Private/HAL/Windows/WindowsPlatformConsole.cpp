// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "HAL/Windows/WindowsPlatformConsole.h"

#ifdef PLATFORM_WINDOWS

#include "Diagnostic/Logger.h"
#include "IO/StringView.h"

#include "HAL/Windows/LastError.h"
#include "HAL/Windows/WindowsPlatformIncludes.h"

#include <iostream>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicCStreamBuf_ : public std::basic_streambuf<_Char> {
public:
    TBasicCStreamBuf_(::FILE* hFile)
    :   _hFile(hFile) {
        Assert(_hFile);
    }

protected:
    using typename std::basic_streambuf<_Char>::int_type;
    using typename std::basic_streambuf<_Char>::traits_type;

    virtual int_type underflow() override final;
    virtual int_type overflow(int_type c) override final;

    virtual std::streamsize xsgetn(_Char* p, std::streamsize n) override final {
        return checked_cast<std::streamsize>(::fread(p, sizeof(_Char), checked_cast<size_t>(n), _hFile));
    }

    virtual std::streamsize xsputn(const _Char* p, std::streamsize n) override final {
        return checked_cast<std::streamsize>(::fwrite(p, sizeof(_Char), checked_cast<size_t>(n), _hFile));
    }

private:
    ::FILE* _hFile;
};
//----------------------------------------------------------------------------
template <>
auto TBasicCStreamBuf_<char>::underflow() -> int_type {
    const int c = ::fgetc(_hFile);
    return (c == EOF ? traits_type::eof() : c);
}
template <>
auto TBasicCStreamBuf_<char>::overflow(int_type c) -> int_type {
    return (::fputc(c, _hFile) == EOF ? traits_type::eof() : c);
}
//----------------------------------------------------------------------------
template <>
auto TBasicCStreamBuf_<wchar_t>::underflow() -> int_type {
    const wint_t c = ::fgetwc(_hFile);
    return (c == WEOF ? traits_type::eof() : int_type(c));
}
template <>
auto TBasicCStreamBuf_<wchar_t>::overflow(int_type c) -> int_type {
    return (::fputwc(checked_cast<wchar_t>(c), _hFile) == WEOF ? traits_type::eof() : c);
}
//----------------------------------------------------------------------------
template <typename _Char>
struct TBasicCStreamBind_ {
    TBasicCStreamBuf_<_Char> Mapped;
    std::basic_ios<_Char>* Stream;
    std::basic_streambuf<_Char>* Original;

    TBasicCStreamBind_(::FILE* hFile)
        : Mapped(hFile)
        , Stream(nullptr)
        , Original(nullptr)
    {}

    void Map(std::basic_ios<_Char>& stream) {
        Assert(nullptr == Stream);
        Assert(nullptr == Original);

        Stream = &stream;
        Original = Stream->rdbuf(&Mapped);
    }

    void Unmap() {
        Assert(Stream);
        Assert(Original);

        Stream->rdbuf(Original);
        Stream = nullptr;
        Original = nullptr;
    }
};
//----------------------------------------------------------------------------
struct FConsoleWin32_ {
    ::CRITICAL_SECTION BarrierIn;
    ::CRITICAL_SECTION BarrierOut;

    ::HANDLE hConsoleIn = INVALID_HANDLE_VALUE;
    ::HANDLE hConsoleOut = INVALID_HANDLE_VALUE;
    ::WORD Attributes = WORD(-1);
    ::WORD RefCount = 0;

    FILE* hStdin = nullptr;
    FILE* hStdout = nullptr;
    FILE* hStderr = nullptr;

    TBasicCStreamBind_<char> Stdin;
    TBasicCStreamBind_<char> Stdout;
    TBasicCStreamBind_<char> Stderr;

    TBasicCStreamBind_<wchar_t> WStdin;
    TBasicCStreamBind_<wchar_t> WStdout;
    TBasicCStreamBind_<wchar_t> WStderr;

    bool Available() const {
        return (INVALID_HANDLE_VALUE != hConsoleIn &&
                INVALID_HANDLE_VALUE != hConsoleOut );
    }

    void Map() {
        Stdin.Map(std::cin);
        Stdout.Map(std::cout);
        Stderr.Map(std::cerr);

        WStdin.Map(std::wcin);
        WStdout.Map(std::wcout);
        WStderr.Map(std::wcerr);
    }

    void Unmap() {
        Stdin.Unmap();
        Stdout.Unmap();
        Stderr.Unmap();

        WStdin.Unmap();
        WStdout.Unmap();
        WStderr.Unmap();
    }

    static NO_INLINE FConsoleWin32_& Get() {
        ONE_TIME_DEFAULT_INITIALIZE(FConsoleWin32_, GInstance);
        return GInstance;
    }

    struct FReadScope {
        FConsoleWin32_& Console;
        FReadScope() : Console(Get()) {
            ::EnterCriticalSection(&Console.BarrierIn);
        }
        ~FReadScope() {
            ::LeaveCriticalSection(&Console.BarrierIn);
        }
    };

    struct FWriteScope {
        FConsoleWin32_& Console;
        FWriteScope() : Console(Get()) {
            ::EnterCriticalSection(&Console.BarrierOut);
        }
        ~FWriteScope() {
            ::LeaveCriticalSection(&Console.BarrierOut);
        }
    };

    struct FReadWriteScope {
        FConsoleWin32_& Console;
        FReadWriteScope() : Console(Get()) {
            ::EnterCriticalSection(&Console.BarrierIn);
            ::EnterCriticalSection(&Console.BarrierOut);
        }
        ~FReadWriteScope() {
            ::LeaveCriticalSection(&Console.BarrierOut);
            ::LeaveCriticalSection(&Console.BarrierIn);
        }
    };

private:
    FConsoleWin32_()
        : Stdin(stdin)
        , Stdout(stdout)
        , Stderr(stderr)
        , WStdin(stdin)
        , WStdout(stdout)
        , WStderr(stderr) {
        Verify(::InitializeCriticalSectionAndSpinCount(&BarrierIn, 0x00000400));
        Verify(::InitializeCriticalSectionAndSpinCount(&BarrierOut, 0x00000400));
    }

    ~FConsoleWin32_() {
        Assert(0 == RefCount);
        Assert(INVALID_HANDLE_VALUE == hConsoleIn);
        Assert(INVALID_HANDLE_VALUE == hConsoleOut);
        ::DeleteCriticalSection(&BarrierIn);
        ::DeleteCriticalSection(&BarrierOut);
    }
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(FWindowsPlatformConsole::FG_BLUE == FOREGROUND_BLUE);
STATIC_ASSERT(FWindowsPlatformConsole::FG_GREEN == FOREGROUND_GREEN);
STATIC_ASSERT(FWindowsPlatformConsole::FG_RED == FOREGROUND_RED);
STATIC_ASSERT(FWindowsPlatformConsole::FG_INTENSITY == FOREGROUND_INTENSITY);
STATIC_ASSERT(FWindowsPlatformConsole::BG_BLUE == BACKGROUND_BLUE);
STATIC_ASSERT(FWindowsPlatformConsole::BG_GREEN == BACKGROUND_GREEN);
STATIC_ASSERT(FWindowsPlatformConsole::BG_RED == BACKGROUND_RED);
STATIC_ASSERT(FWindowsPlatformConsole::BG_INTENSITY == BACKGROUND_INTENSITY);
//----------------------------------------------------------------------------
bool FWindowsPlatformConsole::Open() {
    const FConsoleWin32_::FReadWriteScope win32;

    if (0 == win32.Console.RefCount) {
        if (not ::AllocConsole()) {
            LOG_LASTERROR(HAL, L"FWindowsPlatformConsole::Open");
            return false;
        }
    }

    win32.Console.RefCount++;

    if (not win32.Console.Available()) {
        win32.Console.hConsoleOut = CreateFileW(L"CONOUT$",  GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        win32.Console.hConsoleIn = CreateFileW(L"CONIN$",  GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (not win32.Console.Available())
            return false;

        DWORD dwReadMode = 0;
        if (::GetConsoleMode(win32.Console.hConsoleIn, &dwReadMode)) {
            if (not ::SetConsoleMode(win32.Console.hConsoleIn, dwReadMode
                        | ENABLE_ECHO_INPUT
                        | ENABLE_LINE_INPUT
                        | ENABLE_PROCESSED_INPUT
                        /*
                        | ENABLE_QUICK_EDIT_MODE
                        | ENABLE_INSERT_MODE
                        | ENABLE_EXTENDED_FLAGS */ )) {
                LOG_LASTERROR(HAL, L"FWindowsPlatformConsole::SetConsoleMode(hConsoleIn)");
                // non-fatal
            }
        }
        else {
            LOG_LASTERROR(HAL, L"FWindowsPlatformConsole::GetConsoleMode(hConsoleIn)");
        }

        DWORD dwWriteMode = 0;
        if (::GetConsoleMode(win32.Console.hConsoleOut, &dwWriteMode)) {
            if (not ::SetConsoleMode(win32.Console.hConsoleOut, dwWriteMode
                        | ENABLE_QUICK_EDIT_MODE
                        | ENABLE_EXTENDED_FLAGS )) {
                LOG_LASTERROR(HAL, L"FWindowsPlatformConsole::SetConsoleMode(hConsoleOut)");
                // non-fatal
            }
        }
        else {
            LOG_LASTERROR(HAL, L"FWindowsPlatformConsole::GetConsoleMode(hConsoleOut)");
        }

        Verify(::SetConsoleCP(CP_UTF8));
        Verify(::SetConsoleOutputCP(CP_UTF8));
        Verify(::SetConsoleTitleW(L"PPE - " WSTRINGIZE(BUILD_FAMILY)));

        // redirect CRT standard input, output and error handles to the console window
        Verify(::freopen_s(&win32.Console.hStdin, "CONIN$", "r", stdin) == 0);
        Verify(::freopen_s(&win32.Console.hStdout, "CONOUT$", "w", stdout) == 0);
        Verify(::freopen_s(&win32.Console.hStderr, "CONOUT$", "w", stderr) == 0);

        win32.Console.Map();
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------d
void FWindowsPlatformConsole::Close() {
    const FConsoleWin32_::FReadWriteScope win32;

    if (win32.Console.RefCount && 0 == --win32.Console.RefCount) {
        if (not ::FreeConsole()) {
            LOG_LASTERROR(HAL, L"FWindowsPlatformConsole::Close");
            AssertNotReached();
        }

        if (win32.Console.Available()) {
            win32.Console.Unmap();

            Verify(::fclose(win32.Console.hStdin) == 0);
            Verify(::fclose(win32.Console.hStdout) == 0);
            Verify(::fclose(win32.Console.hStderr) == 0);

            win32.Console.hStdin = nullptr;
            win32.Console.hStdout = nullptr;
            win32.Console.hStderr = nullptr;
        }

        win32.Console.hConsoleIn = INVALID_HANDLE_VALUE;
        win32.Console.hConsoleOut = INVALID_HANDLE_VALUE;
    }
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformConsole::Read(const TMemoryView<char>& buffer) {
    Assert(not buffer.empty());

    const FConsoleWin32_::FReadScope win32;
    Assert(win32.Console.RefCount);

    ::DWORD read = 0;
    if (win32.Console.Available()) {
        Assert(win32.Console.hConsoleIn);

        ::FlushConsoleInputBuffer(win32.Console.hConsoleIn);

        if (not ::ReadConsoleA(win32.Console.hConsoleIn, buffer.data(), checked_cast<::DWORD>(buffer.size()), &read, nullptr)) {
            LOG_LASTERROR(HAL, L"FWindowsPlatformConsole::ReadA");
            AssertNotReached();
        }
    }

    return checked_cast<size_t>(read);
}
//----------------------------------------------------------------------------
size_t FWindowsPlatformConsole::Read(const TMemoryView<wchar_t>& buffer) {
    Assert(not buffer.empty());

    const FConsoleWin32_::FReadScope win32;
    Assert(win32.Console.RefCount);

    ::DWORD read = 0;
    if (win32.Console.Available()) {
        Assert(win32.Console.hConsoleIn);

        ::FlushConsoleInputBuffer(win32.Console.hConsoleIn);

        if (not ::ReadConsoleW(win32.Console.hConsoleIn, buffer.data(), checked_cast<::DWORD>(buffer.size()), &read, nullptr)) {
            LOG_LASTERROR(HAL, L"FWindowsPlatformConsole::ReadW");
            AssertNotReached();
        }
    }

    return checked_cast<size_t>(read);
}
//----------------------------------------------------------------------------
void FWindowsPlatformConsole::Write(const FStringView& text, EAttribute attrs/* = Default */) {
    Assert(not text.empty());

    const FConsoleWin32_::FWriteScope win32;
    Assert(win32.Console.RefCount);

    if (win32.Console.Available()) {
        Assert(win32.Console.hConsoleOut);

        if (attrs != win32.Console.Attributes) {
            win32.Console.Attributes = ::WORD(attrs);
            ::SetConsoleTextAttribute(win32.Console.hConsoleOut, win32.Console.Attributes);
        }

        ::DWORD written = checked_cast<::DWORD>(text.size());
        if (not ::WriteConsoleA(win32.Console.hConsoleOut, text.data(), written, &written, nullptr)) {
            LOG_LASTERROR(HAL, L"FWindowsPlatformConsole::WriteA");
            AssertNotReached();
        }
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformConsole::Write(const FWStringView& text, EAttribute attrs/* = Default */) {
    Assert(not text.empty());

    const FConsoleWin32_::FWriteScope win32;
    Assert(win32.Console.RefCount);

    if (win32.Console.Available()) {
        Assert(win32.Console.hConsoleOut);

        if (attrs != win32.Console.Attributes) {
            win32.Console.Attributes = ::WORD(attrs);
            ::SetConsoleTextAttribute(win32.Console.hConsoleOut, win32.Console.Attributes);
        }

        ::DWORD written = checked_cast<::DWORD>(text.size());
        if (not ::WriteConsoleW(win32.Console.hConsoleOut, text.data(), written, &written, nullptr)) {
            LOG_LASTERROR(HAL, L"FWindowsPlatformConsole::WriteW");
            AssertNotReached();
        }
    }
}
//----------------------------------------------------------------------------
void FWindowsPlatformConsole::Flush() {
    const FConsoleWin32_::FWriteScope win32;
    Assert(win32.Console.RefCount);
    ::FlushProcessWriteBuffers();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!PLATFORM_WINDOWS
