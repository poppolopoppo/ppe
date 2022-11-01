#pragma once

#include "MeshBuilder_fwd.h"

#include "RHI/ResourceId.h"
#include "RHI/VertexDesc.h"

#include "Container/FlatMap.h"
#include "Memory/MemoryStream.h"
#include "Memory/PtrRef.h"
#include "Meta/Optional.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FGenericVertexData {
public:
    template <typename T>
    friend class TGenericVertexSubPart;

    typedef MEMORYSTREAM(MeshBuilder) FVertexStream;

    FGenericVertexData(FGenericMesh& owner, const RHI::FVertexID& semantic, RHI::EVertexFormat format) NOEXCEPT;
    ~FGenericVertexData();

    FGenericVertexData(FGenericVertexData&& ) = default;
    FGenericVertexData& operator =(FGenericVertexData&& ) = default;

    CONSTF const RHI::FVertexID& Semantic() const { return _semantic; }
    CONSTF RHI::EVertexFormat Format() const { return _format; }

    CONSTF size_t VertexCount() const { return _vertexCount; }
    CONSTF size_t SizeInBytes() const { return checked_cast<size_t>(_stream.SizeInBytes()); }
    CONSTF size_t StrideInBytes() const NOEXCEPT;

    void Resize(size_t count, bool keepData = true);
    void Reserve(size_t count);

    void CopyVertex(size_t dst, size_t src);
    void ReadVertex(size_t v, const FRawMemory& dst) const;
    void WriteVertex(size_t v, const FRawMemoryConst& src);

    NODISCARD FRawMemory Vertex(size_t v) NOEXCEPT { return SubRange(v, 1); }
    NODISCARD FRawMemoryConst Vertex(size_t v) const NOEXCEPT { return SubRange(v, 1); }

    NODISCARD FRawMemory MakeView() { return _stream.MakeView(); }
    NODISCARD FRawMemoryConst MakeView() const { return _stream.MakeView(); }

    NODISCARD FRawMemory SubRange(size_t start, size_t count);
    NODISCARD FRawMemoryConst SubRange(size_t start, size_t count) const;

    template <typename T>
    NODISCARD Meta::TOptional<TGenericVertexSubPart<T>> MakeSubPart() const NOEXCEPT;

private:
    FVertexStream _stream;

    const TPtrRef<FGenericMesh> _owner;
    const RHI::FVertexID _semantic;
    const RHI::EVertexFormat _format;

    size_t _vertexCount{ 0 };
};
//----------------------------------------------------------------------------
template <typename T>
class TGenericVertexSubPart {
public:
    STATIC_CONST_INTEGRAL(RHI::EVertexFormat, Format, RHI::VertexAttrib<T>());

    TGenericVertexSubPart() = default;
    explicit TGenericVertexSubPart(FGenericVertexData* data);

    CONSTF bool Valid() const { return (nullptr != _data); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    CONSTF size_t size() const { return (_data ? _data->VertexCount() : 0); }
    CONSTF bool empty() const { return (0 == size()); }

    CONSTF const FGenericVertexData* Data() const { return _data; }

    void Write(const T& src) const;
    void Write(const TMemoryView<const T>& src) const;

    TMemoryView<T> Append(size_t count) const;
    TMemoryView<T> Resize(size_t count, bool keepData = true) const;
    TMemoryView<T> MakeView() const NOEXCEPT;

private:
    TPtrRef<FGenericVertexData> _data;
};
//----------------------------------------------------------------------------
class FGenericMesh {
public:
    template <typename T>
    friend class TGenericVertexSubPart;
    friend FGenericVertexData;

    typedef MEMORYSTREAM(MeshBuilder) FIndexStream;

    FGenericMesh();
    ~FGenericMesh();

    bool empty() const { return (_indices.empty() && _vertices.empty()); }

    CONSTF size_t IndexCount() const { return _indexCount; }
    CONSTF size_t TriangleCount() const { return (_indexCount / 3); }
    CONSTF size_t VertexCount() const { return _vertexCount; }
    CONSTF size_t SubPartCount() const { return _vertices.size(); }

    CONSTF TMemoryView<u32> Indices() { return _indices.MakeView().Cast<u32>(); }
    CONSTF TMemoryView<const u32> Indices() const { return _indices.MakeView().Cast<const u32>(); }

    CONSTF auto Vertices() const { return _vertices.Values(); }

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

    FGenericVertexData& VertexData(const RHI::FVertexID& semantic) const NOEXCEPT;
    FGenericVertexData* VertexDataIFP(const RHI::FVertexID& semantic) const NOEXCEPT;

    FGenericVertexData& AddVertexData(const RHI::FVertexID& semantic, RHI::EVertexFormat format);
    FGenericVertexData& GetOrAddVertexData(const RHI::FVertexID& semantic, RHI::EVertexFormat format);

    void RemoveVertexData(const FGenericVertexData* data);
    void RemoveVertexData(const RHI::FVertexID& semantic);

    template <typename T>
    void RemoveSubPart(const TGenericVertexSubPart<T>& subPart) {
        RemoveVertexData(subPart.Data());
    }

    bool AreVertexEquals(size_t v0, size_t v1) const;
    hash_t VertexHash(size_t v, hash_t seed = 0) const;

    void VertexCopy(size_t dst, size_t src);
    void VertexSwap(size_t v0, size_t v1);
    void IndexAndVertexSwap(size_t v0, size_t v1);

    void Resize(size_t indexCount, size_t vertexCount, bool keepData = true);
    void Reserve(size_t indexCount, size_t vertexCount, bool additional = false);

    void CleanAndOptimize(size_t index = 0);

    NODISCARD bool ExportIndices(const TMemoryView<u16>& dst) const;
    NODISCARD bool ExportIndices(const TMemoryView<u32>& dst) const;
    NODISCARD bool ExportIndices(RHI::EIndexFormat format, const FRawMemory& dst) const;
    NODISCARD bool ExportVertices(const RHI::FVertexInputState& vdecl, const RHI::FVertexBufferID& vertexBuffer, const FRawMemory& dst) const;

    template <typename _Index, typename _Allocator>
    NODISCARD bool ExportIndices(TRawStorage<_Index, _Allocator>& dst) const {
        dst.Resize_DiscardData(IndexCount());
        return ExportIndices(RHI::IndexAttrib<u16>(), dst.MakeView());
    }

    template <typename _Allocator>
    NODISCARD bool ExportIndices(RHI::EIndexFormat format, TRawStorage<u8, _Allocator>& dst) const {
        dst.Resize_DiscardData(RHI::EIndexFormat_SizeOf(format) * IndexCount());
        return ExportIndices(format, dst.MakeView());
    }

    template <typename _Allocator>
    NODISCARD bool ExportVertices(const RHI::FVertexInputState& vdecl, TRawStorage<u8, _Allocator>& dst) const {
        dst.Resize_DiscardData(VertexCount() * vdecl.());
        return ExportVertices(vdecl, dst.MakeView());
    }

    template <typename T, typename _Allocator>
    NODISCARD bool ExportVertices(const RHI::FVertexInputState& vdecl, TRawStorage<T, _Allocator>& dst) const {
        dst.Resize_DiscardData(VertexCount());
        return ExportVertices(vdecl, dst.MakeView().Cast<u8>());
    }

    NODISCARD bool Validate() const NOEXCEPT;

    void Clear();
    void ClearIndices();
    void ClearVertices();

private:
    FLATMAP_INSITU(MeshBuilder, RHI::FVertexID, UGenericVertexData, 6) _vertices;
    FIndexStream _indices;

    size_t _indexCount{ 0 };
    size_t _vertexCount{ 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE

#include "GenericMesh-inl.h"
