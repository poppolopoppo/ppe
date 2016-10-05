#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI_fwd.h"
#include "Core.Graphics/Device/IDeviceAPIDiagnostics.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIContext {
public:
    virtual ~IDeviceAPIContext() {}

    virtual const FAbstractDeviceAPIEncapsulator *APIEncapsulator() const = 0;

    // Alpha/Raster/Depth State

    virtual void SetBlendState(const FBlendState *state) = 0;
    virtual void SetRasterizerState(const FRasterizerState *state) = 0;
    virtual void SetDepthStencilState(const FDepthStencilState *state) = 0;

    // Index/Vertex Buffer

    virtual void SetIndexBuffer(const IndexBuffer *indexBuffer) = 0;
    virtual void SetIndexBuffer(const IndexBuffer *indexBuffer, size_t offset) = 0;

    virtual void SetVertexBuffer(const FVertexBuffer *vertexBuffer) = 0;
    virtual void SetVertexBuffer(const FVertexBuffer *vertexBuffer, u32 vertexOffset) = 0;
    virtual void SetVertexBuffer(const TMemoryView<const FVertexBufferBinding>& bindings) = 0;

    // Shaders

    virtual void SetShaderEffect(const FShaderEffect *effect) = 0;

    virtual void SetConstantBuffer(EShaderProgramType stage, size_t slot, const FConstantBuffer *constantBuffer) = 0;
    virtual void SetConstantBuffers(EShaderProgramType stage, const TMemoryView<const FConstantBuffer *>& constantBuffers) = 0;

    virtual void SetTexture(EShaderProgramType stage, size_t slot, const FTexture *texture) = 0;
    virtual void SetTextures(EShaderProgramType stage, const TMemoryView<const FTexture *>& textures) = 0;
    
    virtual void SetSamplerState(EShaderProgramType stage, size_t slot, const FSamplerState *state) = 0;
    virtual void SetSamplerStates(EShaderProgramType stage, const TMemoryView<const FSamplerState *>& states) = 0;

    // Draw

    virtual void DrawPrimitives(EPrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) = 0;
    virtual void DrawIndexedPrimitives(Graphics::EPrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) = 0;
    virtual void DrawInstancedPrimitives(Graphics::EPrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
