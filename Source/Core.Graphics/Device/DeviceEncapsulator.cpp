#include "stdafx.h"

#include "DeviceEncapsulator.h"

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
#include "Core/Memory/ComPtr.h"

#ifdef OS_WINDOWS
#   include "DirectX11/DX11DeviceAPIEncapsulator.h"
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DeviceEncapsulator::DeviceEncapsulator() {}
//----------------------------------------------------------------------------
DeviceEncapsulator::~DeviceEncapsulator() {
    AssertRelease(!_deviceAPIEncapsulator);
}
//----------------------------------------------------------------------------
DeviceAPI DeviceEncapsulator::API() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);
    return _deviceAPIEncapsulator->API();
}
//----------------------------------------------------------------------------
const PresentationParameters& DeviceEncapsulator::Parameters() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);
    return _deviceAPIEncapsulator->Parameters();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Create(DeviceAPI api, void *windowHandle, const PresentationParameters& presentationParameters) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_deviceAPIEncapsulator);

    LOG(Information, L"[DeviceEncapsulator] CreateDevice({0})", DeviceAPIToCStr(api));

    switch (api)
    {
    case Core::Graphics::DeviceAPI::DirectX11:
        _deviceAPIEncapsulator.reset(new DX11DeviceAPIEncapsulator(this, windowHandle, presentationParameters));
        break;

    case Core::Graphics::DeviceAPI::OpenGL4:
    default:
        AssertNotImplemented();
    }

    Assert(_deviceAPIEncapsulator);

    GraphicsStartup::OnDeviceCreate(this);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);

    LOG(Information, L"[DeviceEncapsulator] DestroyDevice({0})", DeviceAPIToCStr(_deviceAPIEncapsulator->API()));

     _deviceAPIEncapsulator->ClearState();

    GraphicsStartup::OnDeviceDestroy(this);

    _deviceAPIEncapsulator.reset();

    Assert(!_deviceAPIEncapsulator);
}
//----------------------------------------------------------------------------
IDeviceAPIEncapsulator *DeviceEncapsulator::Device() const { 
    return const_cast<DeviceEncapsulator *>(this);
}
//----------------------------------------------------------------------------
IDeviceAPIContext *DeviceEncapsulator::Immediate() const { 
    return const_cast<DeviceEncapsulator *>(this);
}
//----------------------------------------------------------------------------
IDeviceAPIShaderCompiler *DeviceEncapsulator::ShaderCompiler() const { 
    return const_cast<DeviceEncapsulator *>(this);
}
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
IDeviceAPIDiagnostics *DeviceEncapsulator::Diagnostics() const { 
    return const_cast<DeviceEncapsulator *>(this);
}
#endif
//----------------------------------------------------------------------------
// Status
//----------------------------------------------------------------------------
void DeviceEncapsulator::Reset(const PresentationParameters& pp) {
    THIS_THREADRESOURCE_CHECKACCESS();

    ++_revision.Value;

    _deviceAPIEncapsulator->Reset(pp);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Present() {
    THIS_THREADRESOURCE_CHECKACCESS();

    ++_revision.Value;

    _deviceAPIEncapsulator->Present();
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::ClearState() {
    THIS_THREADRESOURCE_CHECKACCESS();

    ++_revision.Value;

    _deviceAPIEncapsulator->ClearState();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *DeviceStatusToCStr(DeviceStatus status) {
    switch (status)
    {
    case Core::Graphics::DeviceStatus::Invalid:
        return "Invalid";
    case Core::Graphics::DeviceStatus::Normal:
        return "Normal";
    case Core::Graphics::DeviceStatus::Reset:
        return "Reset";
    case Core::Graphics::DeviceStatus::Lost:
        return "Lost";
    case Core::Graphics::DeviceStatus::Destroy:
        return "Destroy";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
