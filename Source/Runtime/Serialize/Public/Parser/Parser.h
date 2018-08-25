#pragma once

#include "Serialize.h"

#include "Allocator/PoolAllocatorTag.h"

#include "Lexer/Location.h"
#include "Parser/ParseItem.h"
#include "SerializeExceptions.h"

namespace PPE {
namespace Parser {
POOL_TAG_DECL(Parser);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParserException : public PPE::Serialize::FSerializeException {
public:
    typedef PPE::Serialize::FSerializeException parent_type;

    FParserException(const char *what, Lexer::FLocation site)
        : parent_type(what), _site(site), _item(nullptr) {}

    FParserException(const char *what, const FParseItem *item)
        : parent_type(what), _site(item->Site()), _item(item) {}

    FParserException(const char *what, Lexer::FLocation site, const FParseItem *item)
        : parent_type(what), _site(site), _item(item) {}

    virtual ~FParserException() {}

    const Lexer::FLocation& Site() const { return _site; }
    const FParseItem *Item() const { return _item.get(); }

private:
    Lexer::FLocation _site;
    PCParseItem _item;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FParserStartup {
    static void Start();
    static void Shutdown();
    static void ClearAll_UnusedMemory();

    FParserStartup() { Start(); }
    ~FParserStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
