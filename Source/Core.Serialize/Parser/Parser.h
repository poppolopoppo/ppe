#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Allocator/PoolAllocatorTag.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/Lexer/Location.h"
#include "Core.Serialize/Parser/ParseItem.h"

namespace Core {
namespace Parser {
POOL_TAG_DECL(Parser);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParserException : public Core::Serialize::FSerializeException {
public:
    typedef Core::Serialize::FSerializeException parent_type;

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
} //!namespace Core
