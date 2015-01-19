#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"

#include "Core.Graphics/DirectX11/DX11DeviceWrapper.h"

namespace Core {
namespace Graphics {
class PresentationParameters;
} //!namespace Graphics
} //!namespace Core

namespace Core {
namespace Graphics {
namespace DX11 {
FWD_REFPTR(ConstantWriter);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceEncapsulator :
    public AbstractDeviceAPIEncapsulator
,   private IDeviceAPIEncapsulator
,   private IDeviceAPIContextEncapsulator
,   private IDeviceAPIShaderCompilerEncapsulator
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
,   private IDeviceAPIDiagnosticsEncapsulator
#endif
{
public:
    DeviceEncapsulator(Graphics::DeviceEncapsulator *owner, void *windowHandle, const PresentationParameters& pp);
    virtual ~DeviceEncapsulator();

    const DeviceWrapper& Wrapper() const { return _wrapper; }

    virtual IDeviceAPIEncapsulator *Device() const override { return const_cast<DeviceEncapsulator *>(this); }
    virtual IDeviceAPIContextEncapsulator *Context() const override { return const_cast<DeviceEncapsulator *>(this); }
    virtual IDeviceAPIShaderCompilerEncapsulator *Compiler() const override { return const_cast<DeviceEncapsulator *>(this); }
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
    virtual IDeviceAPIDiagnosticsEncapsulator *Diagnostics() const override { return const_cast<DeviceEncapsulator *>(this); }
#endif

    virtual void Reset(const PresentationParameters& pp) override;
    virtual void Present() override;
    virtual void ClearState() override;

private: // IDeviceAPIEncapsulator impl

    virtual const AbstractDeviceAPIEncapsulator *Encapsulator() const override { return this; }

    // Alpha/Raster/Depth State

    virtual DeviceAPIDependantBlendState *CreateBlendState(Graphics::BlendState *state) override;
    virtual void DestroyBlendState(Graphics::BlendState *state, PDeviceAPIDependantBlendState& entity) override;

    virtual DeviceAPIDependantRasterizerState *CreateRasterizerState(Graphics::RasterizerState *state) override;
    virtual void DestroyRasterizerState(Graphics::RasterizerState *state, PDeviceAPIDependantRasterizerState& entity) override;

    virtual DeviceAPIDependantDepthStencilState *CreateDepthStencilState(Graphics::DepthStencilState *state) override;
    virtual void DestroyDepthStencilState(Graphics::DepthStencilState *state, PDeviceAPIDependantDepthStencilState& entity) override;

    virtual DeviceAPIDependantSamplerState *CreateSamplerState(Graphics::SamplerState *state) override;
    virtual void DestroySamplerState(Graphics::SamplerState *state, PDeviceAPIDependantSamplerState& entity) override;

    // Index/Vertex Buffer

    virtual DeviceAPIDependantVertexDeclaration *CreateVertexDeclaration(Graphics::VertexDeclaration *declaration) override;
    virtual void DestroyVertexDeclaration(Graphics::VertexDeclaration *declaration, PDeviceAPIDependantVertexDeclaration& entity) override;

    virtual DeviceAPIDependantResourceBuffer *CreateIndexBuffer(Graphics::IndexBuffer *indexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyIndexBuffer(Graphics::IndexBuffer *indexBuffer, PDeviceAPIDependantResourceBuffer& entity) override;

    virtual DeviceAPIDependantResourceBuffer *CreateVertexBuffer(Graphics::VertexBuffer *vertexBuffer, DeviceResourceBuffer *resourceBuffer, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyVertexBuffer(Graphics::VertexBuffer *vertexBuffer, PDeviceAPIDependantResourceBuffer& entity) override;

    // Shaders

    virtual DeviceAPIDependantResourceBuffer *CreateConstantBuffer(Graphics::ConstantBuffer *constantBuffer, DeviceResourceBuffer *resourceBuffer, PDeviceAPIDependantConstantWriter& writer) override;
    virtual void DestroyConstantBuffer(Graphics::ConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity, PDeviceAPIDependantConstantWriter& writer) override;

    virtual DeviceAPIDependantShaderEffect *CreateShaderEffect(Graphics::ShaderEffect *effect) override;
    virtual void DestroyShaderEffect(Graphics::ShaderEffect *effect, PDeviceAPIDependantShaderEffect& entity) override;

    // Textures

    virtual DeviceAPIDependantTexture2D *CreateTexture2D(Graphics::Texture2D *texture, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyTexture2D(Graphics::Texture2D *texture, PDeviceAPIDependantTexture2D& entity) override;

    virtual DeviceAPIDependantTextureCube *CreateTextureCube(Graphics::TextureCube *texture, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyTextureCube(Graphics::TextureCube *texture, PDeviceAPIDependantTextureCube& entity) override;

    // Render target

    virtual Graphics::RenderTarget *BackBufferRenderTarget() override;
    virtual Graphics::DepthStencil *BackBufferDepthStencil() override;

    virtual const Graphics::RenderTarget *BackBufferRenderTarget() const override;
    virtual const Graphics::DepthStencil *BackBufferDepthStencil() const override;

    virtual DeviceAPIDependantRenderTarget *CreateRenderTarget(Graphics::RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyRenderTarget(Graphics::RenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) override;

    virtual DeviceAPIDependantDepthStencil *CreateDepthStencil(Graphics::DepthStencil *depthStencil, const MemoryView<const u8>& optionalData) override;
    virtual void DestroyDepthStencil(Graphics::DepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) override;

private: // IDeviceAPIContextEncapsulator

    //virtual const AbstractDeviceAPIEncapsulator *Encapsulator() const override { return _deviceAPIDependantEncapsulator.get(); }

    // Alpha/Raster/Depth State

    virtual void SetViewport(const ViewportF& viewport) override;
    virtual void SetViewports(const MemoryView<const ViewportF>& viewports) override;

    virtual void SetBlendState(const Graphics::BlendState *state) override;
    virtual void SetRasterizerState(const Graphics::RasterizerState *state) override;
    virtual void SetDepthStencilState(const Graphics::DepthStencilState *state) override;

    // Index/Vertex Buffer

    virtual void SetIndexBuffer(const Graphics::IndexBuffer *indexBuffer) override;
    virtual void SetIndexBuffer(const Graphics::IndexBuffer *indexBuffer, size_t offset) override;

    virtual void SetVertexBuffer(const Graphics::VertexBuffer *vertexBuffer) override;
    virtual void SetVertexBuffer(const Graphics::VertexBuffer *vertexBuffer, u32 vertexOffset) override;
    virtual void SetVertexBuffer(const MemoryView<const VertexBufferBinding>& bindings) override;

    // Shaders

    virtual void SetShaderEffect(const Graphics::ShaderEffect *effect) override;
    virtual void SetConstantBuffer(ShaderProgramType stage, size_t slot, const Graphics::ConstantBuffer *constantBuffer) override;
    virtual void SetTexture(ShaderProgramType stage, size_t slot, const Graphics::Texture *texture) override;
    virtual void SetSamplerState(ShaderProgramType stage, size_t slot, const Graphics::SamplerState *state) override;

    // Render target

    virtual void SetRenderTarget(const Graphics::RenderTarget *renderTarget, const Graphics::DepthStencil *depthStencil) override;
    virtual void SetRenderTargets(const MemoryView<const RenderTargetBinding>& bindings, const Graphics::DepthStencil *depthStencil) override;

    virtual void Clear(const Graphics::RenderTarget *renderTarget, const ColorRGBAF& color) override;
    virtual void Clear(const Graphics::DepthStencil *depthStencil, ClearOptions opts, float depth, u8 stencil) override;

    // Draw

    virtual void DrawPrimitives(PrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) override;
    virtual void DrawIndexedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) override;
    virtual void DrawInstancedPrimitives(PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) override;

private: // IDeviceAPIShaderCompilerEncapsulator impl

    virtual DeviceAPIDependantShaderProgram *CreateShaderProgram(
        Graphics::ShaderProgram *program,
        const char *entryPoint,
        Graphics::ShaderCompilerFlags flags,
        const Graphics::ShaderSource *source,
        const Graphics::VertexDeclaration *vertexDeclaration) override;
    virtual void DestroyShaderProgram(Graphics::ShaderProgram *program, PDeviceAPIDependantShaderProgram& entity) override;

    virtual void PreprocessShaderProgram(
        RAWSTORAGE(Shader, char)& output,
        const Graphics::ShaderProgram *program,
        const Graphics::ShaderSource *source,
        const Graphics::VertexDeclaration *vertexDeclaration) override;

    virtual void ReflectShaderProgram(
        ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
        VECTOR(Shader, BindName)& textures,
        const Graphics::ShaderProgram *program) override;

#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
private: // IDeviceAPIDiagnosticsEncapsulator() {}

    //virtual const AbstractDeviceAPIEncapsulator *Encapsulator() const override { return _deviceAPIDependantEncapsulator.get(); }

    virtual bool IsProfilerAttached() const override;

    virtual void BeginEvent(const wchar_t *name) override;
    virtual void EndEvent() override;

    virtual void SetMarker(const wchar_t *name) override;
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS

private:
    DeviceWrapper _wrapper;
    PConstantWriter _writer;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FORCE_INLINE const DeviceWrapper *DX11DeviceWrapper(const IDeviceAPIEncapsulator *device) {
    return &checked_cast<const DX11::DeviceEncapsulator *>(device->Encapsulator())->Wrapper();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
