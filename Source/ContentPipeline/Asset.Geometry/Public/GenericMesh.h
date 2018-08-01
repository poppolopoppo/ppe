#pragma once

#include "Lattice.h"

#include "GenericMesh_fwd.h"

#include "Value.h"

#include "Allocator/PoolAllocator.h"
#include "Container/Stack.h"
#include "Memory/MemoryStream.h"
#include "Memory/RefPtr.h"

namespace PPE {
namespace Graphics {
enum class EIndexElementSize;
FWD_REFPTR(VertexDeclaration);
class FVertexSemantic;
}

namespace Lattice {
FWD_REFPTR(GenericMesh);
class FGenericVertexData;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class TGenericVertexSubPart {
public:
    STATIC_CONST_INTEGRAL(Graphics::EValueType, Type, Graphics::TValueTraits<T>::TypeId);
    STATIC_ASSERT(Type != Graphics::EValueType::Void);

    TGenericVertexSubPart() : TGenericVertexSubPart(nullptr) {}
    explicit TGenericVertexSubPart(const FGenericVertexData* data);

    size_t size() const { return (_data ? _data->VertexCount() : 0); }
    bool empty() const { return (0 == size()); }

    bool valid() const { return (nullptr != _data); }
    operator void* () const { return (void*)_data; }

    const FGenericVertexData* Data() const { return _data; }

    void Write(const T& src) const;
    void Write(const TMemoryView<const T>& src) const;
    TMemoryView<T> Append(size_t count) const;
    TMemoryView<T> Resize(size_t count, bool keepData = true) const;
    TMemoryView<T> MakeView() const;

private:
    const FGenericVertexData* _data;
};
//----------------------------------------------------------------------------
FWD_UNIQUEPTR(GenericVertexData);
class FGenericVertexData {
public:
    template <typename T>
    friend class TGenericVertexSubPart;

    typedef MEMORYSTREAM(GenericMesh) FVertexStream;

    FGenericVertexData(FGenericMesh* owner, const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type);
    ~FGenericVertexData();

    FGenericVertexData(FGenericVertexData&& ) = default;
    FGenericVertexData& operator =(FGenericVertexData&& ) = default;

    const Graphics::FName& Semantic() const { return _semantic; }
    size_t Index() const { return _index; }
    Graphics::EValueType Type() const { return _type; }

    size_t VertexCount() const { return _vertexCount; }
    size_t SizeInBytes() const { return checked_cast<size_t>(_stream.SizeInBytes()); }
    size_t StrideInBytes() const { return Graphics::ValueSizeInBytes(_type); }

    void Resize(size_t count, bool keepData = true);
    void Reserve(size_t count);

    TMemoryView<const u8> VertexView(size_t v) const;

    void CopyVertex(size_t dst, size_t src);
    void ReadVertex(size_t v, Graphics::FValue& dst) const;
    void WriteVertex(size_t v, const Graphics::FValue& src);

    TMemoryView<u8> MakeView() { return _stream.MakeView(); }
    TMemoryView<const u8> MakeView() const { return _stream.MakeView(); }

    TMemoryView<u8> SubRange(size_t start, size_t count);
    TMemoryView<const u8> SubRange(size_t start, size_t count) const;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FGenericMesh* _owner;

    Graphics::FName _semantic;
    Graphics::EValueType _type;
    size_t _index;

    mutable size_t _vertexCount;
    mutable FVertexStream _stream;
};
//----------------------------------------------------------------------------
class FGenericMesh : public FRefCountable {
public:
    template <typename T>
    friend class TGenericVertexSubPart;
    friend FGenericVertexData;

    typedef MEMORYSTREAM(GenericMesh) FIndexStream;

    FGenericMesh();
    ~FGenericMesh();

    bool empty() const { return (_indices.empty() && _vertices.empty()); }

    size_t IndexCount() const { return _indexCount; }
    size_t TriangleCount() const { return (_indexCount / 3); }
    size_t VertexCount() const { return _vertexCount; }
    size_t SubPartCount() const { return _vertices.size(); }

    TMemoryView<u32> Indices() { return _indices.MakeView().Cast<u32>(); }
    TMemoryView<const u32> Indices() const { return _indices.MakeView().Cast<const u32>(); }

    TMemoryView<const UGenericVertexData> Vertices() const { return _vertices.MakeView(); }

    FPositions3f Position3f(size_t index);
    FPositions3f Position3f_IFP(size_t index) const;
    FPositions4f Position4f(size_t index);
    FPositions4f Position4f_IFP(size_t index) const;

    FTexCoords2f TexCoord2f(size_t index);
    FTexCoords2f TexCoord2f_IFP(size_t index) const;
    FTexCoords3f TexCoord3f(size_t index);
    FTexCoords3f TexCoord3f_IFP(size_t index) const;
    FTexCoords4f TexCoord4f(size_t index);
    FTexCoords4f TexCoord4f_IFP(size_t index) const;

    FColors4f    Color4f(size_t index);
    FColors4f    Color4f_IFP(size_t index) const;

    FNormals3f   Normal3f(size_t index);
    FNormals3f   Normal3f_IFP(size_t index) const;
    FNormals4f   Normal4f(size_t index);
    FNormals4f   Normal4f_IFP(size_t index) const;

    FTangents3f  Tangent3f(size_t index);
    FTangents3f  Tangent3f_IFP(size_t index) const;
    FTangents4f  Tangent4f(size_t index);
    FTangents4f  Tangent4f_IFP(size_t index) const;

    FBinormals3f Binormal3f(size_t index);
    FBinormals3f Binormal3f_IFP(size_t index) const;

    void AddIndices(const TMemoryView<const u32>& indices);
    void AddTriangle(size_t i0, size_t i1, size_t i2, size_t offset = 0);

    const FGenericVertexData* GetVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) const;
    const FGenericVertexData* GetVertexDataIFP(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type) const;

    FGenericVertexData* AddVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type);
    FGenericVertexData* GetOrAddVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type);

    void RemoveVertexData(const FGenericVertexData* data);
    void RemoveVertexData(const Graphics::FVertexSemantic& semantic, size_t index, Graphics::EValueType type);

    template <typename T>
    void RemoveSubPart(const TGenericVertexSubPart<T>& subPart) {
        RemoveVertexData(subPart.Data());
    }

    bool AreVertexEquals(size_t v0, size_t v1) const;
    hash_t VertexHash(size_t v, size_t seed = 0 ) const;

    void VertexCopy(size_t dst, size_t src);
    void VertexSwap(size_t v0, size_t v1);
    void IndexAndVertexSwap(size_t v0, size_t v1);

    void Resize(size_t indexCount, size_t vertexCount, bool keepData = true);
    void Reserve(size_t indexCount, size_t vertexCount, bool additional = false);

    void CleanAndOptimize(size_t index = 0);

    bool ExportIndices(const TMemoryView<u16>& dst) const;
    bool ExportIndices(const TMemoryView<u32>& dst) const;
    bool ExportIndices(Graphics::EIndexElementSize eltSize, const TMemoryView<u8>& dst) const;
    bool ExportVertices(const Graphics::FVertexDeclaration* vdecl, const TMemoryView<u8>& dst) const;

    template <typename _Allocator>
    void ExportIndices(TRawStorage<u16, _Allocator>& dst) const {
        dst.Resize_DiscardData(IndexCount());
        ExportIndices(dst.MakeView());
    }

    template <typename _Allocator>
    void ExportIndices(TRawStorage<u32, _Allocator>& dst) const {
        dst.Resize_DiscardData(IndexCount());
        ExportIndices(dst.MakeView());
    }

    template <typename _Allocator>
    void ExportIndices(Graphics::EIndexElementSize eltSize, TRawStorage<u8, _Allocator>& dst) const {
        dst.Resize_DiscardData(size_t(eltSize) * IndexCount());
        ExportIndices(eltSize, dst.MakeView());
    }

    template <typename _Allocator>
    void ExportVertices(const Graphics::FVertexDeclaration* vdecl, TRawStorage<u8, _Allocator>& dst) const {
        dst.Resize_DiscardData(VertexCount() * vdecl->SizeInBytes());
        ExportVertices(vdecl, dst.MakeView());
    }

    template <typename T, typename _Allocator>
    void ExportVertices(const Graphics::FVertexDeclaration* vdecl, TRawStorage<T, _Allocator>& dst) const {
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
    FIndexStream _indices;
    TFixedSizeStack<UGenericVertexData, 8> _vertices;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace PPE

#include "GenericMesh-inl.h"
