#include "stdafx.h"

#include "Core/IO/Format.h"
#include "Core/IO/StreamProvider.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"

#include "Core/Memory/MemoryStream.h"

namespace Core {
namespace ContentGenerator {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <size_t _Dim, size_t _Dim2, typename... _Args>
static void TestFormat_(const char (&expected)[_Dim], const char (&format)[_Dim2], _Args&&... args) {
    const FString str = StringFormat(format, std::forward<_Args>(args)...);

    const bool match = (expected == str);

    std::cout
        << "==================================================================" << eol
        << "Format      = '" << format << "'" << eol
        << "Result      = '" << str << "'" << eol
        << "Expected    = '" << expected << "' => " << std::boolalpha << match << eol;

    AssertRelease(match);
}
//----------------------------------------------------------------------------
template <size_t _Dim, size_t _Dim2, typename... _Args>
static void TestFormat_(const wchar_t (&expected)[_Dim], const wchar_t (&format)[_Dim2], _Args&&... args) {
    const FWString wstr = StringFormat(format, std::forward<_Args>(args)...);

    const bool match = (expected == wstr);

    std::wcout
        << L"==================================================================" << eol
        << L"Format      = '" << format << L"'" << eol
        << L"Result      = '" << wstr << L"'" << eol
        << L"Expected    = '" << expected << L"' => " << std::boolalpha << match << eol;

    AssertRelease(match);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Format() {
    {
        wchar_t buffer[1024];
        Format(buffer, L"string = {2}, decimal = {0}, float = {1}\n", "test", 42, 0.123456f);
        std::cout << buffer;
    }
    {
        FWString wstr = StringFormat(L"num={0} alphabool={0:a}", true);
        std::wcout << wstr << eol;
    }

    TestFormat_("----------------",
                "{0*16}", '-');
    TestFormat_("000000000000test",
                "{0:#16}", "test");
    TestFormat_("test000000000000",
                "{0:#-16}", "test");
    TestFormat_("0042004200420042",
                "{0:#4*4}", 42);
    TestFormat_("true true true ",
                "{0:A-5*3}", true);
    TestFormat_("string =       test test      , decimal =  BADCAFE 0badcafe, float = -0.123    -0.1235",
                "string = {0:10U} {0:-10U}, decimal = {1:8X} {1:#8x}, float = {2:f3} {2:10f4}", "test", 0xBADCAFE, -0.123456f);
    TestFormat_("This a neg test 0042004200420042 = test aligned                   ( 1.23)",
                "This a neg test {0:#4*4} = {1:-30} ({2:5f2})", 42, "test aligned", 1.23456f);
    TestFormat_(L"This a neg test 0042004200420042 = test aligned                   ( 1.23)",
                L"This a neg test {0:#4*4} = {1:-30} ({2:5f2})", 42, "test aligned", 1.23456f);


    MEMORYSTREAM_THREAD_LOCAL(Stream) mem;
    FStreamWriterOStream oss(&mem);
    oss << "Yolo!" << 42 << eol;
    Format(oss, "This a neg test {0:#4*4} = {1:-30} ({2:5f2})", 42, "test aligned", 1.23456f);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
