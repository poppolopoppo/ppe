#include "stdafx.h"

#include "DeviceEncapsulator.h"

#include "GlobalVideoMemory.h"
#include "DeviceEncapsulatorException.h"
#include "PresentationParameters.h"

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

#include "Pool/DeviceSharedEntityPool.h"

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
DeviceEncapsulator::DeviceEncapsulator() 
:   _status(DeviceStatus::Invalid)
,   _revision(0)
,   _videoMemory("DeviceEncapsulator", &GlobalVideoMemory::Instance()) {}
//----------------------------------------------------------------------------
DeviceEncapsulator::~DeviceEncapsulator() {
    Assert(DeviceStatus::Invalid == _status);
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
IDeviceAPIEncapsulator *DeviceEncapsulator::Device() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return const_cast<DeviceEncapsulator *>(this);
}
//----------------------------------------------------------------------------
IDeviceAPIContext *DeviceEncapsulator::Immediate() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return const_cast<DeviceEncapsulator *>(this);
}
//----------------------------------------------------------------------------
IDeviceAPIShaderCompiler *DeviceEncapsulator::ShaderCompiler() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return const_cast<DeviceEncapsulator *>(this);
}
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
IDeviceAPIDiagnostics *DeviceEncapsulator::Diagnostics() const {
    return const_cast<DeviceEncapsulator *>(this);
}
#endif
//----------------------------------------------------------------------------
void DeviceEncapsulator::Create(DeviceAPI api, void *windowHandle, const PresentationParameters& presentationParameters) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_deviceAPIEncapsulator);

    LOG(Information, L"[DeviceEncapsulator] CreateDevice({0})", DeviceAPIToCStr(api));

    Assert(DeviceStatus::Invalid == _status);
    _status = DeviceStatus::Create;

    switch (api)
    {
    case Core::Graphics::DeviceAPI::DirectX11:
        _deviceAPIEncapsulator.reset(new DX11DeviceAPIEncapsulator(this, windowHandle, presentationParameters));
        break;

    case Core::Graphics::DeviceAPI::OpenGL4:
    default:
        AssertNotImplemented();
    }

    Assert(nullptr == _deviceSharedEntityPool);
    _deviceSharedEntityPool.reset(new DeviceSharedEntityPool(&_videoMemory));

    Assert(_deviceAPIEncapsulator);
    GraphicsStartup::OnDeviceCreate(this);

    Assert(DeviceStatus::Create == _status);
    _status = DeviceStatus::Normal;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);

    LOG(Information, L"[DeviceEncapsulator] DestroyDevice({0})", DeviceAPIToCStr(_deviceAPIEncapsulator->API()));

    Assert(DeviceStatus::Normal == _status);
    _status = DeviceStatus::Destroy;

    _deviceAPIEncapsulator->ClearState();

    Assert(nullptr != _deviceSharedEntityPool);
    _deviceSharedEntityPool->ReleaseAll();
    _deviceSharedEntityPool.reset();

    GraphicsStartup::OnDeviceDestroy(this);

    _deviceAPIEncapsulator.reset();

    Assert(DeviceStatus::Destroy == _status);
    _status = DeviceStatus::Invalid;

    Assert(!_deviceAPIEncapsulator);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Reset(const PresentationParameters& pp) {
    THIS_THREADRESOURCE_CHECKACCESS();
    
    Assert(DeviceStatus::Normal == _status);
    _status = DeviceStatus::Reset;

    _revision.Value = 0;
    _deviceAPIEncapsulator->Reset(pp);
    
    Assert(DeviceStatus::Reset == _status);
    _status = DeviceStatus::Invalid;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Present() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->Present();
    ++_revision.Value;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::ClearState() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->ClearState();
}
//----------------------------------------------------------------------------
const AbstractDeviceAPIEncapsulator *DeviceEncapsulator::APIEncapsulator() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);

    return _deviceAPIEncapsulator.get();
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
    case Core::Graphics::DeviceStatus::Create:
        return "Create";
    case Core::Graphics::DeviceStatus::Destroy:
        return "Destroy";
    case Core::Graphics::DeviceStatus::Reset:
        return "Reset";
    case Core::Graphics::DeviceStatus::Lost:
        return "Lost";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
