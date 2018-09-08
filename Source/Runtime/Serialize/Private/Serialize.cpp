#include "stdafx.h"

#include "Serialize.h"

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Text/Grammar.h"

#include "Allocator/PoolAllocatorTag-impl.h"

PRAGMA_INITSEG_LIB

namespace PPE {
namespace Serialize {
POOL_TAG_DEF(Serialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSerializeModule::FSerializeModule()
:   FModule("Runtime/Serialize")
{}
//----------------------------------------------------------------------------
FSerializeModule::~FSerializeModule()
{}
//----------------------------------------------------------------------------
void FSerializeModule::Start(FModuleManager& manager) {
    FModule::Start(manager);

    POOL_TAG(Serialize)::Start();
    Lexer::FLexerStartup::Start();
    Parser::FParserStartup::Start();
    FGrammarStartup::Start();
}
//----------------------------------------------------------------------------
void FSerializeModule::Shutdown() {
    FModule::Shutdown();

    FGrammarStartup::Shutdown();
    Parser::FParserStartup::Shutdown();
    Lexer::FLexerStartup::Shutdown();
    POOL_TAG(Serialize)::Shutdown();
}
//----------------------------------------------------------------------------
void FSerializeModule::ReleaseMemory() {
    FModule::ReleaseMemory();

    Lexer::FLexerStartup::ClearAll_UnusedMemory();
    Parser::FParserStartup::ClearAll_UnusedMemory();
    FGrammarStartup::ClearAll_UnusedMemory();
    POOL_TAG(Serialize)::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
