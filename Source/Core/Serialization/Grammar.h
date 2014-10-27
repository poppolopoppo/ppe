#pragma once

#include "Core/Core.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"

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
