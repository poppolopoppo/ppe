// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RTTI/Typedefs.h"

#include "MetaObject.h"
#include "MetaTransaction.h"

#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RTTI {
BASICTOKEN_CLASS_DEF(FName);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
STATIC_ASSERT(Meta::is_pod_v<RTTI::FName>);
STATIC_ASSERT(Meta::is_pod_v<RTTI::FLazyName>);
STATIC_ASSERT(Meta::is_pod_v<RTTI::FLazyPathName>);
STATIC_ASSERT(Meta::is_pod_v<RTTI::FPathName>);
//----------------------------------------------------------------------------
static FTextWriter& FormatPath_(FTextWriter& oss, const FStringView& namespace_, const FStringView& id) {
    if (not namespace_.empty())
        oss << "$/" << namespace_ << '/';
    return oss << id;
}
//----------------------------------------------------------------------------
static FWTextWriter& FormatPath_(FWTextWriter& oss, const FStringView& namespace_, const FStringView& id) {
    if (not namespace_.empty())
        oss << L"$/" << namespace_ << L'/';
    return oss << id;
}
//----------------------------------------------------------------------------
template <typename _Char>
static bool ParseBinaryData_(const TBasicStringConversion<_Char>& src, FBinaryData* dst) {
    Assert(dst);

    const size_t len = Base64DecodeSize(src.Input);
    dst->Resize_DiscardData(len);

    u8* it = dst->data();
    return Base64Decode(src.Input, { &it, [](void* user, u8&& bin) {
        *(*static_cast<u8**>(user))++ = bin;
    } });
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FLazyPathName::Parse(FLazyPathName* pathName, const FStringView& text, bool withPrefix/* = true */) {
    Assert(pathName);

    FStringView namespace_{ text };
    if (withPrefix) {
        if (text.size() < 3)
            return false;
        if (text[0] != '$' || text[1] != '/')
            return false;

        namespace_ = text.ShiftFront(2); // skip '$/'
    }

    const auto sep = namespace_.FindR('/');
    if (namespace_.rend() == sep)
        return false;

    const FStringView identifier = namespace_.CutStartingAt(sep - 1);
    namespace_ = namespace_.CutBefore(sep);

    if (namespace_.empty() || identifier.empty() ||
        identifier.Contains('/') )
        return false;

    pathName->Namespace = FLazyName(namespace_);
    pathName->Identifier = FLazyName(identifier);

    return pathName->Valid();
}
//----------------------------------------------------------------------------
FPathName FPathName::FromObject(const FMetaObject& obj) NOEXCEPT {
    Assert(obj.RTTI_IsExported());
    Assert(obj.RTTI_Outer());

    FPathName p;
    p.Namespace = obj.RTTI_Outer()->Namespace();
    p.Identifier = obj.RTTI_Name();

    return p;
}
//----------------------------------------------------------------------------
bool FPathName::Parse(FPathName* pathName, const FStringView& text) {
    Assert(pathName);

    FLazyPathName lazy;
    if (FLazyPathName::Parse(&lazy, text)) {
        pathName->Namespace = lazy.Namespace;
        pathName->Identifier = lazy.Identifier;
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FBinaryData& bindata) {
    return oss << Fmt::Base64(bindata.MakeView());
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FBinaryData& bindata) {
    return oss << Fmt::Base64(bindata.MakeView());
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FLazyPathName& pathName) {
    return FormatPath_(oss, pathName.Namespace.MakeView(), pathName.Identifier.MakeView());
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FLazyPathName& pathName) {
    return FormatPath_(oss, pathName.Namespace.MakeView(), pathName.Identifier.MakeView());
}
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const FPathName& pathName) {
    return FormatPath_(oss, pathName.Namespace.MakeView(), pathName.Identifier.MakeView());
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const FPathName& pathName) {
    return FormatPath_(oss, pathName.Namespace.MakeView(), pathName.Identifier.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool operator >>(const FStringConversion& iss, FBinaryData* bindata) {
    return ParseBinaryData_(iss, bindata);
}
//----------------------------------------------------------------------------
bool operator >>(const FWStringConversion& iss, FBinaryData* bindata) {
    return ParseBinaryData_(iss, bindata);
}
//----------------------------------------------------------------------------
bool operator >>(const FStringConversion& iss, FLazyPathName* pathName) {
    Assert(pathName);
    return FLazyPathName::Parse(pathName, iss.Input);
}
//----------------------------------------------------------------------------
bool operator >>(const FWStringConversion& iss, FLazyPathName* pathName) {
    Assert(pathName);
    return FLazyPathName::Parse(pathName, WCHAR_TO_UTF_8(iss.Input));
}
//----------------------------------------------------------------------------
bool operator >>(const FStringConversion& iss, FPathName* pathName) {
    Assert(pathName);
    return FPathName::Parse(pathName, iss.Input);
}
//----------------------------------------------------------------------------
bool operator >>(const FWStringConversion& iss, FPathName* pathName) {
    Assert(pathName);
    return FPathName::Parse(pathName, WCHAR_TO_UTF_8(iss.Input));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RTTI
} //!namespace PPE
