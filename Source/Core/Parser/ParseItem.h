#pragma once

#include "Core/Core.h"

#include "Core/IO/String.h"
#include "Core/Lexer/Location.h"
#include "Core/Memory/RefPtr.h"

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
