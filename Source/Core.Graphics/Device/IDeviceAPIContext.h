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

    virtual const AbstractDeviceAPIEncapsulator *APIEncapsulator() const = 0;

    // Alpha/Raster/Depth State

    virtual void SetBlendState(const BlendState *state) = 0;
    virtual void SetRasterizerState(const RasterizerState *state) = 0;
    virtual void SetDepthStencilState(const DepthStencilState *state) = 0;

    // Index/Vertex Buffer

    virtual void SetIndexBuffer(const IndexBuffer *indexBuffer) = 0;
    virtual void SetIndexBuffer(const IndexBuffer *indexBuffer, size_t offset) = 0;

    virtual void SetVertexBuffer(const VertexBuffer *vertexBuffer) = 0;
    virtual void SetVertexBuffer(const VertexBuffer *vertexBuffer, u32 vertexOffset) = 0;
    virtual void SetVertexBuffer(const MemoryView<const VertexBufferBinding>& bindings) = 0;

    // Shaders

    virtual void SetShaderEffect(const ShaderEffect *effect) = 0;

    virtual void SetConstantBuffer(ShaderProgramType stage, size_t slot, const ConstantBuffer *constantBuffer) = 0;
    virtual void SetConstantBuffers(ShaderProgramType stage, const MemoryView<const ConstantBuffer *>& constantBuffers) = 0;

    virtual void SetTexture(ShaderProgramType stage, size_t slot, const Texture *texture) = 0;
    virtual void SetTextures(ShaderProgramType stage, const MemoryView<const Texture *>& textures) = 0;
    
    virtual void SetSamplerState(ShaderProgramType stage, size_t slot, const SamplerState *state) = 0;
    virtual void SetSamplerStates(ShaderProgramType stage, const MemoryView<const SamplerState *>& states) = 0;

    // Draw

    virtual void DrawPrimitives(PrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) = 0;
    virtual void DrawIndexedPrimitives(Graphics::PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) = 0;
    virtual void DrawInstancedPrimitives(Graphics::PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
