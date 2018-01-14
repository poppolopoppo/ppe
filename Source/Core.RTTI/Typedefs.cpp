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
    oss << "FBinaryData:\"";
    Escape(oss, FStringView((const char*)bindata.data(), bindata.size()), EEscape::Hexadecimal);
    return oss << "\"";
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, const RTTI::FBinaryData& bindata) {
    oss << L"FBinaryData:\"";
    Escape(oss, FWStringView((const wchar_t*)bindata.data(), bindata.size()), EEscape::Hexadecimal);
    return oss << L"\"";
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
