#include "stdafx.h"

#include "RTTI/Typedefs.h"

#include "MetaObject.h"
#include "MetaTransaction.h"

#include "IO/Format.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"

namespace PPE {
namespace RTTI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPathName::FPathName(const FMetaObject& obj) {
    Assert(obj.RTTI_IsExported());
    Assert(obj.RTTI_Outer());

    Transaction = obj.RTTI_Outer()->Name();
    Identifier = obj.RTTI_Name();
}
//----------------------------------------------------------------------------
bool FPathName::Parse(FPathName* pathName, const FStringView& text) {
    Assert(pathName);

    if (text.size() < 3)
        return false;
    if (text[0] != '$' || text[1] != '/')
        return false;

    FStringView transaction;
    FStringView identifier = text.CutStartingAt(2);
    if (not Split(identifier, '/', transaction))
        return false;

    if (transaction.empty())
        return false;
    if (identifier.empty())
        return false;
    if (identifier.Contains('/'))
        return false;

    pathName->Transaction = FName(transaction);
    pathName->Identifier = FName(identifier);

    return true;
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
FTextWriter& operator <<(FTextWriter& oss, const RTTI::FPathName& pathName) {
    return oss << '$' << '/' << pathName.Transaction << '/' << pathName.Identifier;
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FPathName& pathName) {
    return oss << L'$' << L'/' << pathName.Transaction << L'/' << pathName.Identifier;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
