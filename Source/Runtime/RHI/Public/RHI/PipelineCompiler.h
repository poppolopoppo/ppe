#pragma once

#include "RHI_fwd.h"
#include "HAL/TargetRHI.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_RHI_API IPipelineCompiler : public FRefCountable {
public:
    virtual ~IPipelineCompiler() = default;

    virtual FString DisplayName() const NOEXCEPT = 0;;
    virtual ETargetRHI TargetRHI() const NOEXCEPT = 0;

    virtual bool IsSupported(const FMeshPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT = 0;
    virtual bool IsSupported(const FRayTracingPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT = 0;
    virtual bool IsSupported(const FGraphicsPipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT = 0;
    virtual bool IsSupported(const FComputePipelineDesc& desc, EShaderLangFormat fmt) const NOEXCEPT = 0;

    virtual bool Compile(FMeshPipelineDesc& desc, EShaderLangFormat fmt) = 0;
    virtual bool Compile(FRayTracingPipelineDesc& desc, EShaderLangFormat fmt) = 0;
    virtual bool Compile(FGraphicsPipelineDesc& desc, EShaderLangFormat fmt) = 0;
    virtual bool Compile(FComputePipelineDesc& desc, EShaderLangFormat fmt) = 0;

    virtual void ReleaseUnusedMemory() = 0;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
