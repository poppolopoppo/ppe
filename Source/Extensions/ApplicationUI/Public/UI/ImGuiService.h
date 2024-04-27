#pragma once

#include "ApplicationUI_fwd.h"

#include "UI/UIService.h"

#include "Input/Action/InputListener.h"

#include "RHI_fwd.h"
#include "RHI/ResourceId.h"

#include "Meta/Optional.h"
#include "Misc/EventHandle.h"
#include "Modular/ModularDomain.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_APPLICATIONUI_API FImGuiService final : public IUIService {
public:
    FImGuiService();
    ~FImGuiService() override;

    NODISCARD bool Construct(IApplicationService& app, IInputService& input, IRHIService& rhi);
    void TearDown(IApplicationService& app, IInputService& input, IRHIService& rhi);

    const PImGuiContext& ImGuiContext() const { return _imGuiContext; }

    void OnBeginTick(const IApplicationService& app);
    void OnEndTick(const IApplicationService& app);

    void OnUpdateInput(const IInputService& input, FTimespan dt);
    void OnWindowFocus(const IInputService& input, const FGenericWindow* previous);

    void OnRenderFrame(const IRHIService& rhi, FTimespan dt);
    void OnWindowResized(const IRHIService& rhi, const FRHISurfaceCreateInfo& surface);

public: // IUIService

    virtual void ToggleFocus(IInputService& inputs, bool selected, int priority = 0) override;

private:
    void InitializeImGuiStyle_();

    NODISCARD RHI::PFrameTask CreateFontTexture_(const RHI::FCommandBufferBatch& cmd);
    NODISCARD RHI::PFrameTask RecreateBuffers_(const RHI::FCommandBufferBatch& cmd);
    NODISCARD RHI::PFrameTask UpdateUniformBuffer_(const RHI::FCommandBufferBatch& cmd);
    NODISCARD RHI::PFrameTask PrepareRenderCommand_(
        RHI::IFrameGraph& fg,
        RHI::FCommandBufferBatch& cmd,
        RHI::FLogicalPassID renderPass,
        TMemoryView<const RHI::PFrameTask> dependencies);

    NODISCARD RHI::PPipelineResources FindOrAddTextureResources_(RHI::IFrameGraph& fg, const RHI::FImageID& texture);
    void GCUnusedTextureResources_(RHI::IFrameGraph& fg);

    PInputMapping _inputMapping;
    PInputAction _inputSink;

    PImGuiContext _imGuiContext{ nullptr };
    PImPlotContext _imPlotContext{ nullptr };

    FRgba32f _clearColor;

    RHI::FGPipelineID _pipeline;

    ASSOCIATIVE_VECTORINSITU(ImGui, RHI::FResourceHandle, RHI::PPipelineResources, 5) _textureResources;

    RHI::FImageID _fontTexture;
    RHI::FSamplerID _textureSampler;

    RHI::FBufferID _indexBuffer;
    RHI::FBufferID _vertexBuffer;
    RHI::FBufferID _uniformBuffer;

    size_t _indexBufferSize{0};
    size_t _vertexBufferSize{0};

    FEventHandle _onBeginTick;
    FEventHandle _onEndTick;

    FEventHandle _onInputUpdate;
    FEventHandle _onRenderFrame;

    FEventHandle _onWindowFocus;
    FEventHandle _onWindowResized;

    bool _wasFrameRendered{ false };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
