#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/VirtualFileSystem_fwd.h"

#include "Core.Engine/Mesh/Geometry/GenericVertex.h"
#include "Core.Engine/Mesh/Geometry/GenericVertexExport.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
FWD_REFPTR(IndexBuffer);
FWD_REFPTR(VertexBuffer);
FWD_REFPTR(VertexDeclaration);
}

namespace Engine {
// TODO : material and multi material handling !

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct MeshHeader {
    u32 IndexStride;
    u32 IndexCount;
    u32 VertexStride;
    u32 VertexCount;
};
//----------------------------------------------------------------------------
class BasicMeshLoader {
public:
    ~BasicMeshLoader() {}

    BasicMeshLoader(const BasicMeshLoader& ) = delete;
    BasicMeshLoader& operator =(const BasicMeshLoader& ) = delete;

    const MeshHeader& Header() const { return _header; }

protected:
    BasicMeshLoader() { ResetHeader_(); }

    Graphics::IndexBuffer *CreateIndexBuffer_(  Graphics::IDeviceAPIEncapsulator *device,
                                                const char *resourceName,
                                                const MemoryView<const u8>& data) const;

    Graphics::VertexBuffer *CreateVertexBuffer_(Graphics::IDeviceAPIEncapsulator *device,
                                                const char *resourceName,
                                                const Graphics::VertexDeclaration *declaration,
                                                const MemoryView<const u8>& data) const;

    void ResetHeader_();

    MeshHeader _header;
};
//----------------------------------------------------------------------------
template <typename _Index, typename _Vertex>
class MeshLoader : public BasicMeshLoader {
public:
    MeshLoader() {}
    ~MeshLoader() {}

    static const Graphics::VertexDeclaration *VertexDeclaration() { return _Vertex::Declaration; }

    bool Read(const Filename& filename);
    void Clear();

    RAWSTORAGE_ALIGNED(Geometry, _Index, 16)& Indices() { return _indices; }
    RAWSTORAGE_ALIGNED(Geometry, _Vertex, 16)& Vertices() { return _vertices; }

    const RAWSTORAGE_ALIGNED(Geometry, _Index, 16)& Indices() const { return _indices; }
    const RAWSTORAGE_ALIGNED(Geometry, _Vertex, 16)& Vertices() const { return _vertices; }

    Graphics::IndexBuffer *CreateIndexBuffer(   Graphics::IDeviceAPIEncapsulator *device,
                                                const char *resourceName ) const;
    Graphics::VertexBuffer *CreateVertexBuffer( Graphics::IDeviceAPIEncapsulator *device,
                                                const char *resourceName ) const;

private:
    RAWSTORAGE_ALIGNED(Geometry, _Index, 16) _indices;
    RAWSTORAGE_ALIGNED(Geometry, _Vertex, 16) _vertices;
};
//----------------------------------------------------------------------------
template <typename _Index, typename _Vertex>
bool LoadMesh(  Graphics::IDeviceAPIEncapsulator *device,
                Graphics::PIndexBuffer *pindices,
                Graphics::PVertexBuffer *pvertices,
                const Filename& filename );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace PLY {
//----------------------------------------------------------------------------
bool ReadMeshHeader(MeshHeader& header, IVirtualFileSystemIStream *stream);
//----------------------------------------------------------------------------
bool ReadMeshData(const MeshHeader& header, const MemoryView<u8>& indices, GenericVertex& vertices, IVirtualFileSystemIStream *stream);
//----------------------------------------------------------------------------
} //!namespace PLY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core

#include "Core.Engine/Mesh/Loader/MeshLoader-inl.h"
