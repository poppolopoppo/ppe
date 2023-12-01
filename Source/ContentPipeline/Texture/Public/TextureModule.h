#pragma once

#include "Texture_fwd.h"

// #include "RTTI/Module.h"

#include "Diagnostic/Logger_fwd.h"
#include "Modular/ModuleInterface.h"

namespace PPE {
namespace ContentPipeline {
EXTERN_LOG_CATEGORY(PPE_TEXTURE_API, Texture);
// RTTI_MODULE_DECL(PPE_TEXTURE_API, Texture);
} //!namespace ContentPipeline
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_TEXTURE_API FTextureModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FTextureModule() NOEXCEPT;
    virtual ~FTextureModule() override;

    NODISCARD const ITextureService& TextureService() const NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

private:
    UTextureService _textureService;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
