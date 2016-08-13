#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Memory/RefPtr.h"

namespace Core {
    template <typename T>
    class MemoryView;
}

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PresentationParameters;
//----------------------------------------------------------------------------
class DeviceResource;
class DeviceResourceBuffer;
FWD_REFPTR(DeviceAPIDependantResourceBuffer);
//----------------------------------------------------------------------------
class BlendState;
FWD_REFPTR(DeviceAPIDependantBlendState);
class RasterizerState;
FWD_REFPTR(DeviceAPIDependantRasterizerState);
class DepthStencilState;
FWD_REFPTR(DeviceAPIDependantDepthStencilState);
class SamplerState;
FWD_REFPTR(DeviceAPIDependantSamplerState);
//----------------------------------------------------------------------------
enum class PrimitiveType;
class IndexBuffer;
class VertexBuffer;
struct VertexBufferBinding;
class VertexDeclaration;
FWD_REFPTR(DeviceAPIDependantVertexDeclaration);
//----------------------------------------------------------------------------
class Name;
class ConstantBuffer;
FWD_REFPTR(ConstantBufferLayout);
FWD_REFPTR(DeviceAPIDependantConstantWriter);
enum class ShaderProfileType;
class ShaderProgram;
struct ShaderProgramTexture;
enum class ShaderTextureDimension;
enum class ShaderProgramType;
FWD_REFPTR(DeviceAPIDependantShaderProgram);
class ShaderEffect;
FWD_REFPTR(DeviceAPIDependantShaderEffect);
FWD_REFPTR(ShaderCompiled);
//----------------------------------------------------------------------------
class SurfaceFormat;
//----------------------------------------------------------------------------
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
class IDeviceAPIEncapsulator;
class IDeviceAPIDiagnostics;
class IDeviceAPIContext;
//----------------------------------------------------------------------------
class AbstractDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
enum class DeviceAPI;
enum class DeviceResourceType;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
