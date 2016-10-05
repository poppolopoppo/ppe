#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPI_fwd.h"
#include "Core.Graphics/Device/IDeviceAPIDiagnostics.h"

#include "Core/Color/Color_fwd.h"
#include "Core/Maths/ScalarRectangle_fwd.h"
#include "Core/Maths/ScalarVector_fwd.h"

namespace Core {
namespace Graphics {
class IDeviceAPIShaderCompiler;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EClearOptions {
    None            = 0,
    Depth           = 1 << 0,
    Stencil         = 1 << 1,
    NotRenderTarget = 1 << 2,
    FDepthStencil    = Depth|Stencil,
    DepthStencil_NotRenderTarget = FDepthStencil|NotRenderTarget,
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIEncapsulator {
public:
    virtual ~IDeviceAPIEncapsulator() {}

    virtual const FAbstractDeviceAPIEncapsulator *APIEncapsulator() const = 0;

    // Viewport

    virtual void SetViewport(const ViewportF& viewport) = 0;
    virtual void SetViewports(const TMemoryView<const ViewportF>& viewports) = 0;

    // Alpha/Raster/Depth State

    virtual FDeviceAPIDependantBlendState *CreateBlendState(FBlendState *state) = 0;
    virtual void DestroyBlendState(FBlendState *state, PDeviceAPIDependantBlendState& entity) = 0;

    virtual FDeviceAPIDependantRasterizerState *CreateRasterizerState(FRasterizerState *state) = 0;
    virtual void DestroyRasterizerState(FRasterizerState *state, PDeviceAPIDependantRasterizerState& entity) = 0;

    virtual FDeviceAPIDependantDepthStencilState *CreateDepthStencilState(FDepthStencilState *state) = 0;
    virtual void DestroyDepthStencilState(FDepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) = 0;

    virtual FDeviceAPIDependantSamplerState *CreateSamplerState(FSamplerState *state) = 0;
    virtual void DestroySamplerState(FSamplerState *state, PDeviceAPIDependantSamplerState& entity) = 0;

    // Index/Vertex Buffer

    virtual FDeviceAPIDependantVertexDeclaration *CreateVertexDeclaration(FVertexDeclaration *declaration) = 0;
    virtual void DestroyVertexDeclaration(FVertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) = 0;

    virtual FDeviceAPIDependantResourceBuffer *CreateIndexBuffer(IndexBuffer *indexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) = 0;
    virtual void DestroyIndexBuffer(IndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) = 0;

    virtual FDeviceAPIDependantResourceBuffer *CreateVertexBuffer(FVertexBuffer *vertexBuffer, FDeviceResourceBuffer *resourceBuffer, const TMemoryView<const u8>& optionalData) = 0;
    virtual void DestroyVertexBuffer(FVertexBuffer *vertexBuffer, PDeviceAPIDependantResourceBuffer& entity) = 0;

    // Shaders

    virtual const FDeviceAPIDependantConstantWriter *ConstantWriter() const = 0;

    virtual FDeviceAPIDependantResourceBuffer *CreateConstantBuffer(FConstantBuffer *constantBuffer, FDeviceResourceBuffer *resourceBuffer) = 0;
    virtual void DestroyConstantBuffer(FConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity) = 0;

    virtual FDeviceAPIDependantShaderProgram* CreateShaderProgram(FShaderProgram* program) = 0;
    virtual void DestroyShaderProgram(FShaderProgram* program, PDeviceAPIDependantShaderProgram& entity) = 0;

    virtual FDeviceAPIDependantShaderEffect *CreateShaderEffect(FShaderEffect *effect) = 0;
    virtual void DestroyShaderEffect(FShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) = 0;

    // Textures

    virtual FDeviceAPIDependantTexture2D *CreateTexture2D(FTexture2D *texture, const TMemoryView<const u8>& optionalData) = 0;
    virtual void DestroyTexture2D(FTexture2D *texture, PDeviceAPIDependantTexture2D& entity) = 0;

    virtual FDeviceAPIDependantTextureCube *CreateTextureCube(FTextureCube *texture, const TMemoryView<const u8>& optionalData) = 0;
    virtual void DestroyTextureCube(FTextureCube *texture, PDeviceAPIDependantTextureCube& entity) = 0;

    // Render target

    virtual FRenderTarget *BackBufferRenderTarget() = 0;
    virtual FDepthStencil *BackBufferDepthStencil() = 0;

    virtual const FRenderTarget *BackBufferRenderTarget() const = 0;
    virtual const FDepthStencil *BackBufferDepthStencil() const = 0;

    virtual void SetRenderTarget(const FRenderTarget *renderTarget, const FDepthStencil *depthStencil) = 0;
    virtual void SetRenderTargets(const TMemoryView<const FRenderTargetBinding>& bindings, const FDepthStencil *depthStencil) = 0;

    virtual void Clear(const FRenderTarget *renderTarget, const ColorRGBAF& color) = 0;
    virtual void Clear(const FDepthStencil *depthStencil, EClearOptions opts, float depth, u8 stencil) = 0;

    virtual FDeviceAPIDependantRenderTarget *CreateRenderTarget(FRenderTarget *renderTarget, const TMemoryView<const u8>& optionalData) = 0;
    virtual void DestroyRenderTarget(FRenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) = 0;

    virtual FDeviceAPIDependantDepthStencil *CreateDepthStencil(FDepthStencil *depthStencil, const TMemoryView<const u8>& optionalData) = 0;
    virtual void DestroyDepthStencil(FDepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) = 0;

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
