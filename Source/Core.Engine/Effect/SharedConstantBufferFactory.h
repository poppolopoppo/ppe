#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/HashMap.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Meta/ThreadResource.h"

namespace Core {
namespace Graphics {
class FBindName;
class IDeviceAPIEncapsulator;
FWD_REFPTR(ConstantBufferLayout);
}

namespace Engine {
struct FSharedConstantBufferKey;
FWD_REFPTR(SharedConstantBuffer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FSharedConstantBufferFactory : public Meta::FThreadResource {
public:
    FSharedConstantBufferFactory();
    ~FSharedConstantBufferFactory();

    FSharedConstantBufferFactory(const FSharedConstantBufferFactory& ) = delete;
    FSharedConstantBufferFactory& operator =(const FSharedConstantBufferFactory& ) = delete;

    FSharedConstantBuffer *GetOrCreate(  const Graphics::FBindName& name,
                                        const Graphics::FConstantBufferLayout *layout );

    void ReleaseDestroyIFN(PSharedConstantBuffer& buffer);

    void Clear();

    void Start(Graphics::IDeviceAPIEncapsulator *device);
    void Shutdown(Graphics::IDeviceAPIEncapsulator *device);

private:
    Graphics::IDeviceAPIEncapsulator *_device;
    HASHMAP(FEffect, FSharedConstantBufferKey, PSharedConstantBuffer) _buffers;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
