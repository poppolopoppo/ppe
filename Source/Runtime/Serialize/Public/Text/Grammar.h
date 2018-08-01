#pragma once

#include "Serialize.h"

#include "Container/Vector.h"
#include "Memory/RefPtr.h"
#include "Memory/UniquePtr.h"
#include "Meta/Singleton.h"

namespace PPE { namespace Parser {
FWD_REFPTR(ParseExpression);
FWD_REFPTR(ParseStatement);
class FParseList;
}}

namespace PPE {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGrammarStartup {
public:
    PPE_SERIALIZE_API static void Start();
    PPE_SERIALIZE_API static void Shutdown();

    PPE_SERIALIZE_API static void ClearAll_UnusedMemory();

    PPE_SERIALIZE_API static Parser::PCParseExpression ParseExpression(Parser::FParseList& input);
    PPE_SERIALIZE_API static Parser::PCParseStatement ParseStatement(Parser::FParseList& input);

    FGrammarStartup()  { Start(); }
    ~FGrammarStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace PPE
