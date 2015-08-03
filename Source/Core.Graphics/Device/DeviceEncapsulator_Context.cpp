#include "stdafx.h"

#include "DeviceEncapsulator.h"

#include "AbstractDeviceAPIEncapsulator.h"

#include "Geometry/IndexBuffer.h"
#include "Geometry/PrimitiveType.h"
#include "Geometry/VertexBuffer.h"
#include "Geometry/VertexDeclaration.h"

#include "Shader/ConstantBuffer.h"
#include "Shader/ShaderEffect.h"
#include "Shader/ShaderProgram.h"

#include "State/BlendState.h"
#include "State/DepthStencilState.h"
#include "State/RasterizerState.h"
#include "State/SamplerState.h"

#include "Texture/DepthStencil.h"
#include "Texture/RenderTarget.h"
#include "Texture/Texture.h"
#include "Texture/Texture2D.h"
#include "Texture/TextureCube.h"

#include "Core/Diagnostic/Logger.h"

// IDeviceAPIContext

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Alpha/Raster/Depth State
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetBlendState(const BlendState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    state->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetBlendState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetRasterizerState(const RasterizerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    state->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetRasterizerState(state);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetDepthStencilState(const DepthStencilState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    state->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetDepthStencilState(state);
}
//----------------------------------------------------------------------------
// SamplerState
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetSamplerState(ShaderProgramType stage, size_t slot, const SamplerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

    state->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetSamplerState(stage, slot, state);
}

//----------------------------------------------------------------------------
void DeviceEncapsulator::SetSamplerStates(ShaderProgramType stage, const MemoryView<const SamplerState *>& states) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (const SamplerState *state : states) {
        Assert(state);
        Assert(state->Frozen());
        Assert(state->Available());
        state->TerminalEntity()->SetLastUsed(_revision);
    }

    _deviceAPIEncapsulator->Immediate()->SetSamplerStates(stage, states);
}
//----------------------------------------------------------------------------
// IndexBuffer
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetIndexBuffer(const IndexBuffer *indexBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(indexBuffer->Available());

    indexBuffer->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetIndexBuffer(indexBuffer);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetIndexBuffer(const IndexBuffer *indexBuffer, size_t offset) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(indexBuffer->Available());
    Assert(offset < indexBuffer->IndexCount());

    indexBuffer->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetIndexBuffer(indexBuffer, offset);
}
//----------------------------------------------------------------------------
// VertexBuffer
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetVertexBuffer(const VertexBuffer *vertexBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(vertexBuffer->Available());

    vertexBuffer->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetVertexBuffer(vertexBuffer);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetVertexBuffer(const VertexBuffer *vertexBuffer, u32 vertexOffset) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(vertexBuffer->Available());
    Assert(vertexOffset < vertexBuffer->VertexCount());

    vertexBuffer->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetVertexBuffer(vertexBuffer, vertexOffset);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetVertexBuffer(const MemoryView<const VertexBufferBinding>& bindings) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(bindings.size());

    for (const VertexBufferBinding& b : bindings) {
        Assert(b.VertexBuffer);
        Assert(b.VertexBuffer->Frozen());
        Assert(b.VertexBuffer->Available());
        Assert(b.VertexOffset < b.VertexBuffer->VertexCount());
        b.VertexBuffer->TerminalEntity()->SetLastUsed(_revision);
    }

    _deviceAPIEncapsulator->Immediate()->SetVertexBuffer(bindings);
}
//----------------------------------------------------------------------------
// Shaders
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetConstantBuffer(ShaderProgramType stage, size_t slot, const ConstantBuffer *constantBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(constantBuffer);
    Assert(constantBuffer->Frozen());
    Assert(constantBuffer->Available());

    constantBuffer->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetConstantBuffer(stage, slot, constantBuffer);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetConstantBuffers(ShaderProgramType stage, const MemoryView<const ConstantBuffer *>& constantBuffers) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (const ConstantBuffer *constantBuffer : constantBuffers) {
        Assert(constantBuffer);
        Assert(constantBuffer->Frozen());
        Assert(constantBuffer->Available());
        constantBuffer->TerminalEntity()->SetLastUsed(_revision);
    }

    _deviceAPIEncapsulator->Immediate()->SetConstantBuffers(stage, constantBuffers);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetShaderEffect(const ShaderEffect *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());
    Assert(effect->Available());

    effect->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetShaderEffect(effect);
}
//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetTexture(ShaderProgramType stage, size_t slot, const Texture *texture) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!texture || texture->Frozen());
    Assert(!texture || texture->Available());

    texture->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetTexture(stage, slot, texture);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::SetTextures(ShaderProgramType stage, const MemoryView<const Texture *>& textures) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (const Texture *texture : textures) {
        if (!texture) continue;
        Assert(texture->Frozen());
        Assert(texture->Available());
        texture->TerminalEntity()->SetLastUsed(_revision);
    }

    _deviceAPIEncapsulator->Immediate()->SetTextures(stage, textures);
}
//----------------------------------------------------------------------------
// Draw
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawPrimitives(PrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->Immediate()->DrawPrimitives(primitiveType, startVertex, primitiveCount);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawIndexedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->Immediate()->DrawIndexedPrimitives(primitiveType, baseVertex, startIndex, primitiveCount);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::DrawInstancedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->Immediate()->DrawInstancedPrimitives(primitiveType, baseVertex, startIndex, primitiveCount, startInstance, instanceCount);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
