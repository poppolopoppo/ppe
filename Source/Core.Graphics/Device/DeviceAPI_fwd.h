#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Memory/RefPtr.h"

namespace Core {
    template <typename T>
    class TMemoryView;
}

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FPresentationParameters;
//----------------------------------------------------------------------------
class FDeviceResource;
class FDeviceResourceBuffer;
FWD_REFPTR(DeviceAPIDependantResourceBuffer);
//----------------------------------------------------------------------------
class FBlendState;
FWD_REFPTR(DeviceAPIDependantBlendState);
class FRasterizerState;
FWD_REFPTR(DeviceAPIDependantRasterizerState);
class FDepthStencilState;
FWD_REFPTR(DeviceAPIDependantDepthStencilState);
class FSamplerState;
FWD_REFPTR(DeviceAPIDependantSamplerState);
//----------------------------------------------------------------------------
enum class EPrimitiveType;
class IndexBuffer;
class FVertexBuffer;
struct FVertexBufferBinding;
class FVertexDeclaration;
FWD_REFPTR(DeviceAPIDependantVertexDeclaration);
//----------------------------------------------------------------------------
class FName;
class FConstantBuffer;
FWD_REFPTR(ConstantBufferLayout);
FWD_REFPTR(DeviceAPIDependantConstantWriter);
enum class EShaderProfileType;
class FShaderProgram;
struct FShaderProgramTexture;
enum class EShaderTextureDimension;
enum class EShaderProgramType;
FWD_REFPTR(DeviceAPIDependantShaderProgram);
class FShaderEffect;
FWD_REFPTR(DeviceAPIDependantShaderEffect);
FWD_REFPTR(ShaderCompiled);
//----------------------------------------------------------------------------
class FSurfaceFormat;
//----------------------------------------------------------------------------
class FTexture;
FWD_REFPTR(DeviceAPIDependantTexture);
class FTexture2D;
FWD_REFPTR(DeviceAPIDependantTexture2D);
class FTextureCube;
FWD_REFPTR(DeviceAPIDependantTextureCube);
class FRenderTarget;
FWD_REFPTR(DeviceAPIDependantRenderTarget);
class FDepthStencil;
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
} //!namespace Core
