#pragma once

#include "Core/Core.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/UniquePtr.h"
#include "Core/Meta/Singleton.h"

namespace Core { namespace Parser {
    FWD_REFPTR(ParseItem);
    class ParseList;
}}

namespace Core {
namespace Serialize {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Grammar_Create();
//----------------------------------------------------------------------------
void Grammar_Destroy();
//----------------------------------------------------------------------------
Parser::PCParseItem Grammar_Parse(Parser::ParseList& input);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Serialize
} //!namespace Core
