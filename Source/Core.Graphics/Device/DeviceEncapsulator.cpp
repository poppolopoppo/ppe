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
#include "Core/IO/FormatHelpers.h"
#include "Core/Memory/ComPtr.h"

#ifdef PLATFORM_WINDOWS
#   include "DirectX11/DX11DeviceAPIEncapsulator.h"
#endif

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
IDeviceAPIContext* IDeviceAPIEncapsulator::Immediate() const {
    return APIEncapsulator()->Immediate();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDeviceEncapsulator::FDeviceEncapsulator()
:   _status(EDeviceStatus::Invalid)
,   _revision(InvalidDeviceRevision())
,   _videoMemory("FDeviceEncapsulator", &FGlobalVideoMemory::Instance()) {}
//----------------------------------------------------------------------------
FDeviceEncapsulator::~FDeviceEncapsulator() {
    Assert(EDeviceStatus::Invalid == _status);
    AssertRelease(!_deviceAPIEncapsulator);
}
//----------------------------------------------------------------------------
EDeviceAPI FDeviceEncapsulator::API() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);
    return _deviceAPIEncapsulator->API();
}
//----------------------------------------------------------------------------
const FPresentationParameters& FDeviceEncapsulator::Parameters() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);
    return _deviceAPIEncapsulator->Parameters();
}
//----------------------------------------------------------------------------
IDeviceAPIEncapsulator *FDeviceEncapsulator::Device() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return remove_const(this);
}
//----------------------------------------------------------------------------
IDeviceAPIContext *FDeviceEncapsulator::Immediate() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return remove_const(this);
}
//----------------------------------------------------------------------------
#ifdef WITH_CORE_GRAPHICS_DIAGNOSTICS
IDeviceAPIDiagnostics *FDeviceEncapsulator::Diagnostics() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    return remove_const(this);
}
#endif
//----------------------------------------------------------------------------
void FDeviceEncapsulator::Create(EDeviceAPI api, void *windowHandle, const FPresentationParameters& presentationParameters) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_deviceAPIEncapsulator);

    LOG(Info, L"[FDeviceEncapsulator] CreateDevice({0})", DeviceAPIToCStr(api));

    Assert(EDeviceStatus::Invalid == _status);
    _status = EDeviceStatus::Create;
    _revision.Value = 0;

    switch (api)
    {
    case Core::Graphics::EDeviceAPI::DirectX11:
        _deviceAPIEncapsulator.reset(new FDX11DeviceAPIEncapsulator(this, windowHandle, presentationParameters));
        break;

    case Core::Graphics::EDeviceAPI::OpenGL4:
    default:
        AssertNotImplemented();
    }

    Assert(nullptr == _deviceSharedEntityPool);
    _deviceSharedEntityPool.reset(new FDeviceSharedEntityPool(&_videoMemory));

    Assert(_deviceAPIEncapsulator);
    FGraphicsModule::OnDeviceCreate(this);
    _onDeviceCreate.Invoke(this);

    Assert(EDeviceStatus::Create == _status);
    _status = EDeviceStatus::Normal;
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::Destroy() {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);

    LOG(Info, L"[FDeviceEncapsulator] DestroyDevice({0})", DeviceAPIToCStr(_deviceAPIEncapsulator->API()));

    Assert(EDeviceStatus::Normal == _status);
    _status = EDeviceStatus::Destroy;

    _onDeviceDestroy.Invoke(this);

    _deviceAPIEncapsulator->ClearState();

    Assert(nullptr != _deviceSharedEntityPool);
    const size_t poolSizeInBytes = _deviceSharedEntityPool->ReleaseAll_ReturnRealSize();
    if (0 != poolSizeInBytes) {
        LOG(Error, L"[FDeviceSharedEntityPool] There is still {0} used in the pool !", FSizeInBytes{ poolSizeInBytes });
        AssertNotReached();
    }
    _deviceSharedEntityPool.reset();

    FGraphicsModule::OnDeviceDestroy(this);

    _deviceAPIEncapsulator.reset();

    _revision = InvalidDeviceRevision();

    Assert(EDeviceStatus::Destroy == _status);
    _status = EDeviceStatus::Invalid;

    Assert(!_deviceAPIEncapsulator);
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::Reset(const FPresentationParameters& pp) {
    THIS_THREADRESOURCE_CHECKACCESS();

    Assert(EDeviceStatus::Normal == _status);
    _status = EDeviceStatus::Reset;

    _onDeviceReset.Invoke(this);

    _deviceSharedEntityPool->ReleaseAll_ReturnRealSize();
    _deviceAPIEncapsulator->Reset(pp);
    _revision.Value = 0;

    Assert(EDeviceStatus::Reset == _status);
    _status = EDeviceStatus::Invalid;
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::Present() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _onDevicePresent.Invoke(this);

    _deviceAPIEncapsulator->Present();
    ++_revision.Value;
}
//----------------------------------------------------------------------------
void FDeviceEncapsulator::ClearState() {
    THIS_THREADRESOURCE_CHECKACCESS();

    _deviceAPIEncapsulator->ClearState();
}
//----------------------------------------------------------------------------
const FAbstractDeviceAPIEncapsulator *FDeviceEncapsulator::APIEncapsulator() const {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_deviceAPIEncapsulator);

    return _deviceAPIEncapsulator.get();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
