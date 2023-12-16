// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "ModelMesh.h"

#include "ModelMeshSubPart.h"

#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, FModelMesh, );
//----------------------------------------------------------------------------
FModelMesh::FModelMesh(
    u32 indexCount,
    u32 vertexCount,
    Graphics::EPrimitiveType primitiveType,
    Graphics::IndexElementSize indexType,
    const Graphics::FVertexDeclaration *vertexDeclaration,
    MeshRawData&& indices,
    MeshRawData&& vertices,
    VECTOR(Mesh, PModelMeshSubPart)&& subParts )
:   _indexCount(indexCount)
,   _vertexCount(vertexCount)
,   _primitiveType(primitiveType)
,   _indexType(indexType)
,   _vertexDeclaration(vertexDeclaration)
,   _indices(std::move(indices))
,   _vertices(std::move(vertices))
,   _subParts(std::move(subParts)) {
    Assert(indexCount > 0);
    Assert(vertexCount > 0);
    Assert(vertexDeclaration);
    Assert(_indexCount == Graphics::IndexCount(primitiveType, Graphics::PrimitiveCount(primitiveType, _indexCount)) );
    Assert( (_indexType == Graphics::IndexElementSize::ThirtyTwoBits && _indexCount == (_indices.SizeInBytes() / sizeof(u32))) ||
            (_indexType == Graphics::IndexElementSize::SixteenBits && _indexCount == (_indices.SizeInBytes() / sizeof(u16))) );
    Assert(vertexCount == (_vertices.SizeInBytes() / vertexDeclaration->SizeInBytes()) );
}
//----------------------------------------------------------------------------
FModelMesh::~FModelMesh() {
    Assert(!_indexBuffer); // Destroy() must be called before destruction
    Assert(!_vertexBuffer);
}
//----------------------------------------------------------------------------
void FModelMesh::Create(Graphics::IDeviceAPIEncapsulator *device) {
    Assert(!_indexBuffer);
    Assert(!_vertexBuffer);
    Assert(_indices.size());
    Assert(_vertices.size());

    _indexBuffer = new Graphics::IndexBuffer(_indexType, _indexCount, Graphics::EBufferMode::None, Graphics::EBufferUsage::Immutable);
    _indexBuffer->Freeze();
    if (Graphics::IndexElementSize::ThirtyTwoBits == _indexType) {
        _indexBuffer->Create(device, _indices.MakeConstView().Cast<const u32>());
    }
    else {
        Assert(Graphics::IndexElementSize::SixteenBits == _indexType);
        _indexBuffer->Create(device, _indices.MakeConstView().Cast<const u16>());
    }

    _vertexBuffer = new Graphics::FVertexBuffer(_vertexDeclaration.get(), _vertexCount, Graphics::EBufferMode::None, Graphics::EBufferUsage::Immutable);
    _vertexBuffer->Freeze();
    _vertexBuffer->Create(device, _vertices.MakeConstView());
}
//----------------------------------------------------------------------------
void FModelMesh::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    Assert(_indexBuffer);
    Assert(_vertexBuffer);

    _indexBuffer->Destroy(device);
    _vertexBuffer->Destroy(device);

    RemoveRef_AssertReachZero(_indexBuffer);
    RemoveRef_AssertReachZero(_vertexBuffer);
}
//----------------------------------------------------------------------------
void FModelMesh::ReleaseCpuMemory() {
    _indices.clear_ReleaseMemory();
    _vertices.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
