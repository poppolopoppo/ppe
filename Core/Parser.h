#pragma once

#include "Core.h"

#include "ParseContext.h"
#include "ParseExpression.h"
#include "ParseItem.h"
#include "ParseList.h"
#include "ParseProduction.h"
#include "ParseResult.h"
#include "ParseStatement.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ParserException : public std::exception {
public:
    ParserException(const char *what, Lexer::Location site)
        : std::exception(what), _site(site), _item(nullptr) {}

    ParserException(const char *what, const ParseItem *item)
        : std::exception(what), _site(item->Site()), _item(item) {}

    ParserException(const char *what, Lexer::Location site, const ParseItem *item)
        : std::exception(what), _site(site), _item(item) {}

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
