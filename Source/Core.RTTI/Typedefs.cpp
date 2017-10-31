#include "stdafx.h"

#include "Typedefs.h"

#include "Core/IO/Format.h"
#include "Core/IO/StringView.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
std::basic_ostream<char>& operator <<(std::basic_ostream<char>& oss, const RTTI::FBinaryData& bindata) {
    oss << "FBinaryData:\"";

    for (char ch : bindata.MakeConstView().Cast<const char>()) {
        switch (ch) {
        case '\\':
        case '"':
        case '/':
            oss << '\\' << ch;
            break;
        case '\b':
            oss << "\\b";
            break;
        case '\t':
            oss << "\\t";
            break;
        case '\n':
            oss << "\\n";
            break;
        case '\f':
            oss << "\\f";
            break;
        case '\r':
            oss << "\\r";
            break;
        default:
            if (IsPrint(ch))
                oss << ch;
            else
                Format(oss, "\\x{0:#2x}", u8(ch));
        }
    }

    return oss << "\"";
}
//----------------------------------------------------------------------------
std::basic_ostream<wchar_t>& operator <<(std::basic_ostream<wchar_t>& oss, const RTTI::FBinaryData& bindata) {
    oss << L"FBinaryData:\"";

    for (wchar_t wch : bindata.MakeConstView().Cast<const wchar_t>()) {
        switch (wch) {
        case L'\\':
        case L'"':
        case L'/':
            oss << L'\\' << wch;
            break;
        case L'\b':
            oss << L"\\b";
            break;
        case L'\t':
            oss << L"\\t";
            break;
        case L'\n':
            oss << L"\\n";
            break;
        case L'\f':
            oss << L"\\f";
            break;
        case L'\r':
            oss << L"\\r";
            break;
        default:
            if (IsPrint(wch))
                oss << wch;
            else
                Format(oss, L"\\u{0:#4x}", u16(wch));
        }
    }

    return oss << L"\"";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
