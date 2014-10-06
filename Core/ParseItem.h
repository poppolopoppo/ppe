#pragma once

#include "Location.h"
#include "RefPtr.h"
#include "String.h"

namespace Core {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ParseContext;
FWD_REFPTR(ParseItem);
//----------------------------------------------------------------------------
class ParseItem : public RefCountable {
public:
    ParseItem(const Lexer::Location& site);
    virtual ~ParseItem();

    const Lexer::Location& Site() const { return _site; }

    virtual void Invoke(ParseContext *context) const = 0;
    virtual String ToString() const { return String(); }

private:
    Lexer::Location _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
