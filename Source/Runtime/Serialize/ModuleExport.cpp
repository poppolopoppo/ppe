#include "stdafx.h"

#include "ModuleExport.h"

#include "Serialize.h"

#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "Text/Grammar.h"

#include "Diagnostic/Logger.h"

#include "RTTI/Namespace-impl.h"

PRAGMA_INITSEG_LIB

namespace PPE {
namespace Serialize {
LOG_CATEGORY(, Serialize);
RTTI_NAMESPACE_DEF(PPE_SERIALIZE_API, Serialize, MetaSerialize);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FSerializeModule::FSerializeModule()
:   FModule("Runtime/Serialize")
{}
//----------------------------------------------------------------------------
FSerializeModule::~FSerializeModule() = default;
//----------------------------------------------------------------------------
void FSerializeModule::Start(FModuleManager& manager) {
    FModule::Start(manager);

    Lexer::FLexerStartup::Start();
    FGrammarStartup::Start();

    RTTI_NAMESPACE(Serialize).Start();
}
//----------------------------------------------------------------------------
void FSerializeModule::Shutdown() {
    FModule::Shutdown();

    RTTI_NAMESPACE(Serialize).Shutdown();

    FGrammarStartup::Shutdown();
    Lexer::FLexerStartup::Shutdown();
}
//----------------------------------------------------------------------------
void FSerializeModule::ReleaseMemory() {
    FModule::ReleaseMemory();

    Lexer::FLexerStartup::ClearAll_UnusedMemory();
    FGrammarStartup::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
