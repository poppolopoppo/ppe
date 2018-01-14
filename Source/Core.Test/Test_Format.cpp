#include "stdafx.h"

#include "Core/Diagnostic/Logger.h"
#include "Core/IO/BufferedStream.h"
#include "Core/IO/FileStream.h"
#include "Core/IO/Format.h"
#include "Core/IO/StreamProvider.h"
#include "Core/IO/String.h"
#include "Core/IO/StringBuilder.h"
#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter.h"

#include "Core/Memory/MemoryStream.h"

namespace Core {
namespace Test {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <size_t _Dim, size_t _Dim2, typename... _Args>
static void TestFormat_(FTextWriter& oss, const char (&expected)[_Dim], const char (&format)[_Dim2], _Args&&... args) {
    const FString str = StringFormat(format, std::forward<_Args>(args)...);

    const bool match = (expected == str);

    oss
        << "==================================================================" << Eol
        << "Format      = '" << format << "'" << Eol
        << "Result      = '" << str << "'" << Eol
        << "Expected    = '" << expected << "' => " << FTextFormat::BoolAlpha << match << Eol;

    AssertRelease(match);
}
//----------------------------------------------------------------------------
template <size_t _Dim, size_t _Dim2, typename... _Args>
static void TestFormat_(FWTextWriter& oss, const wchar_t (&expected)[_Dim], const wchar_t (&format)[_Dim2], _Args&&... args) {
    const FWString wstr = StringFormat(format, std::forward<_Args>(args)...);

    const bool match = (expected == wstr);

    oss
        << L"==================================================================" << Eol
        << L"Format      = '" << format << L"'" << Eol
        << L"Result      = '" << wstr << L"'" << Eol
        << L"Expected    = '" << expected << L"' => " << FTextFormat::BoolAlpha << match << Eol;

    AssertRelease(match);
}
//----------------------------------------------------------------------------
static void Test_Format_() {
    {
        wchar_t buffer[1024];
        Format(buffer, L"string = {2}, decimal = {0}, float = {1}\n", "test", 42, 0.123456f);
        GStdout << buffer;
    }
    {
        FWString wstr = StringFormat(L"num={0} alphabool={0:a}", true);
        GStdout << wstr << Eol;
    }

    TestFormat_(GStdout, "----------------",
        "{0*16}", '-');
    TestFormat_(GStdout, "000000000000test",
        "{0:#16}", "test");
    TestFormat_(GStdout, "test000000000000",
        "{0:#-16}", "test");
    TestFormat_(GStdout, "0042004200420042",
        "{0:#4*4}", 42);
    TestFormat_(GStdout, "true true true ",
        "{0:A-5*3}", true);
    TestFormat_(GStdout, "string =       test test      , decimal =  BADCAFE 0badcafe, float = -0.123    -0.1235",
        "string = {0:10U} {0:-10U}, decimal = {1:8X} {1:#8x}, float = {2:f3} {2:10f4}", "test", 0xBADCAFE, -0.123456f);
    TestFormat_(GStdout, "This a neg test 0042004200420042 = test aligned                   ( 1.23)",
        "This a neg test {0:#4*4} = {1:-30} ({2:5f2})", 42, "test aligned", 1.23456f);
    TestFormat_(GStdout, "This a neg test 0042004200420042 = test aligned                   ( 1.23)",
        "This a neg test {0:#4*4} = {1:-30} ({2:5f2})", 42, "test aligned", 1.23456f);

    FStringBuilder oss;
    oss << "Yolo!" << 42 << Eol;
    Format(oss, "This a neg test {0:#4*4} = {1:-30} ({2:5f2})", 42, "test aligned", 1.23456f);

    GStdout << oss.ToString() << Endl;
}
//----------------------------------------------------------------------------
static void Test_TextWriter_Ansi_() {
    const void* heap = (void*)&Test_TextWriter_Ansi_;
    FTextWriter oss(FFileStream::StdoutWriter());
    oss << "un lama sachant chasser doit savoir chasser sans son canard" << Eol
        << FTextFormat::BoolAlpha << true << Eol
        >> FTextFormat::BoolAlpha << true << Eol
        << 42 << Eol
        << 1.23f << Eol
        << 1.23456789 << Eol
        << -123456.789 << Eol
        << 1e-7 << Eol
        << 123.14159265359f << Eol
        << 123.14159265358979323846 << Eol
        << FTextFormat::FixedFloat
        << 123.14159265359f << Eol
        << 123.14159265358979323846 << Eol
        << FTextFormat::PrecisionFloat
        << 123.14159265359f << Eol
        << 123.14159265358979323846 << Eol
        << FTextFormat::ScientificFloat
        << 123.14159265359f << Eol
        << 123.14159265358979323846 << Eol;
    oss << (const void*)&oss << Eol;
    oss << heap << Eol;
    oss << '<'
        << FTextFormat::PadCenter(77, '-')
        << FTextFormat::Uppercase
        << " Centered Uppercase "
        << '>' << Endl;
    oss << FTextFormat::Capitalize
        << "caPItaliZe tHIS tEXt pleAsE r2-d2, you're such a sweetheart !" << Endl;
}
//----------------------------------------------------------------------------
static void Test_TextWriter_Wide_() {
    const void* heap = (void*)&Test_TextWriter_Wide_;
    FWTextWriter oss(FFileStream::StdoutWriter());
    oss << L"un lama sachant chasser doit savoir chasser sans son canard" << Eol
        << FTextFormat::BoolAlpha << true << Eol
        >> FTextFormat::BoolAlpha << true << Eol
        << 42 << Eol
        << 1.23f << Eol
        << 1.23456789 << Eol
        << -123456.789 << Eol
        << 1e-7 << Eol
        << 123.14159265359f << Eol
        << 123.14159265358979323846 << Eol
        << FTextFormat::FixedFloat
        << 123.14159265359f << Eol
        << 123.14159265358979323846 << Eol
        << FTextFormat::PrecisionFloat
        << 123.14159265359f << Eol
        << 123.14159265358979323846 << Eol
        << FTextFormat::ScientificFloat
        << 123.14159265359f << Eol
        << 123.14159265358979323846 << Eol;
    oss << (const void*)&oss << Eol;
    oss << heap << Eol;
    oss << L'<'
        << FTextFormat::PadCenter(77, L'-')
        << FTextFormat::Uppercase
        << L" Centered Uppercase "
        << L'>' << Endl;
    oss << FTextFormat::Capitalize
        << L"caPItaliZe tHIS tEXt pleAsE r2-d2, you're such a sweetheart !" << Endl;
}
//----------------------------------------------------------------------------
static void Test_TextWriter_() {
    Test_TextWriter_Ansi_();
    Test_TextWriter_Wide_();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Format() {
    Test_Format_();
    Test_TextWriter_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace Core
