// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Parser/Parser.h"

#if USE_PPE_EXCEPTION_DESCRIPTION
#   include "Parser/ParseItem.h"
#   include "IO/TextWriter.h"
#endif

namespace PPE {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FTextWriter& FParserException::Description(FTextWriter& oss) const {
    oss << MakeCStringView(What()) << ": ";

    if (_item)
        oss << _item->ToString() << Eol;

    return oss << "\tat " << _site;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
