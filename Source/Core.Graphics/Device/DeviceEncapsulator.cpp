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
,   _revision(InvalidDeviceRevision())
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
    return remove_const(this);
}
//----------------------------------------------------------------------------
IDeviceAPIContext *DeviceEncapsulator::Immediate() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return remove_const(this);
}
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
IDeviceAPIDiagnostics *DeviceEncapsulator::Diagnostics() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return remove_const(this);
}
#endif
//----------------------------------------------------------------------------
void DeviceEncapsulator::Create(DeviceAPI api, void *windowHandle, const PresentationParameters& presentationParameters) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_deviceAPIEncapsulator);

    LOG(Info, L"[DeviceEncapsulator] CreateDevice({0})", DeviceAPIToCStr(api));

    Assert(DeviceStatus::Invalid == _status);
    _status = DeviceStatus::Create;
    _revision.Value = 0;

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
    _onDeviceCreate.Invoke(this);

    Assert(DeviceStatus::Create == _status);
    _status = DeviceStatus::Normal;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);

    LOG(Info, L"[DeviceEncapsulator] DestroyDevice({0})", DeviceAPIToCStr(_deviceAPIEncapsulator->API()));

    Assert(DeviceStatus::Normal == _status);
    _status = DeviceStatus::Destroy;

    _onDeviceDestroy.Invoke(this);

    _deviceAPIEncapsulator->ClearState();

    Assert(nullptr != _deviceSharedEntityPool);
    const size_t poolSizeInBytes = _deviceSharedEntityPool->ReleaseAll_ReturnRealSize();
    if (0 != poolSizeInBytes) {
        LOG(Error, L"[DeviceSharedEntityPool] There is still {0} used in the pool !", SizeInBytes(poolSizeInBytes));
        AssertNotReached();
    }
    _deviceSharedEntityPool.reset();

    GraphicsStartup::OnDeviceDestroy(this);

    _deviceAPIEncapsulator.reset();

    _revision = InvalidDeviceRevision();

    Assert(DeviceStatus::Destroy == _status);
    _status = DeviceStatus::Invalid;

    Assert(!_deviceAPIEncapsulator);
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Reset(const PresentationParameters& pp) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Assert(DeviceStatus::Normal == _status);
    _status = DeviceStatus::Reset;

    _onDeviceReset.Invoke(this);

    _deviceSharedEntityPool->ReleaseAll_ReturnRealSize();
    _deviceAPIEncapsulator->Reset(pp);
    _revision.Value = 0;

    Assert(DeviceStatus::Reset == _status);
    _status = DeviceStatus::Invalid;
}
//----------------------------------------------------------------------------
void DeviceEncapsulator::Present() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _onDevicePresent.Invoke(this);

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
} //!namespace Graphics
} //!namespace Core
