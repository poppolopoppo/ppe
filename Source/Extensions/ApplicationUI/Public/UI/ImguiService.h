#pragma once

#include "ApplicationUI_fwd.h"

#include "UI/UIService.h"

#include "RHI_fwd.h"
#include "RHI/ResourceId.h"

#include "Meta/Optional.h"
#include "Misc/EventHandle.h"
#include "Modular/ModularDomain.h"
#include "Time/Timepoint.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATIONUI_API FImguiService final : public IUIService {
public:
    explicit FImguiService(PImguiContext imguiContext) NOEXCEPT;
    ~FImguiService() override;

    NODISCARD bool Construct(IInputService& input, IRHIService& rhi);
    void TearDown(IInputService& input, IRHIService& rhi);

    const PImguiContext& ImguiContext() const { return _imguiContext; }

    void OnUpdateInput(const IInputService& input, FTimespan dt) override;
    void OnWindowFocus(const IInputService& input, const FGenericWindow* previous) override;

    void OnRenderFrame(const IRHIService& rhi, FTimespan dt) override;
    void OnWindowResized(const IRHIService& rhi, const FRHISurfaceCreateInfo& surface) override;

private:
    NODISCARD RHI::PFrameTask CreateFontTexture_(const RHI::FCommandBufferBatch& cmd);
    NODISCARD RHI::PFrameTask RecreateBuffers_(const RHI::FCommandBufferBatch& cmd);
    NODISCARD RHI::PFrameTask UpdateUniformBuffer_(const RHI::FCommandBufferBatch& cmd);
    NODISCARD RHI::PFrameTask PrepareRenderCommand_(
        RHI::FCommandBufferBatch& cmd,
        RHI::FLogicalPassID renderPass,
        TMemoryView<const RHI::PFrameTask> dependencies);

    const PImguiContext _imguiContext;

    FRgba32f _clearColor;

    RHI::FGPipelineID _pipeline;
    RHI::PPipelineResources _resources;

    RHI::FImageID _fontTexture;
    RHI::FSamplerID _fontSampler;

    RHI::FBufferID _indexBuffer;
    RHI::FBufferID _vertexBuffer;
    RHI::FBufferID _uniformBuffer;

    size_t _indexBufferSize{0};
    size_t _vertexBufferSize{0};

    FEventHandle _onInputUpdate;
    FEventHandle _onRenderFrame;

    FEventHandle _onWindowFocus;
    FEventHandle _onWindowResized;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
