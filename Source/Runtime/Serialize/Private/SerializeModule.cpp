// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "SerializeModule.h"

#include "Serialize.h"
#include "Lexer/Lexer.h"
#include "Text/Grammar.h"

#include "Modular/ModuleRegistration.h"

#include "Diagnostic/Logger.h"
#include "RTTI/Module-impl.h"

#include "BuildModules.generated.h"
#include "Diagnostic/BuildVersion.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace Serialize {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_SERIALIZE_API, Serialize)
//----------------------------------------------------------------------------
RTTI_MODULE_DEF(PPE_SERIALIZE_API, Serialize, MetaSerialize);
//----------------------------------------------------------------------------
} //!namespace Serialize
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FSerializeModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FSerializeModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Bare,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList,
        CurrentBuildVersion() )
};
//----------------------------------------------------------------------------
FSerializeModule::FSerializeModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
void FSerializeModule::Start(FModularDomain& domain) {
    IModuleInterface::Start(domain);

    using namespace Serialize;

    Lexer::FLexerStartup::Start();
    FGrammarStartup::Start();

    RTTI_MODULE(Serialize).Start();
}
//----------------------------------------------------------------------------
void FSerializeModule::Shutdown(FModularDomain& domain) {
    IModuleInterface::Shutdown(domain);

    using namespace Serialize;

    RTTI_MODULE(Serialize).Shutdown();

    FGrammarStartup::Shutdown();
    Lexer::FLexerStartup::Shutdown();
}
//----------------------------------------------------------------------------
void FSerializeModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);

}
//----------------------------------------------------------------------------
void FSerializeModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);

    using namespace Serialize;

    Lexer::FLexerStartup::ClearAll_UnusedMemory();
    FGrammarStartup::ClearAll_UnusedMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
