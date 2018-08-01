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

#include "Diagnostic/Logger.h"

// IDeviceAPIContext

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS
//#   define WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT //%_NOCOMMIT%
#endif

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Alpha/Raster/Depth State
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetBlendState(const FBlendState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), state ? state->ResourceName() : L"Null");
#endif

    state->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetBlendState(state);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetRasterizerState(const FRasterizerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), state ? state->ResourceName() : L"Null");
#endif

    state->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetRasterizerState(state);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetDepthStencilState(const FDepthStencilState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), state ? state->ResourceName() : L"Null");
#endif

    state->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetDepthStencilState(state);
}
//----------------------------------------------------------------------------
// FSamplerState
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetSamplerState(EShaderProgramType stage, size_t slot, const FSamplerState *state) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(state);
    Assert(state->Frozen());
    Assert(state->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), state ? state->ResourceName() : L"Null");
#endif

    state->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetSamplerState(stage, slot, state);
}

//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetSamplerStates(EShaderProgramType stage, const TMemoryView<const FSamplerState *>& states) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (const FSamplerState *state : states) {
        Assert(state);
        Assert(state->Frozen());
        Assert(state->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
        GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), state ? state->ResourceName() : L"Null");
#endif

        state->TerminalEntity()->SetLastUsed(_revision);
    }

    _deviceAPIEncapsulator->Immediate()->SetSamplerStates(stage, states);
}
//----------------------------------------------------------------------------
// IndexBuffer
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetIndexBuffer(const FIndexBuffer *indexBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(indexBuffer->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), indexBuffer ? indexBuffer->ResourceName() : L"Null");
#endif

    indexBuffer->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetIndexBuffer(indexBuffer);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetIndexBuffer(const FIndexBuffer *indexBuffer, size_t offset) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(indexBuffer);
    Assert(indexBuffer->Frozen());
    Assert(indexBuffer->Available());
    Assert(offset < indexBuffer->IndexCount());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), indexBuffer ? indexBuffer->ResourceName() : L"Null");
#endif

    indexBuffer->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetIndexBuffer(indexBuffer, offset);
}
//----------------------------------------------------------------------------
// FVertexBuffer
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetVertexBuffer(const FVertexBuffer *vertexBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(vertexBuffer->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), vertexBuffer ? vertexBuffer->ResourceName() : L"Null");
#endif

    vertexBuffer->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetVertexBuffer(vertexBuffer);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetVertexBuffer(const FVertexBuffer *vertexBuffer, u32 vertexOffset) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(vertexBuffer);
    Assert(vertexBuffer->Frozen());
    Assert(vertexBuffer->Available());
    Assert(vertexOffset < vertexBuffer->VertexCount());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), vertexBuffer ? vertexBuffer->ResourceName() : L"Null");
#endif

    vertexBuffer->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetVertexBuffer(vertexBuffer, vertexOffset);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetVertexBuffer(const TMemoryView<const FVertexBufferBinding>& bindings) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(bindings.size());

    for (const FVertexBufferBinding& b : bindings) {
        Assert(b.VertexBuffer);
        Assert(b.VertexBuffer->Frozen());
        Assert(b.VertexBuffer->Available());
        Assert(b.VertexOffset < b.VertexBuffer->VertexCount());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
        GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), b.VertexBuffer ? b.VertexBuffer->ResourceName() : L"Null");
#endif

        b.VertexBuffer->TerminalEntity()->SetLastUsed(_revision);
    }

    _deviceAPIEncapsulator->Immediate()->SetVertexBuffer(bindings);
}
//----------------------------------------------------------------------------
// Shaders
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetConstantBuffer(EShaderProgramType stage, size_t slot, const FConstantBuffer *constantBuffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(constantBuffer);
    Assert(constantBuffer->Frozen());
    Assert(constantBuffer->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), constantBuffer ? constantBuffer->ResourceName() : L"Null");
#endif

    constantBuffer->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetConstantBuffer(stage, slot, constantBuffer);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetConstantBuffers(EShaderProgramType stage, const TMemoryView<const FConstantBuffer *>& constantBuffers) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (const FConstantBuffer *constantBuffer : constantBuffers) {
        Assert(constantBuffer);
        Assert(constantBuffer->Frozen());
        Assert(constantBuffer->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
        GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), constantBuffer ? constantBuffer->ResourceName() : L"Null");
#endif

        constantBuffer->TerminalEntity()->SetLastUsed(_revision);
    }

    _deviceAPIEncapsulator->Immediate()->SetConstantBuffers(stage, constantBuffers);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetShaderEffect(const FShaderEffect *effect) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(effect);
    Assert(effect->Frozen());
    Assert(effect->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), effect ? effect->ResourceName() : L"Null");
#endif

    effect->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetShaderEffect(effect);
}
//----------------------------------------------------------------------------
// Textures
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetTexture(EShaderProgramType stage, size_t slot, const FTexture *texture) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!texture || texture->Frozen());
    Assert(!texture || texture->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
    GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), texture ? texture->ResourceName() : L"Null");
#endif

    texture->TerminalEntity()->SetLastUsed(_revision);
    _deviceAPIEncapsulator->Immediate()->SetTexture(stage, slot, texture);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::SetTextures(EShaderProgramType stage, const TMemoryView<const FTexture *>& textures) {
    THIS_THREADRESOURCE_CHECKACCESS();

    for (const FTexture *texture : textures) {
        if (!texture) continue;
        Assert(texture->Frozen());
        Assert(texture->Available());

#ifdef WITH_PPE_GRAPHICS_DIAGNOSTICS_FOR_CONTEXT
        GRAPHICS_DIAGNOSTICS_SETMARKER(_deviceAPIEncapsulator->Diagnostics(), texture ? texture->ResourceName() : L"Null");
#endif

        texture->TerminalEntity()->SetLastUsed(_revision);
    }

    _deviceAPIEncapsulator->Immediate()->SetTextures(stage, textures);
}
//----------------------------------------------------------------------------
// Draw
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DrawPrimitives(EPrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->Immediate()->DrawPrimitives(primitiveType, startVertex, primitiveCount);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DrawIndexedPrimitives(EPrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->Immediate()->DrawIndexedPrimitives(primitiveType, baseVertex, startIndex, primitiveCount);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::DrawInstancedPrimitives(EPrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->Immediate()->DrawInstancedPrimitives(primitiveType, baseVertex, startIndex, primitiveCount, startInstance, instanceCount);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
