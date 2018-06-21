#include "stdafx.h"

#include "Console.h"

#ifdef PLATFORM_WINDOWS

#include "LastError.h"
#include "Logger.h"

#include "Misc/Platform_Windows.h"

#include <iostream>

namespace Core {
LOG_CATEGORY(, Console);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename _Char>
class TBasicCStreamBuf_ : public std::basic_streambuf<_Char> {
public:
	TBasicCStreamBuf_(::FILE* hFile) 
	:	_hFile(hFile) {
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
	::CRITICAL_SECTION Barrier;
	::HANDLE hConsoleIn = nullptr;
	::HANDLE hConsoleOut = nullptr;
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

	struct FScope {
		FConsoleWin32_& Console;
		FScope() : Console(Get()) { 
			::EnterCriticalSection(&Console.Barrier); 
		}
		~FScope() { 
			::LeaveCriticalSection(&Console.Barrier); 
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
		::InitializeCriticalSectionAndSpinCount(&Barrier, 0x00000400);
	}

	~FConsoleWin32_() {
		Assert(0 == RefCount);
		Assert(nullptr == hConsoleIn);
		Assert(nullptr == hConsoleOut);
		::DeleteCriticalSection(&Barrier);
	}
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(FConsole::FG_BLUE == FOREGROUND_BLUE);
STATIC_ASSERT(FConsole::FG_GREEN == FOREGROUND_GREEN);
STATIC_ASSERT(FConsole::FG_RED == FOREGROUND_RED);
STATIC_ASSERT(FConsole::FG_INTENSITY == FOREGROUND_INTENSITY);
STATIC_ASSERT(FConsole::BG_BLUE == BACKGROUND_BLUE);
STATIC_ASSERT(FConsole::BG_GREEN == BACKGROUND_GREEN);
STATIC_ASSERT(FConsole::BG_RED == BACKGROUND_RED);
STATIC_ASSERT(FConsole::BG_INTENSITY == BACKGROUND_INTENSITY);
//----------------------------------------------------------------------------
void FConsole::Open() {
	const FConsoleWin32_::FScope win32;
	
	if (0 == win32.Console.RefCount++) {
		if (not ::AllocConsole()) {
			LOG_LASTERROR(Console, L"FConsole::AllocConsole");
			AssertNotReached();
		}

		win32.Console.hConsoleIn = ::GetStdHandle(STD_INPUT_HANDLE);
		win32.Console.hConsoleOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
		
		::SetConsoleOutputCP(CP_UTF8);
		::SetConsoleTitleW(L"Core - " WIDESTRING(STRINGIZE(BUILDCONFIG)));
		::SetConsoleMode(win32.Console.hConsoleIn, ENABLE_QUICK_EDIT_MODE | ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_EXTENDED_FLAGS);
		::SetConsoleMode(win32.Console.hConsoleOut, ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS);

		// redirect CRT standard input, output and error handles to the console window
		::freopen_s(&win32.Console.hStdin, "CONIN$", "r", stdin);
		::freopen_s(&win32.Console.hStdout, "CONOUT$", "w", stdout);
		::freopen_s(&win32.Console.hStderr, "CONOUT$", "w", stderr);

		win32.Console.Map();
	}
}
//----------------------------------------------------------------------------
void FConsole::Close() {
	const FConsoleWin32_::FScope win32;

	Assert(win32.Console.RefCount > 0);
	if (0 == --win32.Console.RefCount) {
		win32.Console.Unmap();

		if (not ::FreeConsole()) {
			LOG_LASTERROR(Console, L"FConsole::FreeConsole");
			AssertNotReached();
		}

		::fclose(win32.Console.hStdin);
		::fclose(win32.Console.hStdout);
		::fclose(win32.Console.hStderr);

		win32.Console.hStdin = nullptr;
		win32.Console.hStdout = nullptr;
		win32.Console.hStderr = nullptr;

		win32.Console.hConsoleIn = nullptr;
		win32.Console.hConsoleOut = nullptr;
	}
}
//----------------------------------------------------------------------------
size_t FConsole::Read(const TMemoryView<char>& buffer) {
	Assert(not buffer.empty());

	const FConsoleWin32_::FScope win32;

	Assert(win32.Console.RefCount);
	Assert(win32.Console.hConsoleIn);

	::FlushConsoleInputBuffer(win32.Console.hConsoleIn);

	::DWORD read = checked_cast<::DWORD>(buffer.size());
	if (not ::ReadConsoleA(win32.Console.hConsoleIn, buffer.data(), read, &read, nullptr)) {
		LOG_LASTERROR(Console, L"FConsole::ReadA");
		AssertNotReached();
	}

	return checked_cast<size_t>(read);
}
//----------------------------------------------------------------------------
size_t FConsole::Read(const TMemoryView<wchar_t>& buffer) {
	Assert(not buffer.empty());

	const FConsoleWin32_::FScope win32;

	Assert(win32.Console.RefCount);
	Assert(win32.Console.hConsoleIn);

	::FlushConsoleInputBuffer(win32.Console.hConsoleIn);

	::DWORD read = checked_cast<::DWORD>(buffer.size());
	if (not ::ReadConsoleW(win32.Console.hConsoleIn, buffer.data(), read, &read, nullptr)) {
		LOG_LASTERROR(Console, L"FConsole::ReadA");
		AssertNotReached();
	}
	
	return checked_cast<size_t>(read);
}
//----------------------------------------------------------------------------
void FConsole::Write(const FStringView& text, EAttribute attrs) {
	Assert(not text.empty());

	const FConsoleWin32_::FScope win32;

	Assert(win32.Console.RefCount);
	Assert(win32.Console.hConsoleOut);

	if (attrs != win32.Console.Attributes) {
		win32.Console.Attributes = ::WORD(attrs);
		::SetConsoleTextAttribute(win32.Console.hConsoleOut, win32.Console.Attributes);
	}

	::DWORD written = checked_cast<::DWORD>(text.size());
	if (not ::WriteConsoleA(win32.Console.hConsoleOut, text.data(), written, &written, nullptr)) {
		LOG_LASTERROR(Console, L"FConsole::WriteA");
		AssertNotReached();
	}
}
//----------------------------------------------------------------------------
void FConsole::Write(const FWStringView& text, EAttribute attrs) {
	Assert(not text.empty());

	const FConsoleWin32_::FScope win32;

	Assert(win32.Console.RefCount);
	Assert(win32.Console.hConsoleOut);

	if (attrs != win32.Console.Attributes) {
		win32.Console.Attributes = ::WORD(attrs);
		::SetConsoleTextAttribute(win32.Console.hConsoleOut, win32.Console.Attributes);
	}

	::DWORD written = checked_cast<::DWORD>(text.size());
	if (not ::WriteConsoleW(win32.Console.hConsoleOut, text.data(), written, &written, nullptr)) {
		LOG_LASTERROR(Console, L"FConsole::WriteW");
		AssertNotReached();
	}
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#else

// TODO

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FConsole::Open() {}
//----------------------------------------------------------------------------
void FConsole::Close() {}
//----------------------------------------------------------------------------
void FConsole::Write(const FWStringView& , EAttribute ) {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#endif