#include "stdafx.h"

#include "Serialize.h"

#include "Text/Grammar.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

#ifdef OS_WINDOWS
#   pragma warning(disable: 4073) // initialiseurs placés dans la zone d'initialisation d'une bibliothèque
#   pragma init_seg(lib)
#else
#   error "missing compiler specific command"
#endif

namespace Core {
namespace Serialize {
POOL_TAG_DEF(Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void SerializeStartup::Start() {
    POOL_TAG(Serialize)::Start();
    GrammarStartup::Start();
}
//----------------------------------------------------------------------------
void SerializeStartup::Shutdown() {
    GrammarStartup::Shutdown();
    POOL_TAG(Serialize)::Shutdown();
}
//----------------------------------------------------------------------------
void SerializeStartup::ClearAll_UnusedMemory() {
    GrammarStartup::ClearAll_UnusedMemory();
    POOL_TAG(Serialize)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
