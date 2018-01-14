#include "stdafx.h"

#include "Serialize.h"

#include "Lexer/Lexer.h"
/*
#include "Parser/Parser.h"
#include "Text/Grammar.h"
#include "XML/XML.h"
*/

#include "Core/Allocator/PoolAllocatorTag-impl.h"

PRAGMA_INITSEG_LIB

namespace Core {
namespace Serialize {
POOL_TAG_DEF(Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FSerializeModule::Start() {
    CORE_MODULE_START(Serialize);

    POOL_TAG(Serialize)::Start();
    Lexer::FLexerStartup::Start();
    /*
    Parser::FParserStartup::Start();
    FGrammarStartup::Start();
    XML::FXMLStartup::Start();
    */
}
//----------------------------------------------------------------------------
void FSerializeModule::Shutdown() {
    CORE_MODULE_SHUTDOWN(Serialize);

    /*
    XML::FXMLStartup::Shutdown();
    FGrammarStartup::Shutdown();
    Parser::FParserStartup::Shutdown();
    */
    Lexer::FLexerStartup::Shutdown();
    POOL_TAG(Serialize)::Shutdown();
}
//----------------------------------------------------------------------------
void FSerializeModule::ClearAll_UnusedMemory() {
    CORE_MODULE_CLEARALL(Serialize);

    Lexer::FLexerStartup::ClearAll_UnusedMemory();
    /*
    Parser::FParserStartup::ClearAll_UnusedMemory();
    FGrammarStartup::ClearAll_UnusedMemory();
    XML::FXMLStartup::ClearAll_UnusedMemory();
    */
    POOL_TAG(Serialize)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
