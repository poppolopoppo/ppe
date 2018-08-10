#include "stdafx.h"

#include "Parser/Parser.h"

#include "Allocator/PoolAllocatorTag-impl.h"

namespace PPE {
namespace Parser {
POOL_TAG_DEF(Parser);
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
