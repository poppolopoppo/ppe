﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "AbstractDeviceAPIEncapsulator.h"

#include "DeviceAPIDependantEntity.h"
#include "DeviceEncapsulator.h"
#include "DeviceResource.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FAbstractDeviceAPIEncapsulator::FAbstractDeviceAPIEncapsulator(EDeviceAPI api, FDeviceEncapsulator *owner, const FPresentationParameters& pp)
:   _api(api)
,   _owner(owner)
,   _parameters(pp)
,   _usedMemory("UsedMemory", remove_const(&owner->VideoMemory())) {
    Assert(owner);
}
//----------------------------------------------------------------------------
FAbstractDeviceAPIEncapsulator::~FAbstractDeviceAPIEncapsulator() {}
//----------------------------------------------------------------------------
void FAbstractDeviceAPIEncapsulator::OnCreateEntity(const FDeviceResource *resource, FDeviceAPIDependantEntity *entity) {
    Assert(entity);

    entity->AttachResource(resource);
    entity->SetCreatedAt(_owner->Revision());

    const size_t sizeInBytes = entity->VideoMemorySizeInBytes();
    _usedMemory.Allocate(1, sizeInBytes);
}
//----------------------------------------------------------------------------
void FAbstractDeviceAPIEncapsulator::OnDestroyEntity(const FDeviceResource *resource, FDeviceAPIDependantEntity *entity) {
    Assert(entity);

    const size_t sizeInBytes = entity->VideoMemorySizeInBytes();
    _usedMemory.Deallocate(1, sizeInBytes);

    entity->DetachResource(resource);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView DeviceAPIToCStr(EDeviceAPI api) {
    switch (api)
    {
    case PPE::Graphics::EDeviceAPI::DirectX11:
        return MakeStringView("DirectX11");
    case PPE::Graphics::EDeviceAPI::OpenGL4:
        return MakeStringView("OpenGL4");
    case PPE::Graphics::EDeviceAPI::Unknown:
        AssertNotReached();
        break;
    default:
        AssertNotImplemented();
        break;
    }
    return FStringView();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
