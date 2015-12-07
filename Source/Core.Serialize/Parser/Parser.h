#pragma once

#include "Core/Core.h"

#include "Core.Serialize/Exceptions.h"
#include "Core.Serialize/Parser/ParseContext.h"
#include "Core.Serialize/Parser/ParseExpression.h"
#include "Core.Serialize/Parser/ParseItem.h"
#include "Core.Serialize/Parser/ParseList.h"
#include "Core.Serialize/Parser/ParseProduction.h"
#include "Core.Serialize/Parser/ParseResult.h"
#include "Core.Serialize/Parser/ParseStatement.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ParserException : public Core::Serialize::SerializeException {
public:
    typedef Core::Serialize::SerializeException parent_type;

    ParserException(const char *what, Lexer::Location site)
        : parent_type(what), _site(site), _item(nullptr) {}

    ParserException(const char *what, const ParseItem *item)
        : parent_type(what), _site(item->Site()), _item(item) {}

    ParserException(const char *what, Lexer::Location site, const ParseItem *item)
        : parent_type(what), _site(site), _item(item) {}

    virtual ~ParserException() {}

    const Lexer::Location& Site() const { return _site; }
    const ParseItem *Item() const { return _item.get(); }

private:
    Lexer::Location _site;
    PCParseItem _item;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct ParserStartup {
    static void Start();
    static void Shutdown();

    ParserStartup() { Start(); }
    ~ParserStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
