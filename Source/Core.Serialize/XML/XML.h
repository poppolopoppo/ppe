#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/Lexer/Location.h"

#include "Core/Allocator/PoolAllocatorTag.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace XML {
POOL_TAG_DECL(XML);
FWD_REFPTR(Document);
FWD_REFPTR(Element);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class XMLException : public Core::Serialize::SerializeException {
public:
    typedef Core::Serialize::SerializeException parent_type;

    XMLException(const char *what, const Lexer::Location& site)
        : parent_type(what), _site(site) {}

    virtual ~XMLException() {}

    const Lexer::Location& Site() const { return _site; }

private:
    Lexer::Location _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct XMLStartup {
    static void Start();
    static void Shutdown();
    static void ClearAll_UnusedMemory();

    XMLStartup() { Start(); }
    ~XMLStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace XML
} //!namespace Core
