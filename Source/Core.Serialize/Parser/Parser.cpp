#include "stdafx.h"

#include "Parser.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

namespace Core {
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
} //!namespace Core
