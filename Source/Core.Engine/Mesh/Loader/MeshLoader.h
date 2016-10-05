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
struct FMeshHeader {
    u32 IndexStride;
    u32 IndexCount;
    u32 VertexStride;
    u32 VertexCount;
};
//----------------------------------------------------------------------------
class FBasicMeshLoader {
public:
    ~FBasicMeshLoader() {}

    FBasicMeshLoader(const FBasicMeshLoader& ) = delete;
    FBasicMeshLoader& operator =(const FBasicMeshLoader& ) = delete;

    const FMeshHeader& FHeader() const { return _header; }

protected:
    FBasicMeshLoader() { ResetHeader_(); }

    Graphics::IndexBuffer *CreateIndexBuffer_(  Graphics::IDeviceAPIEncapsulator *device,
                                                const char *resourceName,
                                                const TMemoryView<const u8>& data) const;

    Graphics::FVertexBuffer *CreateVertexBuffer_(Graphics::IDeviceAPIEncapsulator *device,
                                                const char *resourceName,
                                                const Graphics::FVertexDeclaration *declaration,
                                                const TMemoryView<const u8>& data) const;

    void ResetHeader_();

    FMeshHeader _header;
};
//----------------------------------------------------------------------------
template <typename _Index, typename _Vertex>
class TMeshLoader : public FBasicMeshLoader {
public:
    TMeshLoader() {}
    ~TMeshLoader() {}

    static const Graphics::FVertexDeclaration *FVertexDeclaration() { return _Vertex::Declaration; }

    bool Read(const FFilename& filename);
    void Clear();

    RAWSTORAGE_ALIGNED(Geometry, _Index, 16)& Indices() { return _indices; }
    RAWSTORAGE_ALIGNED(Geometry, _Vertex, 16)& Vertices() { return _vertices; }

    const RAWSTORAGE_ALIGNED(Geometry, _Index, 16)& Indices() const { return _indices; }
    const RAWSTORAGE_ALIGNED(Geometry, _Vertex, 16)& Vertices() const { return _vertices; }

    Graphics::IndexBuffer *CreateIndexBuffer(   Graphics::IDeviceAPIEncapsulator *device,
                                                const char *resourceName ) const;
    Graphics::FVertexBuffer *CreateVertexBuffer( Graphics::IDeviceAPIEncapsulator *device,
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
                const FFilename& filename );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace PLY {
//----------------------------------------------------------------------------
bool ReadMeshHeader(FMeshHeader& header, IVirtualFileSystemIStream *stream);
//----------------------------------------------------------------------------
bool ReadMeshData(const FMeshHeader& header, const TMemoryView<u8>& indices, FGenericVertex& vertices, IVirtualFileSystemIStream *stream);
//----------------------------------------------------------------------------
} //!namespace PLY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core

#include "Core.Engine/Mesh/Loader/MeshLoader-inl.h"
