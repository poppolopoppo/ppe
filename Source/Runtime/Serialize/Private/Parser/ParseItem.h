#pragma once

#include "Serialize.h"

#include "Lexer/Location.h"
#include "IO/String.h"
#include "Memory/RefPtr.h"

namespace PPE {
namespace Parser {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FParseContext;
FWD_REFPTR(ParseItem);
//----------------------------------------------------------------------------
class FParseItem : public FRefCountable {
public:
    FParseItem(const Lexer::FLocation& site);
    virtual ~FParseItem();

    const Lexer::FLocation& Site() const { return _site; }

    virtual void Invoke(FParseContext *context) const = 0;
    virtual FString ToString() const { return FString(); }

    template <typename _ParseItemImpl>
    const _ParseItemImpl *As() const {
        return checked_cast<const _ParseItemImpl *>(this);
    }

private:
    Lexer::FLocation _site;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Parser
} //!namespace PPE
