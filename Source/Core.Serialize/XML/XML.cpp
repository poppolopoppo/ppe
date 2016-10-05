#include "stdafx.h"

#include "XML.h"

#include "Name.h"

#include "Core/Allocator/PoolAllocatorTag-impl.h"

namespace Core {
namespace XML {
POOL_TAG_DEF(XML);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FXMLStartup::Start() {
    POOL_TAG(XML)::Start();
    XML::FName::Start(256);
}
//----------------------------------------------------------------------------
void FXMLStartup::Shutdown() {
    XML::FName::Shutdown();
    POOL_TAG(XML)::Shutdown();
}
//----------------------------------------------------------------------------
void FXMLStartup::ClearAll_UnusedMemory() {
    POOL_TAG(XML)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
