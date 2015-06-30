#include "stdafx.h"

#include "SharedConstantBufferFactory.h"

#include "SharedConstantBuffer.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/Shader/ConstantBufferLayout.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Container/Hash.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SharedConstantBufferFactory::SharedConstantBufferFactory() 
:   _device(nullptr) {}
//----------------------------------------------------------------------------
SharedConstantBufferFactory::~SharedConstantBufferFactory() {
    THIS_THREADRESOURCE_CHECKACCESS();
    AssertRelease(_buffers.empty()); // someone forgot to call ReleaseDestroyIFN()
    Assert(nullptr == _device);
}
//----------------------------------------------------------------------------
SharedConstantBuffer *SharedConstantBufferFactory::GetOrCreate(
    const Graphics::BindName& name,
    const Graphics::ConstantBufferLayout *layout ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);
    Assert(!name.empty());
    Assert(layout);

    const SharedConstantBufferKey sharedKey{name, layout};
    PSharedConstantBuffer& buffer = _buffers[sharedKey];
    if (!buffer) {
        buffer = new SharedConstantBuffer(sharedKey);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        buffer->SetResourceName(name.cstr());
#endif
        buffer->Freeze();
        buffer->Create(_device);
    }

    return buffer.get();
}
//----------------------------------------------------------------------------
void SharedConstantBufferFactory::ReleaseDestroyIFN(PSharedConstantBuffer& buffer) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);
    Assert(buffer);
    Assert(buffer->RefCount() > 1);

    if (buffer->RefCount() == 2) {
        _buffers.erase(buffer->SharedKey());
        buffer->Destroy(_device);
        RemoveRef_AssertReachZero(buffer);
    }
    else {
        buffer.reset();
    }
}
//----------------------------------------------------------------------------
void SharedConstantBufferFactory::Clear() {
    AssertRelease(_buffers.empty()); // should be already empty after EffectCompiler.Clear() !
}
//----------------------------------------------------------------------------
void SharedConstantBufferFactory::Start(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_device);

    _device = device;
}
//----------------------------------------------------------------------------
void SharedConstantBufferFactory::Shutdown(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(device == _device);
    AssertRelease(_buffers.empty());

    _device = nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
