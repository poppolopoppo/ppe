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
enum class EIndexElementSize;
enum class EPrimitiveType;
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
class FModelMesh : public FRefCountable {
public:
    FModelMesh(  u32 indexCount,
                u32 vertexCount,
                Graphics::EPrimitiveType primitiveType,
                Graphics::IndexElementSize indexType,
                const Graphics::FVertexDeclaration *vertexDeclaration,
                MeshRawData&& indices,
                MeshRawData&& vertices,
                VECTOR(Mesh, PModelMeshSubPart)&& subParts );
    ~FModelMesh();

    FModelMesh(const FModelMesh& ) = delete;
    FModelMesh& operator =(const FModelMesh& ) = delete;

    u32 IndexCount() const { return _indexCount; }
    u32 VertexCount() const { return _vertexCount; }

    Graphics::EPrimitiveType EPrimitiveType() const { return _primitiveType; }
    Graphics::IndexElementSize IndexType() const { return _indexType; }
    const Graphics::FVertexDeclaration *FVertexDeclaration() const { return _vertexDeclaration; }

    const MeshRawData& Indices() const { return _indices; }
    const MeshRawData& Vertices() const { return _vertices; }

    const Graphics::PIndexBuffer& IndexBuffer() const { return _indexBuffer; }
    const Graphics::PVertexBuffer& FVertexBuffer() const { return _vertexBuffer; }

    const VECTOR(Mesh, PModelMeshSubPart)& SubParts() const { return _subParts; }

    void Create(Graphics::IDeviceAPIEncapsulator *device);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    void ReleaseCpuMemory();

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    u32 _indexCount;
    u32 _vertexCount;

    Graphics::EPrimitiveType _primitiveType;
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
