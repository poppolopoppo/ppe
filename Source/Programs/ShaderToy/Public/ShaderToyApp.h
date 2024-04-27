#pragma once

#include "Application/ApplicationWindow.h"

#include "UI/Widgets/FileDialogWidget.h"
#include "UI/Widgets/LogViewerWidget.h"
#include "UI/Widgets/MemoryUsageWidget.h"

#include "Widgets/ImportTextureWidget.h"

#include "Texture_fwd.h"

#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "IO/Filename.h"
#include "Memory/UniquePtr.h"
#include "Time/DateTime.h"
#include "Time/Timeline.h"

#include "RHIApi.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FShaderToyApp : public Application::FApplicationWindow {
    typedef Application::FApplicationWindow parent_type;
public:
    explicit FShaderToyApp(FModularDomain& domain);
    ~FShaderToyApp() override;

    virtual void Start() override;
    virtual void Run() override;
    virtual void Shutdown() override;

    struct FPushConstantData {
        // Frame
        float   iTime;                  // Current time in seconds
        float   iTimeDelta;             // Time it takes to render a frame, in seconds
        int     iFrame;                 // Current frame
        float   iFrameRate;             // Number of frames rendered per second
        float4  iMouse;                 // xy = current pixel coords (if LMB is down). zw = click pixel
        // Buffer
        float4  iResolution;            // Width / Height / WidthOO / HeightOO
        float4  iChannelResolution[4];  // Width / Height / WidthOO / HeightOO
    };

    FWD_REFPTR(Buffer);
    FWD_REFPTR(Source);

    class FSource : public FRefCountable {
    public:
        RHI::TAutoResource<RHI::FImageID> Image;

        RHI::FImageDesc Desc;
        FString DebugName;

        SCBuffer OptionalBufferRef;
    };

    struct FInput {
        RHI::TAutoResource<RHI::FSamplerID> Sampler;

        PCSource Source;
    };

    class FBuffer : public FRefCountable {
    public:
        RHI::PFrameTask LastTask;
        RHI::PPipelineResources Resources;

        RHI::TAutoResource<RHI::FGPipelineID> Pipeline;
        RHI::TAutoResource<RHI::FImageID> RenderTarget;
        RHI::TAutoResource<RHI::FBufferID> UniformBuffer;

        FTimestamp LastModified;
        uint2 Resolution;

        FInput Inputs[4];

        FString DebugName;
        FFilename FragmentSource;

        bool bVisible{ true };

        NODISCARD bool Construct(const FShaderToyApp& app, const FFilename& fragmentSource);
        void TearDown(const FShaderToyApp& app);

        NODISCARD bool RecreatePipeline(RHI::IFrameGraph& fg, const RHI::FGraphicsPipelineDesc::FShader& vertexShader);
        NODISCARD bool RecreateRenderTarget(RHI::IFrameGraph& fg, const uint2& viewportSize);

        NODISCARD RHI::PFrameTask UpdateUniformBuffer(const RHI::FCommandBufferBatch& cmd, const FPushConstantData& frameData);
        NODISCARD RHI::PFrameTask RenderFrame(
            const FShaderToyApp& app,
            const RHI::FCommandBufferBatch& cmd,
            const FPushConstantData& frameData);

        void RenderUI(FShaderToyApp& app, RHI::IFrameGraph& fg);
    };

protected:
    virtual void Update(FTimespan dt) override;
    virtual void Render(RHI::IFrameGraph& fg, FTimespan dt) override;

    FBuffer _main;
    VECTOR(UserDomain, PBuffer) _buffers;
    HASHMAP(UserDomain, FFilename, PSource) _sources;

    PSource _dummySource;
    RHI::FGraphicsPipelineDesc::FShader _vertexShader;

    FTimeline _refreshCooldown;
    bool _needRefresh{ true };

    TUniquePtr<Application::FImportTextureWidget> _importTexture;

    TUniquePtr<Application::FFileDialogWidget> _fileDialog;
    TUniquePtr<Application::FMemoryUsageWidget> _memoryUsage;
#if USE_PPE_LOGGER
    TUniquePtr<Application::FLogViewerWidget> _logViewer;
#endif

    FTimeline _shaderTime;
    float _shaderTimeSpeed{ 1.0f };

    ImGuiID _dockspaceLeft = Zero;
    ImGuiID _dockspaceBottom = Zero;

    bool bImGuiShowDemoWindow{ false };
    bool bImGuiShowUserGuide{ false };

    bool bImPlotShowDemoWindow{ false };
    bool bImPlotShowUserGuide{ false };

    NODISCARD bool CreateDummySource_(RHI::IFrameGraph& fg, const uint2& size, RHI::EPixelFormat format);
    NODISCARD PCSource CreateTextureSource_(RHI::IFrameGraph& fg, const FFilename& texturePath);
    NODISCARD PCSource CreateTextureSource_(RHI::IFrameGraph& fg, const ContentPipeline::PTexture& texture);
    NODISCARD bool RecreateRenderTargets_(RHI::IFrameGraph& fg, const uint2& viewportSize);

    void StartInterafaceWidgets_();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
