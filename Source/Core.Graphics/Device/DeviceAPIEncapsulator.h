#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Color/Color.h"
#include "Core/Container/AssociativeVector.h"
#include "Core/Container/RawStorage.h"
#include "Core/Maths/Geometry/ScalarRectangle.h"
#include "Core/Maths/Geometry/ScalarVector.h"

#ifndef FINAL_RELEASE
#   define WITH_CORE_GRAPHICS_DIAGNOSTICS
#endif

namespace Core {
    template <typename T>
    class MemoryView;
}

namespace Core {
namespace Graphics {
enum class DeviceAPI;
class AbstractDeviceAPIEncapsulator;

class PresentationParameters;

class DeviceResource;
class DeviceResourceBuffer;
FWD_REFPTR(DeviceAPIDependantResourceBuffer);

class BlendState;
FWD_REFPTR(DeviceAPIDependantBlendState);
class RasterizerState;
FWD_REFPTR(DeviceAPIDependantRasterizerState);
class DepthStencilState;
FWD_REFPTR(DeviceAPIDependantDepthStencilState);
class SamplerState;
FWD_REFPTR(DeviceAPIDependantSamplerState);

enum class PrimitiveType;
class IndexBuffer;
class VertexBuffer;
struct VertexBufferBinding;
class VertexDeclaration;
FWD_REFPTR(DeviceAPIDependantVertexDeclaration);

class BindName;
class ConstantBuffer;
FWD_REFPTR(ConstantBufferLayout);
FWD_REFPTR(DeviceAPIDependantConstantWriter);
enum class ShaderCompilerFlags;
class ShaderProgram;
enum class ShaderProgramType;
FWD_REFPTR(DeviceAPIDependantShaderProgram);
class ShaderEffect;
FWD_REFPTR(DeviceAPIDependantShaderEffect);
class ShaderSource;

class SurfaceFormat;

class Texture;
FWD_REFPTR(DeviceAPIDependantTexture);
class Texture2D;
FWD_REFPTR(DeviceAPIDependantTexture2D);
class TextureCube;
FWD_REFPTR(DeviceAPIDependantTextureCube);
class RenderTarget;
FWD_REFPTR(DeviceAPIDependantRenderTarget);
class DepthStencil;
FWD_REFPTR(DeviceAPIDependantDepthStencil);
struct RenderTargetBinding;

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

    virtual const AbstractDeviceAPIEncapsulator *Encapsulator() const = 0;

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

    virtual DeviceAPIDependantResourceBuffer *CreateConstantBuffer(ConstantBuffer *constantBuffer, DeviceResourceBuffer *resourceBuffer, PDeviceAPIDependantConstantWriter& writer) = 0;
    virtual void DestroyConstantBuffer(ConstantBuffer *constantBuffer, PDeviceAPIDependantResourceBuffer& entity, PDeviceAPIDependantConstantWriter& writer) = 0;

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

    virtual DeviceAPIDependantRenderTarget *CreateRenderTarget(RenderTarget *renderTarget, const MemoryView<const u8>& optionalData) = 0;
    virtual void DestroyRenderTarget(RenderTarget *renderTarget, PDeviceAPIDependantRenderTarget& entity) = 0;

    virtual DeviceAPIDependantDepthStencil *CreateDepthStencil(DepthStencil *depthStencil, const MemoryView<const u8>& optionalData) = 0;
    virtual void DestroyDepthStencil(DepthStencil *depthStencil, PDeviceAPIDependantDepthStencil& entity) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIContextEncapsulator {
public:
    virtual ~IDeviceAPIContextEncapsulator() {}

    virtual const AbstractDeviceAPIEncapsulator *Encapsulator() const = 0;

    // Alpha/Raster/Depth State

    virtual void SetViewport(const ViewportF& viewport) = 0;
    virtual void SetViewports(const MemoryView<const ViewportF>& viewports) = 0;

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
    virtual void SetTexture(ShaderProgramType stage, size_t slot, const Texture *texture) = 0;
    virtual void SetSamplerState(ShaderProgramType stage, size_t slot, const SamplerState *state) = 0;

    // Render target

    virtual void SetRenderTarget(const RenderTarget *renderTarget, const DepthStencil *depthStencil) = 0;
    virtual void SetRenderTargets(const MemoryView<const RenderTargetBinding>& bindings, const DepthStencil *depthStencil) = 0;

    virtual void Clear(const RenderTarget *renderTarget, const ColorRGBAF& color) = 0;
    virtual void Clear(const DepthStencil *depthStencil, ClearOptions opts, float depth, u8 stencil) = 0;

    // Draw

    virtual void DrawPrimitives(PrimitiveType primitiveType, size_t startVertex, size_t primitiveCount) = 0;
    virtual void DrawIndexedPrimitives(Graphics::PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount) = 0;
    virtual void DrawInstancedPrimitives(Graphics::PrimitiveType primitiveType, size_t baseVertex, size_t startIndex, size_t primitiveCount, size_t startInstance, size_t instanceCount) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class IDeviceAPIShaderCompilerEncapsulator {
public:
    virtual ~IDeviceAPIShaderCompilerEncapsulator() {}

    virtual const AbstractDeviceAPIEncapsulator *Encapsulator() const = 0;

    virtual DeviceAPIDependantShaderProgram *CreateShaderProgram(
        ShaderProgram *program,
        const char *entryPoint,
        ShaderCompilerFlags flags,
        const ShaderSource *source,
        const VertexDeclaration *vertexDeclaration) = 0;
    virtual void DestroyShaderProgram(ShaderProgram *program, PDeviceAPIDependantShaderProgram& entity) = 0;

    virtual void PreprocessShaderProgram(
        RAWSTORAGE(Shader, char)& output,
        const ShaderProgram *program,
        const ShaderSource *source,
        const VertexDeclaration *vertexDeclaration) = 0;

    virtual void ReflectShaderProgram(
        ASSOCIATIVE_VECTOR(Shader, BindName, PCConstantBufferLayout)& constants,
        VECTOR(Shader, BindName)& textures,
        const ShaderProgram *program) = 0;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
class IDeviceAPIDiagnosticsEncapsulator {
public:
    virtual ~IDeviceAPIDiagnosticsEncapsulator() {}

    virtual const AbstractDeviceAPIEncapsulator *Encapsulator() const = 0;

    virtual bool IsProfilerAttached() const = 0;

    virtual void BeginEvent(const wchar_t *name) = 0;
    virtual void EndEvent() = 0;

    virtual void SetMarker(const wchar_t *name) = 0;
};
#endif //!WITH_CORE_GRAPHICS_DIAGNOSTICS
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
