#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/RawStorage.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(IndexBuffer);
enum class IndexElementSize;
enum class PrimitiveType;
FWD_REFPTR(VertexBuffer);
FWD_REFPTR(VertexDeclaration);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
typedef RAWSTORAGE_ALIGNED(Mesh, u8, 16) MeshRawData;
FWD_REFPTR(ModelMeshSubPart);
//----------------------------------------------------------------------------
FWD_REFPTR(ModelMesh);
class ModelMesh : public RefCountable {
public:
    ModelMesh(  u32 indexCount,
                u32 vertexCount,
                Graphics::PrimitiveType primitiveType,
                Graphics::IndexElementSize indexType,
                const Graphics::VertexDeclaration *vertexDeclaration,
                MeshRawData&& indices,
                MeshRawData&& vertices,
                VECTOR(Mesh, PModelMeshSubPart)&& subParts );
    ~ModelMesh();

    ModelMesh(const ModelMesh& ) = delete;
    ModelMesh& operator =(const ModelMesh& ) = delete;

    u32 IndexCount() const { return _indexCount; }
    u32 VertexCount() const { return _vertexCount; }

    Graphics::PrimitiveType PrimitiveType() const { return _primitiveType; }
    Graphics::IndexElementSize IndexType() const { return _indexType; }
    const Graphics::VertexDeclaration *VertexDeclaration() const { return _vertexDeclaration; }

    const MeshRawData& Indices() const { return _indices; }
    const MeshRawData& Vertices() const { return _vertices; }

    const Graphics::PIndexBuffer& IndexBuffer() const { return _indexBuffer; }
    const Graphics::PVertexBuffer& VertexBuffer() const { return _vertexBuffer; }

    const VECTOR(Mesh, PModelMeshSubPart)& SubParts() const { return _subParts; }

    void Create(Graphics::IDeviceAPIEncapsulator *device);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    void ReleaseCpuMemory();

    SINGLETON_POOL_ALLOCATED_DECL(ModelMesh);

private:
    u32 _indexCount;
    u32 _vertexCount;

    Graphics::PrimitiveType _primitiveType;
    Graphics::IndexElementSize _indexType;
    Graphics::PCVertexDeclaration _vertexDeclaration;

    MeshRawData _indices;
    MeshRawData _vertices;

    Graphics::PIndexBuffer _indexBuffer;
    Graphics::PVertexBuffer _vertexBuffer;

    VECTOR(Mesh, PModelMeshSubPart) _subParts;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
