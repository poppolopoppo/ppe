// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "VirtualFileSystem_fwd.h"

#include "Container/Tuple.h"
#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "IO/ConstChar.h"
#include "IO/FileStream.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/regexp.h"
#include "IO/StreamProvider.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "IO/StringConversion.h"
#include "IO/StringView.h"
#include "IO/TextReader.h"
#include "IO/TextWriter.h"
#include "Maths/RandomGenerator.h"

#include "Memory/MemoryStream.h"

namespace PPE {
namespace Test {
LOG_CATEGORY(, Test_Format)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename... _Args>
static void TestFormat_(const FStringView& expected, const FStringView& format, _Args... args) {
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
        "{0:#16}", MakeStringView("test"));
    TestFormat_("test000000000000",
        "{0:#-16}", MakeStringView("test"));
    TestFormat_("0042004200420042",
        "{0:#4*4}", 42);
    TestFormat_("true true true ",
        "{0:a-5*3}", true);
    TestFormat_("string =       TEST TEST      , decimal =  BADCAFE 0badcafe, float = -0.123    -0.1235",
        "string = {0:10U} {0:-10U}, decimal = {1:8X} {1:#8x}, float = {2:f3} {2:10f4}", MakeStringView("test"), 0xBADCAFE, -0.123456f);
    TestFormat_("This a neg test 0042004200420042 = test aligned                   ( 1.23)",
        "This a neg test {0:#4*4} = {1:-30} ({2:5f2})", 42, MakeStringView("test aligned"), 1.23456f);
    TestFormat_("000000000000test",
        "{0:#16}", "test");
    TestFormat_("test000000000000",
        "{0:#-16}", "test");
    TestFormat_("1",
        "{0:/1}", "123456789");
    TestFormat_("1234",
        "{0:/4}", "123456789");
    TestFormat_("1234567",
        "{0:/7}", "123456789");
    TestFormat_("123456789",
        "{0:/9}", "123456789");
    TestFormat_("                       123456789",
        "{0:/32}", "123456789");
    TestFormat_("123456789                       ",
        "{0:/-32}", "123456789");
}
//----------------------------------------------------------------------------
template <typename _Char>
static void Test_TextReader_Impl_() {
    FMemoryViewReader src(MakeStringView(STRING_LITERAL(_Char,
        "f 123/2707/-4613243\n"
        "vt 0.972642 0.024023\n"
        " \t   vt \t0.950977   -0.024023  \n"
        "done\n")));
    TBasicTextReader<_Char> r(&src);

    TBasicString<_Char> str;
    VerifyRelease(r >> &str);
    AssertRelease(str == STRING_LITERAL(_Char, "f"));

    i8 i0; u16 i1; i32 i2;
    VerifyRelease(r >> &i0);
    AssertRelease(i0 == 123);
    VerifyRelease(r.Expect(STRING_LITERAL(_Char, '/')));
    VerifyRelease(r >> &i1);
    AssertRelease(i1 == 2707);
    VerifyRelease(r.Expect(STRING_LITERAL(_Char, '/')));
    VerifyRelease(r >> &i2);
    AssertRelease(i2 == -4613243);

    VerifyRelease(r >> &str);
    AssertRelease(str == STRING_LITERAL(_Char, "vt"));

    float f0; double f1;
    VerifyRelease(r >> &f0);
    AssertRelease(f0 == 0.972642f);
    VerifyRelease(r >> &f1);
    AssertRelease(f1 == 0.024023);

    VerifyRelease(r >> &str);
    AssertRelease(str == STRING_LITERAL(_Char, "vt"));

    VerifyRelease(r >> &f0);
    AssertRelease(f0 == 0.950977f);
    VerifyRelease(r >> &f1);
    AssertRelease(f1 == -0.024023);

    VerifyRelease(r >> &str);
    AssertRelease(str == STRING_LITERAL(_Char, "done"));

    r.SkipSpaces();

    AssertRelease(r.Eof());
}
static void Test_TextReader_() {
    Test_TextReader_Impl_<char>();
    Test_TextReader_Impl_<wchar_t>();
}
//----------------------------------------------------------------------------
template <typename _Char>
static void Test_TextWriter_Impl_() {
    const void* heap = (void*)&Test_TextWriter_Impl_<_Char>;
    TBasicStringBuilder<_Char> oss;
    oss << STRING_LITERAL(_Char, "un lama sachant chasser doit savoir chasser sans son canard") << Eol
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
    oss << STRING_LITERAL(_Char, '<')
        << FTextFormat::PadCenter(77, L'-')
        << FTextFormat::Uppercase
        << STRING_LITERAL(_Char, " Centered Uppercase ")
        << STRING_LITERAL(_Char, '>') << Endl;
    oss << FTextFormat::Capitalize
        << STRING_LITERAL(_Char, "caPItaliZe tHIS tEXt pleAsE r2-d2, you're such a sweetheart !") << Endl;
    LOG(Test_Format, Info, L"{1} writer: {0}", oss.ToString(), MakeCStringView(typeid(_Char).name()));
}
static void Test_TextWriter_() {
    Test_TextWriter_Impl_<char>();
    Test_TextWriter_Impl_<wchar_t>();
}
//----------------------------------------------------------------------------
template <typename _DstChar, typename _SrcChar>
static void Test_StringEscaping_(TBasicStringView<_SrcChar> input, EEscape escape) {
    TBasicStringBuilder<_DstChar> escaped;
    Escape(escaped, input, escape);
    TBasicString<_DstChar> tmp{ escaped.ToString() };

    TBasicStringBuilder<_SrcChar> unescaped;
    Unescape(unescaped, tmp);
    TBasicString<_SrcChar> output{ unescaped.ToString() };

    LOG(Test_Format, Info, L"Src: {0}\n{1}", Fmt::Quoted(input, L'"'), Fmt::HexDump(input.MakeView()) );
    LOG(Test_Format, Info, L"Dst: {0}\n{1}", Fmt::Quoted(output, L'"'), Fmt::HexDump(output.MakeView()) );
    AssertRelease(output == input);
}
static void Test_StringEscaping_() {
    Test_StringEscaping_<char>(MakeStringView("This\t\x10\nescaPed\23\b\r\n\"123"), EEscape::Octal);
    Test_StringEscaping_<char>(MakeStringView("This\t\x10\nescaPed\23\b\r\n\"123"), EEscape::Hexadecimal);
    Test_StringEscaping_<wchar_t>(MakeStringView(L"This\t\x10\nescaPed\23\b\r\n\"123\u1FA4unicode"), EEscape::Octal);
    Test_StringEscaping_<wchar_t>(MakeStringView(L"This\t\x10\nescaPed\23\b\r\n\"123\u1FA4unicode"), EEscape::Hexadecimal);
    Test_StringEscaping_<wchar_t>(MakeStringView(L"This\t\x10\nescaPed\23\b\r\n\"123\u1FA4unicode"), EEscape::Unicode);
}
//----------------------------------------------------------------------------
static void Test_LogPrintf_() {
    LOG_PRINTF(Test_Format, Info, L"Printf testing %d", 42);
    LOG_PRINTF(Test_Format, Info, L"Printf testing %s", "42");
}
//----------------------------------------------------------------------------
static void Test_Base64_() {
    FRandomGenerator rnd;
    forrange(i, 0, 10) {
        FRawStorage src;
        src.Resize_DiscardData(rnd.Next(8, 100*(i+1)));
        for (u8& raw : src)
            rnd.Randomize(raw);

        TRawStorage<char> dst;
        dst.Resize_DiscardData(Base64EncodeSize(src.MakeView()));
        Base64Encode(src.MakeView(), dst.MakeView());

        FRawStorage chk;
        chk.Resize_DiscardData(Base64DecodeSize(dst.MakeView()));
        AssertRelease(chk.SizeInBytes() == src.SizeInBytes());
        VerifyRelease(Base64Decode(dst.MakeView(), chk.MakeView()));

        AssertRelease(0 == FPlatformMemory::Memcmp(src.data(), chk.data(), src.SizeInBytes()));
    }
}
//----------------------------------------------------------------------------
static void Test_Conversion_() {
    bool b;
    VerifyRelease( FStringConversion{ "true" } >> &b );
    AssertRelease( b );
    VerifyRelease( FStringConversion{ "1" } >> &b );
    AssertRelease( b );
    VerifyRelease( FStringConversion{ "false" } >> &b );
    AssertRelease( not b );
    VerifyRelease( FStringConversion{ "0" } >> &b );
    AssertRelease( not b );
    int i;
    VerifyRelease( FStringConversion{ "123456" } >> &i );
    AssertRelease( i == 123456 );
    VerifyRelease( FStringConversion{ "-420314152" } >> &i );
    AssertRelease( i == -420314152 );
    float f;
    VerifyRelease( FStringConversion{ "2.123456" } >> &f );
    AssertRelease( f == 2.123456f );
    VerifyRelease( FStringConversion{ "-120.3" } >> &f );
    AssertRelease( f == -120.3f );
    double d;
    VerifyRelease( FStringConversion{ "2.123456789" } >> &d );
    AssertRelease( d == 2.123456789 );
    VerifyRelease( FStringConversion{ "-1420.314156" } >> &d );
    AssertRelease( d == -1420.314156 );
}
//----------------------------------------------------------------------------
static void Test_Regexp_() {
    FRegexp re;
    re.Compile(R"(int:([+-]?\d+),\s+float:([+-]?[0-9]*[.]?[0-9]+),\s+word:(\w+)\s+end)", ECase::Sensitive);
    FRegexp reI;
    reI.Compile(R"(int:([+-]?\d+),\s+float:([+-]?[0-9]*[.]?[0-9]+),\s+word:(\w+)\s+end)", ECase::Insensitive);

    FRegexp::FMatches matches;
    VerifyRelease(re.Capture(&matches, "int:42, float:123.456, word:PoPpolLoPpOpo42 end"));

    LOG(Test_Format, Info, L"int:{0}, float:{1}, word:{2} end",
        FStringConversion(matches[1]).ConvertTo<int>(),
        FStringConversion(matches[2]).ConvertTo<float>(),
        FStringView(matches[3]) );

    VerifyRelease(reI.Capture(&matches, "iNt:42, FLoat:123.456, WORD:PoPpolLoPpOpo42 End"));

    LOG(Test_Format, Info, L"int:{0}, float:{1}, word:{2} end",
        FStringConversion(matches[1]).ConvertTo<int>(),
        FStringConversion(matches[2]).ConvertTo<float>(),
        FStringView(matches[3]) );

    int parseInt{};
    float parseFloat{};
    FStringView parseWord{};
    VerifyRelease(not re.Capture("INT:-69,   floaT:-0.1,     Word:Tornado\tend",
        &parseInt, &parseFloat, &parseWord) );
    VerifyRelease(reI.Capture("INT:-69,   floaT:-0.1,     Word:Tornado\tend",
        &parseInt, &parseFloat, &parseWord) );

    TTuple<int, float, FStringView> parseArgs;
    VerifyRelease(reI.Capture(&parseArgs, "INT:-69,   floaT:-0.1,     Word:Tornado\tend"));

    LOG(Test_Format, Info, L"int:{0}, float:{1}, word:{2} end",
        std::get<0>(parseArgs), std::get<1>(parseArgs), std::get<2>(parseArgs) );
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Test_Format() {
    PPE_DEBUG_NAMEDSCOPE("Test_Format");

    Test_Base64_();
    Test_Conversion_();
    Test_Regexp_();
    Test_TextReader_();
    Test_TextWriter_();
    Test_Format_();
    Test_StringEscaping_();
    Test_LogPrintf_();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Test
} //!namespace PPE
