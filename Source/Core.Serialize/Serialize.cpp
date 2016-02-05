#include "stdafx.h"

#include "Serialize.h"

#include "Text/Grammar.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

namespace Core {
namespace Serialize {
POOLTAG_DEF(Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void SerializeStartup::Start() {
    POOLTAG(Serialize)::Start();
    GrammarStartup::Start();
}
//----------------------------------------------------------------------------
void SerializeStartup::Shutdown() {
    GrammarStartup::Shutdown();
    POOLTAG(Serialize)::Shutdown();
}
//----------------------------------------------------------------------------
void SerializeStartup::ClearAll_UnusedMemory() {
    GrammarStartup::ClearAll_UnusedMemory();
    POOLTAG(Serialize)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
