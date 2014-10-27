#include "stdafx.h"

#include "MeshLoader.h"

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"

#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Graphics::IndexBuffer *BasicMeshLoader::CreateIndexBuffer_(
    Graphics::IDeviceAPIEncapsulator *device,
    const char *resourceName,
    const MemoryView<const u8>& data) const {
    Assert(resourceName);
    Assert(data.SizeInBytes() == _header.IndexCount * _header.IndexStride);

    Graphics::IndexBuffer *indices = new Graphics::IndexBuffer(Graphics::IndexElementSize(_header.IndexStride), _header.IndexCount, Graphics::BufferMode::None, Graphics::BufferUsage::Default);
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
Graphics::VertexBuffer *BasicMeshLoader::CreateVertexBuffer_(
    Graphics::IDeviceAPIEncapsulator *device,
    const char *resourceName,
    const Graphics::VertexDeclaration *declaration,
    const MemoryView<const u8>& data) const {
    Assert(resourceName);
    Assert(declaration);
    Assert(data.SizeInBytes() == _header.VertexCount * declaration->SizeInBytes());

    Graphics::VertexBuffer *vertices = new Graphics::VertexBuffer(declaration, _header.VertexCount, Graphics::BufferMode::None, Graphics::BufferUsage::Default);
    vertices->SetResourceName(resourceName);
    vertices->Freeze();

    vertices->Create(device, data);

    return vertices;
}
//----------------------------------------------------------------------------
void BasicMeshLoader::ResetHeader_() {
    memset(&_header, 0xCD, sizeof(_header));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
