#pragma once

#include "MeshLoader.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Index, typename _Vertex>
bool MeshLoader<_Index, _Vertex>::Read(const Filename& filename) {
    Assert(filename.Extname().Equals(L".ply")); // only format supported for now

    UniquePtr<IVirtualFileSystemIStream> iss = VFS_OpenReadable(filename, AccessPolicy::Binary);
    Assert(iss);

    if (!PLY::ReadMeshHeader(_header, iss.get())) {
        ResetHeader_();
        return false;
    }

    Assert(_header.IndexStride == sizeof(_Index));
    _indices.Resize_DiscardData(_header.IndexCount);
    _vertices.Resize_DiscardData(_header.VertexCount);

    const Graphics::VertexDeclaration *vertexDeclaration = VertexDeclaration();
    Assert(vertexDeclaration);
    Assert(vertexDeclaration->SizeInBytes() == sizeof(_Vertex));

    GenericVertex genericVertex(vertexDeclaration);
    genericVertex.SetDestination(_vertices.MakeView().Cast<u8>());

    if (PLY::ReadMeshData(_header, _indices.MakeView().Cast<u8>(), genericVertex, iss.get()))
        return true;

    Clear();
    return false;
}
//----------------------------------------------------------------------------
template <typename _Index, typename _Vertex>
void MeshLoader<_Index, _Vertex>::Clear() {
    ResetHeader_();

    _indices.Clear_ReleaseMemory();
    _vertices.Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
template <typename _Index, typename _Vertex>
Graphics::IndexBuffer *MeshLoader<_Index, _Vertex>::CreateIndexBuffer(
    Graphics::IDeviceAPIEncapsulator *device,
    const char *resourceName ) const {
    return CreateIndexBuffer_(device, resourceName, _indices.MakeView().Cast<const u8>());
}
//----------------------------------------------------------------------------
template <typename _Index, typename _Vertex>
Graphics::VertexBuffer *MeshLoader<_Index, _Vertex>::CreateVertexBuffer(
    Graphics::IDeviceAPIEncapsulator *device,
    const char *resourceName ) const {
    return CreateVertexBuffer_(device, resourceName, VertexDeclaration(), _vertices.MakeView().Cast<const u8>());
}
//----------------------------------------------------------------------------
template <typename _Index, typename _Vertex>
bool LoadMesh(  Graphics::IDeviceAPIEncapsulator *device,
                Graphics::PIndexBuffer *pindices,
                Graphics::PVertexBuffer *pvertices,
                const Filename& filename ) {
    Assert(device);
    Assert(pindices);
    Assert(pvertices);
    Assert(!filename.empty());

    MeshLoader<_Index, _Vertex> loader;
    if (!loader.Read(filename))
        return false;

    const float ACMR0 = VertexAverageCacheMissRate(loader.Indices().MakeView());

    OptimizeIndicesAndVerticesOrder(
        loader.Vertices(),
        _Vertex::Declaration,
        loader.Header().VertexCount,
        loader.Indices().MakeView() );

    const float ACMR1 = VertexAverageCacheMissRate(loader.Indices().MakeView());

    LOG(Warning, L"Optimized mesh average cache miss rate from {0}% to {1}%", ACMR0, ACMR1);

    const String resourceName = filename.ToString();
    *pindices = loader.CreateIndexBuffer(device, resourceName.c_str());
    *pvertices = loader.CreateVertexBuffer(device, resourceName.c_str());

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
