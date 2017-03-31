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
} //!namespace Core
