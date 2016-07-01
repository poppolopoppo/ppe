#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core.Serialize/Lexer/Location.h"
#include "Core/IO/String.h"
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

    template <typename _ParseItemImpl>
    const _ParseItemImpl *As() const {
        return checked_cast<const _ParseItemImpl *>(this);
    }

private:
    Lexer::Location _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace Core
