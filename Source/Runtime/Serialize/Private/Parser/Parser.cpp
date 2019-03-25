#include "stdafx.h"

#include "Parser/Parser.h"

#include "Allocator/PoolAllocatorTag-impl.h"

#if USE_PPE_EXCEPTION_DESCRIPTION
#   include "Parser/ParseItem.h"
#   include "IO/TextWriter.h"
#endif

namespace PPE {
namespace Parser {
POOL_TAG_DEF(Parser);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_EXCEPTION_DESCRIPTION
FWTextWriter& FParserException::Description(FWTextWriter& oss) const {
    oss << MakeCStringView(What()) << L": ";

    if (_item)
        oss << _item->ToString() << Eol;

    return oss << L"\tat " << _site;
}
#endif
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FParserStartup::Start() {
    POOL_TAG(Parser)::Start();
}
//----------------------------------------------------------------------------
void FParserStartup::Shutdown() {
    POOL_TAG(Parser)::Shutdown();
}
//----------------------------------------------------------------------------
void FParserStartup::ClearAll_UnusedMemory() {
    POOL_TAG(Parser)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
