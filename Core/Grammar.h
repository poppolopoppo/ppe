#pragma once

#include "Core.h"
#include "RefPtr.h"
#include "UniquePtr.h"
#include "Vector.h"

namespace Core { namespace Parser {
    FWD_REFPTR(ParseItem);
    class ParseList;
}}

namespace Core {
namespace Serialization {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class GrammarImpl;
//----------------------------------------------------------------------------
class Grammar {
public:
    Grammar();
    ~Grammar();

    Grammar(const Grammar&) = delete;
    Grammar& operator =(const Grammar&) = delete;

    Parser::PCParseItem Parse(Parser::ParseList& input) const;

private:
    UniquePtr<GrammarImpl> _impl;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialization
} //!namespace Core
