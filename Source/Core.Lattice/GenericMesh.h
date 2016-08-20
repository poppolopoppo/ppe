#pragma once

#include "Core.Lattice/Lattice.h"

#include "Core.Lattice/GenericMesh_fwd.h"

#include "Core.Graphics/Value.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Stack.h"
#include "Core/Memory/MemoryStream.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
enum class IndexElementSize;
FWD_REFPTR(VertexDeclaration);
class VertexSemantic;
}

namespace Lattice {
FWD_REFPTR(GenericMesh);
class GenericVertexData;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class GenericVertexSubPart {
public:
    STATIC_CONST_INTEGRAL(Graphics::ValueType, Type, Graphics::ValueTraits<T>::TypeId);
    STATIC_ASSERT(Type != Graphics::ValueType::Void);

    GenericVertexSubPart() : GenericVertexSubPart(nullptr) {}
    explicit GenericVertexSubPart(const GenericVertexData* data);

    size_t size() const { return (_data ? _data->VertexCount() : 0); }
    bool empty() const { return (0 == size()); }

    bool valid() const { return (nullptr != _data); }
    operator void* () const { return (void*)_data; }

    const GenericVertexData* Data() const { return _data; }

    void Write(const T& src) const;
    void Write(const MemoryView<const T>& src) const;
    MemoryView<T> Append(size_t count) const;
    MemoryView<T> Resize(size_t count, bool keepData = true) const;
    MemoryView<T> MakeView() const;

private:
    const GenericVertexData* _data;
};
//----------------------------------------------------------------------------
class GenericVertexData {
public:
    template <typename T>
    friend class GenericVertexSubPart;

    typedef MEMORYSTREAM_THREAD_LOCAL(GenericMesh) VertexStream;

    GenericVertexData(GenericMesh* owner, const Graphics::VertexSemantic& semantic, size_t index, Graphics::ValueType type);
    ~GenericVertexData();

    GenericVertexData(GenericVertexData&& ) = default;
    GenericVertexData& operator =(GenericVertexData&& ) = default;

    const Graphics::Name& Semantic() const { return _semantic; }
    size_t Index() const { return _index; }
    Graphics::ValueType Type() const { return _type; }

    size_t VertexCount() const { return _vertexCount; }
    size_t SizeInBytes() const { return checked_cast<size_t>(_stream.SizeInBytes()); }
    size_t StrideInBytes() const { return Graphics::ValueSizeInBytes(_type); }

    void Resize(size_t count, bool keepData = true);
    void Reserve(size_t count);
    MemoryView<const u8> VertexView(size_t v) const;
    void CopyVertex(size_t dst, size_t src);
    void ReadVertex(size_t v, Graphics::Value& dst) const;
    void WriteVertex(size_t v, const Graphics::Value& src);

    MemoryView<u8> MakeView() { return _stream.MakeView(); }
    MemoryView<const u8> MakeView() const { return _stream.MakeView(); }

    MemoryView<u8> SubRange(size_t start, size_t count);
    MemoryView<const u8> SubRange(size_t start, size_t count) const;

private:
    GenericMesh* _owner;

    Graphics::Name _semantic;
    Graphics::ValueType _type;
    size_t _index;

    mutable size_t _vertexCount;
    mutable VertexStream _stream;
};
//----------------------------------------------------------------------------
class GenericMesh : public RefCountable {
public:
    template <typename T>
    friend class GenericVertexSubPart;
    friend GenericVertexData;

    typedef MEMORYSTREAM_THREAD_LOCAL(GenericMesh) IndexStream;

    GenericMesh();
    ~GenericMesh();

    bool empty() const { return (_indices.empty() && _vertices.empty()); }

    size_t IndexCount() const { return _indexCount; }
    size_t TriangleCount() const { return (_indexCount / 3); }
    size_t VertexCount() const { return _vertexCount; }

    MemoryView<u32> Indices() { return _indices.MakeView().Cast<u32>(); }
    MemoryView<const u32> Indices() const { return _indices.MakeView().Cast<const u32>(); }

    MemoryView<GenericVertexData> Vertices() { return _vertices.MakeView(); }
    MemoryView<const GenericVertexData> Vertices() const { return _vertices.MakeView(); }

    Positions3f Position3f(size_t index);
    Positions3f Position3f_IFP(size_t index) const;
    Positions4f Position4f(size_t index);
    Positions4f Position4f_IFP(size_t index) const;

    TexCoords2f TexCoord2f(size_t index);
    TexCoords2f TexCoord2f_IFP(size_t index) const;
    TexCoords3f TexCoord3f(size_t index);
    TexCoords3f TexCoord3f_IFP(size_t index) const;
    TexCoords4f TexCoord4f(size_t index);
    TexCoords4f TexCoord4f_IFP(size_t index) const;

    Colors4f    Color4f(size_t index);
    Colors4f    Color4f_IFP(size_t index) const;

    Normals3f   Normal3f(size_t index);
    Normals3f   Normal3f_IFP(size_t index) const;

    Tangents3f  Tangent3f(size_t index);
    Tangents3f  Tangent3f_IFP(size_t index) const;
    Tangents4f  Tangent4f(size_t index);
    Tangents4f  Tangent4f_IFP(size_t index) const;

    Binormals3f Binormal3f(size_t index);
    Binormals3f Binormal3f_IFP(size_t index) const;

    void AddTriangle(size_t i0, size_t i1, size_t i2, size_t offset = 0);

    const GenericVertexData* GetVertexData(const Graphics::VertexSemantic& semantic, size_t index, Graphics::ValueType type) const;
    const GenericVertexData* GetVertexDataIFP(const Graphics::VertexSemantic& semantic, size_t index, Graphics::ValueType type) const;

    void AddVertexData(const Graphics::VertexSemantic& semantic, size_t index, Graphics::ValueType type);
    GenericVertexData* GetOrAddVertexData(const Graphics::VertexSemantic& semantic, size_t index, Graphics::ValueType type);

    bool AreVertexEquals(size_t v0, size_t v1) const;
    hash_t VertexHash(size_t v, size_t seed = 0 ) const;

    void VertexCopy(size_t dst, size_t src);
    void VertexSwap(size_t v0, size_t v1);
    void IndexAndVertexSwap(size_t v0, size_t v1);

    void Resize(size_t indexCount, size_t vertexCount, bool keepData = true);
    void Reserve(size_t indexCount, size_t vertexCount, bool additional = false);

    bool ExportIndices(const MemoryView<u16>& dst) const;
    bool ExportIndices(const MemoryView<u32>& dst) const;
    bool ExportIndices(Graphics::IndexElementSize eltSize, const MemoryView<u8>& dst) const;
    bool ExportVertices(const Graphics::VertexDeclaration* vdecl, const MemoryView<u8>& dst) const;

    template <typename _Allocator>
    void ExportIndices(RawStorage<u16, _Allocator>& dst) const {
        dst.Resize_DiscardData(IndexCount());
        ExportIndices(dst.MakeView());
    }

    template <typename _Allocator>
    void ExportIndices(RawStorage<u32, _Allocator>& dst) const {
        dst.Resize_DiscardData(IndexCount());
        ExportIndices(dst.MakeView());
    }

    template <typename _Allocator>
    void ExportIndices(Graphics::IndexElementSize eltSize, RawStorage<u8, _Allocator>& dst) const {
        dst.Resize_DiscardData(size_t(eltSize) * IndexCount());
        ExportIndices(eltSize, dst.MakeView());
    }

    template <typename T, typename _Allocator>
    void ExportVertices(const Graphics::VertexDeclaration* vdecl, RawStorage<T, _Allocator>& dst) const {
        dst.Resize_DiscardData(VertexCount());
        ExportVertices(vdecl, dst.MakeView().Cast<u8>());
    }

    void Clear();
    void ClearIndices();
    void ClearVertices();

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    size_t _indexCount;
    size_t _vertexCount;
    IndexStream _indices;
    FixedSizeStack<GenericVertexData, 6> _vertices;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core

#include "Core.Lattice/GenericMesh-inl.h"
