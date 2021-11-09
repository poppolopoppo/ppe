#pragma once

#include "PipelineCompiler_fwd.h"

#include "Modular/ModuleInterface.h"

#include "Diagnostic/Logger_fwd.h"
#include "Misc/EventHandle.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_PIPELINECOMPILER_API, PipelineCompiler)
} //!namespace RHI
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_PIPELINECOMPILER_API FPipelineCompilerModule final : public IModuleInterface {
public:
    static const FModuleInfo StaticInfo;

    explicit FPipelineCompilerModule() NOEXCEPT;

    virtual void Start(FModularDomain& domain) override;
    virtual void Shutdown(FModularDomain& domain) override;

    virtual void DutyCycle(FModularDomain& domain) override;
    virtual void ReleaseMemory(FModularDomain& domain) NOEXCEPT override;

private:
    FEventHandle _onDeviceCreated;
    FEventHandle _onDeviceTeardown;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
