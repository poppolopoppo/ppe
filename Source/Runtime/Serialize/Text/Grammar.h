#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/Singleton.h"

namespace Core { namespace Parser {
FWD_REFPTR(ParseExpression);
FWD_REFPTR(ParseStatement);
class FParseList;
}}

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGrammarStartup {
public:
    CORE_SERIALIZE_API static void Start();
    CORE_SERIALIZE_API static void Shutdown();

    CORE_SERIALIZE_API static void ClearAll_UnusedMemory();

    CORE_SERIALIZE_API static Parser::PCParseExpression ParseExpression(Parser::FParseList& input);
    CORE_SERIALIZE_API static Parser::PCParseStatement ParseStatement(Parser::FParseList& input);

    FGrammarStartup()  { Start(); }
    ~FGrammarStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
