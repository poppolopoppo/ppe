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
LOG_CATEGORY(, Test_Format);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename... _Args>
static void TestFormat_(const FStringView& expected, const FStringView& format, _Args&&... args) {
    const FString str = StringFormat(format, std::forward<_Args>(args)...);

    const bool match = (expected == str);

    LOG(Test_Format, Info,
        L"==================================================================\n"
        L"Format      = '{0}'\n"
        L"Result      = '{1}'\n"
        L"Expected    = '{2}' => {3:A}",
        format, str, expected, match );

    AssertRelease(match);
}
//----------------------------------------------------------------------------
static void Test_Format_() {
    {
        FStringBuilder oss;
        {
            wchar_t buffer[1024];
            Format(buffer, L"string = {0}, decimal = {1}, float = {1}\n\0", L"test", 42, 0.123456f);
            oss << MakeCStringView(buffer);
        }
        {
            FString wstr = StringFormat("num={0} alphabool={0:a}", true);
            oss << wstr << Endl;
        }

        oss << "Yolo!" << 42 << Eol;
        Format(oss, "This a neg test {0:#4*4} = {1:-30} ({2:5f2})", 42, "test aligned", 1.23456f);

        LOG(Test_Format, Info, L"Format: {0}", oss.ToString());
    }

    TestFormat_("722556564 -1465266736 -1246678562 414408744",
        "{0} {1} {2} {3}",
        int(722556564), int(-1465266736), int(-1246678562), int(414408744));
    TestFormat_("-0.002351284 0.0027836573 -0.0017139912 0.0020291738 0.0028858185 -0.0034164863 -0.0035932064 0.004253954",
        "{0} {1} {2} {3} {4} {5} {6} {7}",
        -0.002351284, 0.0027836573, -0.0017139912, 0.0020291738, 0.0028858185, -0.0034164863, -0.0035932064, 0.004253954);
    TestFormat_("----------------",
        "{0:*16}", '-');
    TestFormat_("000000000000test",
        "{0:#16}", "test");
    TestFormat_("test000000000000",
        "{0:#-16}", "test");
    TestFormat_("0042004200420042",
        "{0:#4*4}", 42);
    TestFormat_("true true true ",
        "{0:a-5*3}", true);
    TestFormat_("string =       TEST TEST      , decimal =  BADCAFE 0badcafe, float = -0.123    -0.1235",
        "string = {0:10U} {0:-10U}, decimal = {1:8X} {1:#8x}, float = {2:f3} {2:10f4}", "test", 0xBADCAFE, -0.123456f);
    TestFormat_("This a neg test 0042004200420042 = test aligned                   ( 1.23)",
        "This a neg test {0:#4*4} = {1:-30} ({2:5f2})", 42, "test aligned", 1.23456f);
}
//----------------------------------------------------------------------------
static void Test_TextWriter_Ansi_() {
    const void* heap = (void*)&Test_TextWriter_Ansi_;
    FStringBuilder oss;
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
    LOG(Test_Format, Info, L"ANSI writer: {0}", oss.ToString());
}
//----------------------------------------------------------------------------
static void Test_TextWriter_Wide_() {
    const void* heap = (void*)&Test_TextWriter_Wide_;
    FWStringBuilder oss;
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
    LOG(Test_Format, Info, L"Wide writer: {0}", oss.ToString());
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
