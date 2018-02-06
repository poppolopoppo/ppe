#include "stdafx.h"

#include "Typedefs.h"

#include "Core/IO/Format.h"
#include "Core/IO/StringView.h"
#include "Core/IO/TextWriter.h"

namespace Core {
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
} //!namespace Core
