#pragma once

#include "Core.Serialize/Serialize.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/Singleton.h"

namespace Core { namespace Parser {
    FWD_REFPTR(ParseItem);
    class FParseList;
}}

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGrammarStartup {
public:
    static void Start();
    static void Shutdown();

    static void ClearAll_UnusedMemory();

    static Parser::PCParseItem Parse(Parser::FParseList& input);

    FGrammarStartup()  { Start(); }
    ~FGrammarStartup() { Shutdown(); }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
