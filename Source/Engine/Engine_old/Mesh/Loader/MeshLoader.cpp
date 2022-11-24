// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "MeshLoader.h"

#include "Core.Graphics/Device/DeviceAPI.h"

#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Graphics::IndexBuffer *FBasicMeshLoader::CreateIndexBuffer_(
    Graphics::IDeviceAPIEncapsulator *device,
    const char *resourceName,
    const TMemoryView<const u8>& data) const {
    Assert(resourceName);
    Assert(data.SizeInBytes() == _header.IndexCount * _header.IndexStride);

    Graphics::IndexBuffer *indices = new Graphics::IndexBuffer(Graphics::IndexElementSize(_header.IndexStride), _header.IndexCount, Graphics::EBufferMode::None, Graphics::EBufferUsage::Default);
    indices->SetResourceName(resourceName);
    indices->Freeze();

    if (_header.IndexStride == u32(Graphics::IndexElementSize::ThirtyTwoBits)) {
        indices->Create(device, data.Cast<const u32>());
    }
    else {
        Assert(_header.IndexStride == u32(Graphics::IndexElementSize::SixteenBits));
        indices->Create(device, data.Cast<const u16>());
    }

    return indices;
}
//----------------------------------------------------------------------------
Graphics::FVertexBuffer *FBasicMeshLoader::CreateVertexBuffer_(
    Graphics::IDeviceAPIEncapsulator *device,
    const char *resourceName,
    const Graphics::FVertexDeclaration *declaration,
    const TMemoryView<const u8>& data) const {
    Assert(resourceName);
    Assert(declaration);
    Assert(data.SizeInBytes() == _header.VertexCount * declaration->SizeInBytes());

    Graphics::FVertexBuffer *vertices = new Graphics::FVertexBuffer(declaration, _header.VertexCount, Graphics::EBufferMode::None, Graphics::EBufferUsage::Default);
    vertices->SetResourceName(resourceName);
    vertices->Freeze();

    vertices->Create(device, data);

    return vertices;
}
//----------------------------------------------------------------------------
void FBasicMeshLoader::ResetHeader_() {
    memset(&_header, 0xCD, sizeof(_header));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
