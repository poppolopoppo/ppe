#include "stdafx.h"

#include "Uri.h"

#include "Core/IO/Stream.h"

namespace Core {
namespace Network {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool DontRequireURIEncoding_(char ch) {
    return (IsAlnum(ch) || '-' == ch || '_' == ch || '.' == ch || '~' == ch);
}
//----------------------------------------------------------------------------
static bool FromHex_(char& dst, char src) {
    if (IsDigit(src)) {
        dst = (src - '0');
        return true;
    }
    else if (src <= 'f' && src >= 'a') {
        dst = (src - 'a' + 10);
        return true;
    }
    else if (src <= 'F' && src >= 'A') {
        dst = (src - 'A' + 10);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
static char ToHex_(size_t x) {
    return char(x + (x > 9 ? 'A' - 10 : '0'));
}
//----------------------------------------------------------------------------
static bool UriDecode_(FOStream& oss, const FStringView& str) {
    forrange(i, 0, str.size()) {
        const char ch = str[i];
        if ('+' == ch) {
            oss << ' ';
        }
        else if ('%' == ch  && i + 2 < str.size()) {
            char d0, d1;
            if (not FromHex_(d0, str[i + 1]) ||
                not FromHex_(d1, str[i + 2]) ) {
                return false;
            }
            oss << char(d0 << 4 | d1);
            i += 2;
        }
        else if (DontRequireURIEncoding_(ch)) {
            oss << ch;
        }
        else {
            return false;
        }
    }
    return true;
}
//----------------------------------------------------------------------------
static bool UriEncode_(FOStream& oss, const FStringView& str) {
    for (char ch : str) {
        if (DontRequireURIEncoding_(ch)) {
            oss << ch;
        }
        else if (IsSpace(ch)) {
            oss << '+';
        }
        else {
            const size_t x = (unsigned char)ch;
            oss << '%' << ToHex_(x >> 4) << ToHex_(x & 15);
        }
    }
    return true;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FUri::FUri() : _port(80) {}
//----------------------------------------------------------------------------
FUri::~FUri() {}
//----------------------------------------------------------------------------
bool FUri::Pack(
    FUri& dst,
    const FStringView& scheme,
    const FStringView& username,
    size_t port,
    const FStringView& hostname,
    const FStringView& path,
    const FQueryMap& query,
    const FStringView& anchor ) {
    Assert(scheme.size());
    Assert(hostname.size());

    STACKLOCAL_OCSTRSTREAM(oss, 1024);

    if (scheme.size()) {
        if (not IsAlpha(scheme))
            return false;

        oss << scheme << "://";
    }
    const size_t ischeme = checked_cast<size_t>(oss.size());

    if (username.size()) {
        if (not IsIdentifier(username))
            return false;

        oss << username << '@';
    }
    const size_t iusername = checked_cast<size_t>(oss.size());

    if (not IsIdentifier(hostname))
        return false;
    oss << hostname;
    const size_t ihostname = checked_cast<size_t>(oss.size());

    if (port != 0) {
        Assert(port <= UINT16_MAX);
        oss << ':' << port;
    }
    const size_t iport = checked_cast<size_t>(oss.size());

    if (path.size()) {
        if (not StartsWith(path, "/"))
            return false;

        oss << path;
    }
    const size_t ipath = checked_cast<size_t>(oss.size());

    if (query.size()) {
        char prefix = '?';
        for (const auto& it : query) {
            oss << prefix;

            if (not UriEncode_(oss, MakeStringView(it.first)) )
                return false;

            oss << '=';

            if (not UriEncode_(oss, MakeStringView(it.second)) )
                return false;

            prefix = '&';
        }
    }
    const size_t iquery = checked_cast<size_t>(oss.size());

    if (anchor.size()) {
        if (not IsIdentifier(anchor))
            return false;

        oss << '#' << anchor;
    }
    const size_t ianchor = checked_cast<size_t>(oss.size());

    dst._str = ToString(oss.MakeView_NullTerminated());

    const FStringView written = MakeStringView(dst._str);
    Assert(written.size() == ianchor);

    dst._scheme = written.CutBefore(ischeme);
    dst._username = written.SubRange(ischeme, iusername > ischeme ? iusername - ischeme - 1 : 0);
    dst._hostname = written.SubRange(iusername, ihostname - iusername);
    dst._port = port;
    dst._path = written.SubRange(iport, ipath - iport);

    dst._query = (iquery > ipath)
        ? written.SubRange(ipath + 1, iquery - ipath - 1)
        : written.SubRange(ipath, 0);

    dst._anchor = (ianchor > iquery)
        ? written.SubRange(iquery + 1, ianchor - iquery - 1)
        : written.SubRange(ianchor, 0);

    return true;
}
//----------------------------------------------------------------------------
bool FUri::Unpack(FQueryMap& dst, const FUri& src) {
    FStringView decl;
    FStringView query = src._query;

    while (Split(query, '&', decl)) {
        const auto it = decl.Find('=');
        if (decl.end() == it)
            return false;

        const FStringView key = decl.CutBefore(it);
        if (key.empty())
            return false;

        FString decodedKey;
        if (not Decode(decodedKey, key))
            return false;

        const FStringView value = decl.CutStartingAt(it + 1);

        FString decodedValue;
        if (not Decode(decodedValue, value))
            return false;

        dst.Insert_AssertUnique(std::move(decodedKey), std::move(decodedValue));
    }

    return true;
}
//----------------------------------------------------------------------------
bool FUri::Parse(FUri& dst, FString&& src) {
    dst._str = std::move(src);

    FStringView str = MakeStringView(dst._str);

    const FStringView::iterator ischeme = str.FindSubRange("://");
    if (str.end() != ischeme) {
        dst._scheme = str.CutBefore(ischeme);
        if (not IsAlpha(dst._scheme))
            return false;

        str = str.CutStartingAt(ischeme + 3);
    }
    else {
        dst._scheme = FStringView();
    }

    const FStringView::iterator iusername = str.Find('@');
    if (str.end() != iusername) {
        dst._username = str.CutBefore(iusername);
        if (not IsIdentifier(dst._scheme))
            return false;

        str = str.CutStartingAt(iusername + 1);
    }
    else {
        dst._username = FStringView();
    }

    const FStringView::reverse_iterator ianchor = str.FindR('#');
    if (str.rend() != ianchor) {
        dst._anchor = str.CutStartingAt(ianchor - 1);
        if (not IsIdentifier(dst._scheme))
            return false;

        str = str.CutBefore(ianchor);
    }
    else {
        dst._anchor = FStringView();
    }

    const FStringView::reverse_iterator iquery = str.FindR('?');
    if (str.rend() != iquery) {
        dst._query = str.CutStartingAt(iquery - 1);
        str = str.CutBefore(iquery);
    }
    else {
        dst._query = FStringView();
    }

    const FStringView::iterator ipath = str.Find('/');
    if (str.end() != ipath) {
        dst._path = str.CutStartingAt(ipath);
        str = str.CutBefore(ipath);
    }
    else {
        dst._path = FStringView();
    }

    const FStringView::reverse_iterator iport = str.FindR(':');
    if (str.rend() != iport) {
        const FStringView port = str.CutStartingAt(iport - 1);
        if (not Atoi(&dst._port, port, 10))
            return false;

        str = str.CutBefore(iport);
    }
    else {
        dst._port = 0;
    }

    dst._hostname = str;
    if (not IsIdentifier(dst._hostname))
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool FUri::Parse(FUri& dst, const FStringView& strview) {
    return Parse(dst, ToString(strview));
}
//----------------------------------------------------------------------------
bool FUri::Decode(FString& dst, const FStringView& src) {
    FOStringStream oss;

    if (UriDecode_(oss, src)) {
        dst = oss.str();
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FUri::Encode(FString& dst, const FStringView& src) {
    FOStringStream oss;

    if (UriEncode_(oss, src)) {
        dst = oss.str();
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FUri::Decode(FOStream& dst, const FStringView& src) {
    return UriDecode_(dst, src);
}
//----------------------------------------------------------------------------
bool FUri::Encode(FOStream& dst, const FStringView& src) {
    return UriEncode_(dst, src);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Network
} //!namespace Core
