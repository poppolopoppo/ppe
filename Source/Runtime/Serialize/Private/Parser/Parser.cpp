#include "stdafx.h"

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
FWTextWriter& FParserException::Description(FWTextWriter& oss) const {
    oss << MakeCStringView(What()) << L": ";

    if (_item)
        oss << _item->ToString().MakeView() << Eol;

    return oss << L"\tat " << _site;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
