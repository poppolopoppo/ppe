#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Meta/ThreadResource.h"

namespace Core {
namespace Graphics {
class BindName;
class IDeviceAPIEncapsulator;
FWD_REFPTR(ConstantBufferLayout);
}

namespace Engine {
struct SharedConstantBufferKey;
FWD_REFPTR(SharedConstantBuffer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SharedConstantBufferFactory : public Meta::ThreadResource {
public:
    SharedConstantBufferFactory();
    ~SharedConstantBufferFactory();

    SharedConstantBufferFactory(const SharedConstantBufferFactory& ) = delete;
    SharedConstantBufferFactory& operator =(const SharedConstantBufferFactory& ) = delete;

    SharedConstantBuffer *GetOrCreate(  const Graphics::BindName& name,
                                        const Graphics::ConstantBufferLayout *layout );

    void ReleaseDestroyIFN(PSharedConstantBuffer& buffer);

    void Clear();

    void Start(Graphics::IDeviceAPIEncapsulator *device);
    void Shutdown(Graphics::IDeviceAPIEncapsulator *device);

private:
    Graphics::IDeviceAPIEncapsulator *_device;
    HASHMAP(Effect, SharedConstantBufferKey, PSharedConstantBuffer) _buffers;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
