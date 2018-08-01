#pragma once

#include "Graphics.h"

#include "Memory/RefPtr.h"

namespace PPE {
    template <typename T>
    class TMemoryView;
}

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FPresentationParameters;
//----------------------------------------------------------------------------
FWD_REFPTR(DeviceResource);
FWD_REFPTR(DeviceResourceBuffer);
FWD_REFPTR(DeviceAPIDependantResourceBuffer);
//----------------------------------------------------------------------------
FWD_REFPTR(BlendState);
FWD_REFPTR(DeviceAPIDependantBlendState);
FWD_REFPTR(RasterizerState);
FWD_REFPTR(DeviceAPIDependantRasterizerState);
FWD_REFPTR(DepthStencilState);
FWD_REFPTR(DeviceAPIDependantDepthStencilState);
FWD_REFPTR(SamplerState);
FWD_REFPTR(DeviceAPIDependantSamplerState);
//----------------------------------------------------------------------------
enum class EPrimitiveType;
FWD_REFPTR(IndexBuffer);
FWD_REFPTR(VertexBuffer);
struct FVertexBufferBinding;
FWD_REFPTR(VertexDeclaration);
FWD_REFPTR(DeviceAPIDependantVertexDeclaration);
//----------------------------------------------------------------------------
class FName;
FWD_REFPTR(ConstantBuffer);
FWD_REFPTR(ConstantBufferLayout);
FWD_REFPTR(DeviceAPIDependantConstantWriter);
enum class EShaderProfileType;
FWD_REFPTR(ShaderProgram);
struct FShaderProgramTexture;
enum class EShaderTextureDimension;
enum class EShaderProgramType;
FWD_REFPTR(DeviceAPIDependantShaderProgram);
FWD_REFPTR(ShaderEffect);
FWD_REFPTR(DeviceAPIDependantShaderEffect);
FWD_REFPTR(ShaderCompiled);
//----------------------------------------------------------------------------
class FSurfaceFormat;
//----------------------------------------------------------------------------
FWD_REFPTR(Texture);
FWD_REFPTR(DeviceAPIDependantTexture);
FWD_REFPTR(Texture2D);
FWD_REFPTR(DeviceAPIDependantTexture2D);
FWD_REFPTR(TextureCube);
FWD_REFPTR(DeviceAPIDependantTextureCube);
FWD_REFPTR(RenderTarget);
FWD_REFPTR(DeviceAPIDependantRenderTarget);
FWD_REFPTR(DepthStencil);
FWD_REFPTR(DeviceAPIDependantDepthStencil);
struct FRenderTargetBinding;
//----------------------------------------------------------------------------
class IDeviceAPIEncapsulator;
class IDeviceAPIDiagnostics;
class IDeviceAPIContext;
//----------------------------------------------------------------------------
class FAbstractDeviceAPIEncapsulator;
//----------------------------------------------------------------------------
enum class EDeviceAPI;
enum class EDeviceResourceType;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
