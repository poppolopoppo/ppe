// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

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
FSharedConstantBufferFactory::FSharedConstantBufferFactory() 
:   _device(nullptr) {}
//----------------------------------------------------------------------------
FSharedConstantBufferFactory::~FSharedConstantBufferFactory() {
    THIS_THREADRESOURCE_CHECKACCESS();
    AssertRelease(_buffers.empty()); // someone forgot to call ReleaseDestroyIFN()
    Assert(nullptr == _device);
}
//----------------------------------------------------------------------------
FSharedConstantBuffer *FSharedConstantBufferFactory::GetOrCreate(
    const Graphics::FBindName& name,
    const Graphics::FConstantBufferLayout *layout ) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(_device);
    Assert(!name.empty());
    Assert(layout);

    const FSharedConstantBufferKey sharedKey{name, layout};
    PSharedConstantBuffer& buffer = _buffers[sharedKey];
    if (!buffer) {
        buffer = new FSharedConstantBuffer(sharedKey);
#ifdef WITH_GRAPHICS_DEVICERESOURCE_NAME
        buffer->SetResourceName(name.c_str());
#endif
        buffer->Freeze();
        buffer->Create(_device);
    }

    return buffer.get();
}
//----------------------------------------------------------------------------
void FSharedConstantBufferFactory::ReleaseDestroyIFN(PSharedConstantBuffer& buffer) {
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
void FSharedConstantBufferFactory::Clear() {
    AssertRelease(_buffers.empty()); // should be already empty after FEffectCompiler.Clear() !
}
//----------------------------------------------------------------------------
void FSharedConstantBufferFactory::Start(Graphics::IDeviceAPIEncapsulator *device) {
    THIS_THREADRESOURCE_CHECKACCESS();
    Assert(!_device);

    _device = device;
}
//----------------------------------------------------------------------------
void FSharedConstantBufferFactory::Shutdown(Graphics::IDeviceAPIEncapsulator *device) {
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
