#include "stdafx.h"

#include "ModuleExport.h"

#include "Serialize.h"

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Text/Grammar.h"

#include "Allocator/PoolAllocatorTag-impl.h"
#include "Diagnostic/Logger.h"

#include "RTTI/Namespace-impl.h"

PRAGMA_INITSEG_LIB

namespace PPE {
namespace Serialize {
LOG_CATEGORY(, Serialize);
POOL_TAG_DEF(Serialize);
RTTI_NAMESPACE_DEF(PPE_SERIALIZE_API, Serialize, MetaSerialize);
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

    RTTI_NAMESPACE(Serialize).Start();
}
//----------------------------------------------------------------------------
void FSerializeModule::Shutdown() {
    FModule::Shutdown();

    RTTI_NAMESPACE(Serialize).Shutdown();

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
