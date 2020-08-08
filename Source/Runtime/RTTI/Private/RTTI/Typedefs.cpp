#include "stdafx.h"

#include "RTTI/Typedefs.h"

#include "MetaObject.h"
#include "MetaTransaction.h"

#include "IO/Format.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RTTI {
BASICTOKEN_CLASS_DEF(FName);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FLazyPathName::Parse(FLazyPathName* pathName, const FStringView& text) {
    Assert(pathName);

    if (text.size() < 3)
        return false;
    if (text[0] != '$' || text[1] != '/')
        return false;

    FStringView namespace_;
    FStringView identifier = text.CutStartingAt(2);
    if (not Split(identifier, '/', namespace_))
        return false;

    if (namespace_.empty() ||
        identifier.empty() ||
        identifier.Contains('/') )
            return false;

    pathName->Namespace = FLazyName(namespace_);
    pathName->Identifier = FLazyName(identifier);

    return true;
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
} //!namespace RTTI
} //!namespace PPE

namespace PPE {
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
        oss << '$' << '/' << namespace_ << '/';
    return oss << id;
}
//----------------------------------------------------------------------------
static FWTextWriter& FormatPath_(FWTextWriter& oss, const FStringView& namespace_, const FStringView& id) {
    if (not namespace_.empty())
        oss << L'$' << L'/' << namespace_ << L'/';
    return oss << id;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FBinaryData& bindata) {
    oss << '"';
    Escape(oss, bindata.MakeConstView().Cast<const char>(), EEscape::Hexadecimal);
    return oss << '"';
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FBinaryData& bindata) {
    oss << L'"';
    Escape(oss, bindata.MakeConstView().Cast<const wchar_t>(), EEscape::Unicode);
    return oss << L'"';
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FLazyPathName& pathName) {
    return FormatPath_(oss, pathName.Namespace.MakeView(), pathName.Identifier.MakeView());
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FLazyPathName& pathName) {
    return FormatPath_(oss, pathName.Namespace.MakeView(), pathName.Identifier.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FPathName& pathName) {
    return FormatPath_(oss, pathName.Namespace.MakeView(), pathName.Identifier.MakeView());
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FPathName& pathName) {
    return FormatPath_(oss, pathName.Namespace.MakeView(), pathName.Identifier.MakeView());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
