#include "stdafx.h"

#include "AbstractDeviceAPIEncapsulator.h"

#include "DeviceAPIDependantEntity.h"
#include "DeviceEncapsulator.h"
#include "DeviceResource.h"

#include "Core/Diagnostic/Logger.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
AbstractDeviceAPIEncapsulator::AbstractDeviceAPIEncapsulator(DeviceAPI api, DeviceEncapsulator *owner, const PresentationParameters& pp)
:   _api(api)
,   _owner(owner)
,   _parameters(pp)
,   _usedMemory("UsedMemory",const_cast<MemoryTrackingData *>(&owner->VideoMemory())) {
    Assert(owner);
}
//----------------------------------------------------------------------------
AbstractDeviceAPIEncapsulator::~AbstractDeviceAPIEncapsulator() {}
//----------------------------------------------------------------------------
void AbstractDeviceAPIEncapsulator::OnCreateEntity(const DeviceResource *resource, DeviceAPIDependantEntity *entity) {
    Assert(entity);

    entity->AttachResource(resource);
    entity->SetCreatedAt(_owner->Revision());

    const size_t sizeInBytes = entity->VideoMemorySizeInBytes();
    _usedMemory.Allocate(1, sizeInBytes);
}
//----------------------------------------------------------------------------
void AbstractDeviceAPIEncapsulator::OnDestroyEntity(const DeviceResource *resource, DeviceAPIDependantEntity *entity) {
    Assert(entity);

    const size_t sizeInBytes = entity->VideoMemorySizeInBytes();
    _usedMemory.Deallocate(1, sizeInBytes);

    entity->DetachResource(resource);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *DeviceAPIToCStr(DeviceAPI api) {
    switch (api)
    {
    case Core::Graphics::DeviceAPI::DirectX11:
        return "DirectX11";
    case Core::Graphics::DeviceAPI::OpenGL4:
        return "OpenGL4";
    case Core::Graphics::DeviceAPI::Unknown:
        AssertNotReached();
        break;
    default:
        AssertNotImplemented();
        break;
    }
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
