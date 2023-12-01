// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "TextureModule.h"

#include "TextureService.h"

// #include "RTTI/Module-impl.h"

#include "Diagnostic/Logger.h"
#include "Modular/ModularDomain.h"
#include "Modular/ModuleRegistration.h"

#include "Diagnostic/BuildVersion.h"
#include "BuildModules.generated.h"
// #include "RTTI/Service.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace ContentPipeline {
//----------------------------------------------------------------------------
LOG_CATEGORY(PPE_TEXTURE_API, Texture)
//----------------------------------------------------------------------------
// RTTI_MODULE_DEF(PPE_TEXTURE_API, Texture, Texture);
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FModuleInfo FTextureModule::StaticInfo{
    FModuleStaticRegistration::MakeInfo<FTextureModule>(
        STRINGIZE(BUILD_TARGET_NAME),
        EModulePhase::Framework,
        EModuleUsage::Runtime,
        EModuleSource::Core,
        BUILD_TARGET_ORDINAL,
        Generated::DependencyList,
        CurrentBuildVersion() )
};
//----------------------------------------------------------------------------
FTextureModule::FTextureModule() NOEXCEPT
:   IModuleInterface(StaticInfo)
{}
//----------------------------------------------------------------------------
FTextureModule::~FTextureModule() = default;
//----------------------------------------------------------------------------
const ITextureService& FTextureModule::TextureService() const NOEXCEPT {
    return (*_textureService);
}
//----------------------------------------------------------------------------
void FTextureModule::Start(FModularDomain& domain) {
    using namespace ContentPipeline;
    auto& services = domain.Services();

    // RTTI_MODULE(Texture).Start();
    // services.Get<IRTTIService>().RegisterModule(this, RTTI_MODULE(Texture));

    ITextureService::MakeDefault(&_textureService);
    services.Add<ITextureService>(_textureService.get());

    IModuleInterface::Start(domain);
}
//----------------------------------------------------------------------------
void FTextureModule::Shutdown(FModularDomain& domain) {
    using namespace ContentPipeline;
    IModuleInterface::Shutdown(domain);

    auto& services = domain.Services();

    services.Remove<ITextureService>();
    _textureService.reset();

    // services.Get<IRTTIService>().UnregisterModule(this, RTTI_MODULE(Texture));
    // RTTI_MODULE(Texture).Shutdown();
}
//----------------------------------------------------------------------------
void FTextureModule::DutyCycle(FModularDomain& domain) {
    IModuleInterface::DutyCycle(domain);
}
//----------------------------------------------------------------------------
void FTextureModule::ReleaseMemory(FModularDomain& domain) NOEXCEPT {
    IModuleInterface::ReleaseMemory(domain);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
