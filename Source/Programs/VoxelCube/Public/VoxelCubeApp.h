#pragma once

#include "Application/ApplicationWindow.h"

#include "RHIApi.h"

#include "Input/Action/InputMapping.h"

#include "Mesh/GenericMesh.h"

#include "Viewport/Camera.h"
#include "Viewport/CameraController.h"
#include "Viewport/ViewportClient.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FVoxelCubeApp : public Application::FApplicationWindow {
    typedef Application::FApplicationWindow parent_type;
public:
    explicit FVoxelCubeApp(FModularDomain& domain);
    ~FVoxelCubeApp() override;

    virtual void Start() override;
    virtual void Run() override;
    virtual void Shutdown() override;

    struct FVertexData {
        float3 Position;
        FRgba8u Color;
        //ubyte2n Normal;
        float3 Normal;
    };

    struct FUniformData {
        float4x4 View;
        float4x4 Projection;
        float4x4 ViewProjection;
        float4x4 InvertViewProjection;
    };

protected:
    virtual void Render(RHI::IFrameGraph& fg, FTimespan dt) override;

private:
    bool ReloadContent_(RHI::IFrameGraph& fg);

    bool RecreateRenderTarget_(RHI::IFrameGraph& fg, const uint2& viewportSize);

    bool CreateMeshBuffers_(RHI::IFrameGraph& fg);
    bool CreateUniformBuffers_(RHI::IFrameGraph& fg);
    bool CreateGraphicsPipeline_(RHI::IFrameGraph& fg);

    RHI::TAutoResource<RHI::FImageID> _colorRT;
    RHI::TAutoResource<RHI::FImageID> _depthRT;

    RHI::TAutoResource<RHI::FGPipelineID> _graphicsPpln;

    RHI::TAutoResource<RHI::FBufferID> _indexBuffer;
    RHI::TAutoResource<RHI::FBufferID> _vertexBuffer;

    RHI::PPipelineResources _resources;
    RHI::TAutoResource<RHI::FBufferID> _uniformBuffer;

    RHI::FVertexInputState _vertexInput;

    Application::PCamera _camera;
    Application::PViewportClient _viewport;
    Application::FFreeLookCameraController _freeLookCamera;
    Application::PInputMapping _cameraInputs;

    ContentPipeline::FGenericMesh _genericMesh;

    bool _bRecomputeNormals{ false };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
