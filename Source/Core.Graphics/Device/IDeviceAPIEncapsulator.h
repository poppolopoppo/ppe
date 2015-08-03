#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI_fwd.h"
#include "Core.Graphics/Device/IDeviceAPIDiagnostics.h"

#include "Core/Color/Color_fwd.h"
#include "Core/Maths/Geometry/ScalarRectangle_fwd.h"
#include "Core/Maths/Geometry/ScalarVector_fwd.h"

namespace Core {
namespace Graphics {
class IDeviceAPIShaderCompiler;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ClearOptions {
    None            = 0,
    Depth           = 1 << 0,
    Stencil         = 1 << 1,
    NotRenderTarget = 1 << 2,
    DepthStencil    = Depth|Stencil,
    DepthStencil_NotRenderTarget = DepthStencil|NotRenderTarget,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIEncapsulator {
public:
    virtual ~IDeviceAPIEncapsulator() {}

    virtual const AbstractDeviceAPIEncapsulator *APIEncapsulator() const = 0;

    // Viewport

    virtual void SetViewport(const ViewportF& viewport) = 0;
    virtual void SetViewports(const MemoryView<const ViewportF>& viewports) = 0;

    // Alpha/Raster/Depth State

    virtual DeviceAPIDependantBlendState *CreateBlendState(BlendState *state) = 0;
    virtual void DestroyBlendState(BlendState *state, PDeviceAPIDependantBlendState& entity) = 0;

    virtual DeviceAPIDependantRasterizerState *CreateRasterizerState(RasterizerState *state) = 0;
    virtual void DestroyRasterizerState(RasterizerState *state, PDeviceAPIDependantRasterizerState& entity) = 0;

    virtual DeviceAPIDependantDepthStencilState *CreateDepthStencilState(DepthStencilState *state) = 0;
    virtual void DestroyDepthStencilState(DepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) = 0;

    virtual DeviceAPIDependantSamplerState *CreateSamplerState(SamplerState *state) = 0;
    virtual void DestroySamplerState(SamplerState *state, PDeviceAPIDependantSamplerState& entity) = 0;

    // Index/Vertex Buffer

    virtual DeviceAPIDependantVertexDeclaration *CreateVertexDeclaration(VertexDeclaration *declaration) = 0;
    virtual void DestroyVertexDeclaration(VertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) = 0;

    virtual DeviceAPIDependantResourceBuffer *CreateIndexBuffer(IndexBuffer *indexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) = 0;
    virtual void DestroyIndexBuffer(IndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) = 0;

    virtual DeviceAPIDependantResourceBuffer *CreateVertexBuffer(VertexBuffer *vertexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) = 0;
    virtual void DestroyVertexBuffer(VertexBuffer *vertexBuffer, PDeviceAPIDependantResourceBuffer& entity) = 0;

    // Shaders

    virtual const DeviceAPIDependantConstantWriter *ConstantWriter() const = 0;

    virtual DeviceAPIDependantResourceBuffer *CreateConstantBuffer(ConstantBuffer *constantBuffer, DeviceResourceBuffer *resourceBuffer) = 0;
    virtual void DestroyConstantBuffer(ConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity) = 0;

    virtual DeviceAPIDependantShaderEffect *CreateShaderEffect(ShaderEffect *effect) = 0;
    virtual void DestroyShaderEffect(ShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) = 0;

    // Textures

    virtual DeviceAPIDependantTexture2D *CreateTexture2D(Texture2D *texture, const MemoryView<const u8>& optionalData) = 0;
    virtual void DestroyTexture2D(Texture2D *texture, PDeviceAPIDependantTexture2D& entity) = 0;

    virtual DeviceAPIDependantTextureCube *CreateTextureCube(TextureCube *texture, const MemoryView<const u8>& optionalData) = 0;
    virtual void DestroyTextureCube(TextureCube *texture, PDeviceAPIDependantTextureCube& entity) = 0;

    // Render target

    virtual RenderTarget *BackBufferRenderTarget() = 0;
    virtual DepthStencil *BackBufferDepthStencil() = 0;

    virtual const RenderTarget *BackBufferRenderTarget() const = 0;
    virtual const DepthStencil *BackBufferDepthStencil() const = 0;

    virtual void SetRenderTarget(const RenderTarget *renderTarget, const DepthStencil *depthStencil) = 0;
    virtual void SetRenderTargets(const MemoryView<const RenderTargetBinding>& bindings, const DepthStencil *depthStencil) = 0;

    virtual void Clear(const RenderTarget *renderTarget, const ColorRGBAF& color) = 0;
    virtual void Clear(const DepthStencil *depthStencil, ClearOptions opts, float depth, u8 stencil) = 0;

    virtual DeviceAPIDependantRenderTarget *CreateRenderTarget(RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) = 0;
    virtual void DestroyRenderTarget(RenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) = 0;

    virtual DeviceAPIDependantDepthStencil *CreateDepthStencil(DepthStencil *depthStencil, const MemoryView<const u8>& optionalData) = 0;
    virtual void DestroyDepthStencil(DepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) = 0;

    // Diagnostics
    
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnostics *DeviceDiagnostics() const = 0;
#endif
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
