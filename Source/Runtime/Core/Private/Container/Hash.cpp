#include "stdafx.h"

#include "Container/Hash.h"

#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTextWriter& operator <<(FTextWriter& oss, hash_t h) {
    return *oss.FormatScope()
        << '#'
        << FTextFormat::Hexadecimal
        << Fmt::PadLeft(h._value, sizeof(hash_t) * 2, '0');
}
//----------------------------------------------------------------------------
FWTextWriter& operator <<(FWTextWriter& oss, hash_t h) {
    return *oss.FormatScope()
        << L'#'
        << FTextFormat::Hexadecimal
        << Fmt::PadLeft(h._value, sizeof(hash_t) * 2, L'0');
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
}  //! namespace PPE
