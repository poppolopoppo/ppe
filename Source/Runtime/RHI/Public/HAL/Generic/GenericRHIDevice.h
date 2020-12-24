#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "Memory/MemoryView.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericPresentMode : u32 {
    Immediate = 0,
    Fifo,
    RelaxedFifo,
    Mailbox,
};
//----------------------------------------------------------------------------
struct FGenericShaderModule {

};
//----------------------------------------------------------------------------
class PPE_RHI_API FGenericDevice : Meta::FNonCopyable {
public: // must be defined by every RHI:
    FGenericDevice() = default;

    const TMemoryView<const EGenericPresentMode> PresentModes() const NOEXCEPT = delete;
    const TMemoryView<const FGenericSurfaceFormat> SurfaceFormats() const NOEXCEPT = delete;

    const FGenericSwapChain* SwapChain() const NOEXCEPT = delete;

    void CreateSwapChain(
        FGenericWindowSurface surface,
        EGenericPresentMode present,
        const FGenericSurfaceFormat& surfaceFormat ) = delete;
    void DestroySwapChain() = delete;

    FGenericShaderModule* CreateShaderModule(const FRawMemoryConst& code) = delete;
    void DestroyShaderModule(FGenericShaderModule* shaderModule) = delete;

    FGenericDescriptorSetLayoutHandle CreateDescriptorSetLayout(const FGenericDescriptorSetLayout& desc) = delete;
    void DestroyDescriptorSetLayout(FGenericDescriptorSetLayoutHandle setLayout) = delete;

    FGenericPipelineLayoutHandle CreatePipelineLayout(const FGenericPipelineLayout& desc) = delete;
    void DestroyPipelineLayout(FGenericPipelineLayoutHandle pipelineLayout) = delete;

public: // shared by each device

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
