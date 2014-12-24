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
FWD_REFPTR(Material);
typedef RAWSTORAGE_ALIGNED(Mesh, u8, 16) MeshRawData;
//----------------------------------------------------------------------------
FWD_REFPTR(ModelMesh);
class ModelMesh : public RefCountable {
public:
    ModelMesh(  Engine::Material *material,
                u32 indexCount,
                u32 vertexCount,
                Graphics::PrimitiveType primitiveType,
                Graphics::IndexElementSize indexType,
                const Graphics::VertexDeclaration *vertexDeclaration,
                MeshRawData&& indices,
                MeshRawData&& vertices );
    ~ModelMesh();

    ModelMesh(const ModelMesh& ) = delete;
    ModelMesh& operator =(const ModelMesh& ) = delete;

    const Engine::Material *Material() const { return _material.get(); }

    u32 IndexCount() const { return _indexCount; }
    u32 VertexCount() const { return _vertexCount; }

    Graphics::PrimitiveType PrimitiveType() const { return _primitiveType; }
    Graphics::IndexElementSize IndexType() const { return _indexType; }
    const Graphics::VertexDeclaration *VertexDeclaration() const { return _vertexDeclaration; }

    const MeshRawData& Indices() const { return _indices; }
    const MeshRawData& Vertices() const { return _vertices; }

    const Graphics::PIndexBuffer& IndexBuffer() const { return _indexBuffer; }
    const Graphics::PVertexBuffer& VertexBuffer() const { return _vertexBuffer; }

    void Create(Graphics::IDeviceAPIEncapsulator *device);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    void ReleaseRawData();

    SINGLETON_POOL_ALLOCATED_DECL(ModelMesh);

private:
    PMaterial _material;

    u32 _indexCount;
    u32 _vertexCount;

    Graphics::PrimitiveType _primitiveType;
    Graphics::IndexElementSize _indexType;
    Graphics::PCVertexDeclaration _vertexDeclaration;

    MeshRawData _indices;
    MeshRawData _vertices;

    Graphics::PIndexBuffer _indexBuffer;
    Graphics::PVertexBuffer _vertexBuffer;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
