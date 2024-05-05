#pragma once

#include "MeshBuilder_fwd.h"

#include "RHI/EnumHelpers.h"
#include "RHI/ResourceId.h"
#include "RHI/VertexInputState.h"
#include "RHI/VertexDesc.h"

#include "Container/FlatMap.h"
#include "IO/Filename.h"
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

    PPE_MESHBUILDER_API FGenericVertexData(FGenericMesh& owner, const RHI::FVertexID& semantic, RHI::EVertexFormat format) NOEXCEPT;
    PPE_MESHBUILDER_API ~FGenericVertexData();

    FGenericVertexData(FGenericVertexData&& ) = default;
    FGenericVertexData& operator =(FGenericVertexData&& ) = delete;

    NODISCARD CONSTF const RHI::FVertexID& Semantic() const { return _semantic; }
    NODISCARD CONSTF RHI::EVertexFormat Format() const { return _format; }

    NODISCARD CONSTF size_t VertexCount() const { return _vertexCount; }
    NODISCARD CONSTF size_t SizeInBytes() const { return checked_cast<size_t>(_stream.SizeInBytes()); }
    NODISCARD PPE_MESHBUILDER_API CONSTF size_t StrideInBytes() const NOEXCEPT;

    PPE_MESHBUILDER_API void Resize(size_t count, bool keepData = true);
    PPE_MESHBUILDER_API void Reserve(size_t count);

    PPE_MESHBUILDER_API void CopyVertex(size_t dst, size_t src);
    PPE_MESHBUILDER_API void ReadVertex(size_t v, const FRawMemory& dst) const;
    PPE_MESHBUILDER_API void WriteVertex(size_t v, const FRawMemoryConst& src);

    NODISCARD FRawMemory Vertex(size_t v) NOEXCEPT { return SubRange(v, 1); }
    NODISCARD FRawMemoryConst Vertex(size_t v) const NOEXCEPT { return SubRange(v, 1); }

    NODISCARD FRawMemory MakeView() { return _stream.MakeView(); }
    NODISCARD FRawMemoryConst MakeView() const { return _stream.MakeView(); }

    NODISCARD PPE_MESHBUILDER_API FRawMemory SubRange(size_t start, size_t count);
    NODISCARD PPE_MESHBUILDER_API FRawMemoryConst SubRange(size_t start, size_t count) const;

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

    explicit TGenericVertexSubPart(FGenericVertexData* pDataIFP) NOEXCEPT;
    explicit TGenericVertexSubPart(FGenericVertexData& data) NOEXCEPT
    :   TGenericVertexSubPart(&data)
    {}

    NODISCARD CONSTF bool Valid() const { return (!!_data); }
    PPE_FAKEBOOL_OPERATOR_DECL() { return Valid(); }

    NODISCARD CONSTF size_t size() const { return (_data ? _data->VertexCount() : 0); }
    NODISCARD CONSTF bool empty() const { return (0 == size()); }

    NODISCARD CONSTF const FGenericVertexData* Data() const { return _data; }

    void Write(const T& src) const;
    void Write(const TMemoryView<const T>& src) const;

    TMemoryView<T> Append(size_t count) const;
    TMemoryView<T> Resize(size_t count, bool keepData = true) const;
    NODISCARD TMemoryView<T> MakeView() const NOEXCEPT;

    NODISCARD T& operator [](size_t index) const NOEXCEPT {
        return MakeView().at(index);
    }

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

    FGenericMesh() = default;
    PPE_MESHBUILDER_API ~FGenericMesh();

    FGenericMesh(FGenericMesh&& rvalue) NOEXCEPT { operator =(std::move(rvalue)); }
    PPE_MESHBUILDER_API FGenericMesh& operator =(FGenericMesh&& rvalue) NOEXCEPT;

    NODISCARD bool empty() const { return (_indexCount == 0 and _vertexCount == 0); }

    NODISCARD CONSTF u32 IndexCount() const { return checked_cast<u32>(_indexCount); }
    NODISCARD CONSTF u32 TriangleCount() const { return checked_cast<u32>(_indexCount / 3); }
    NODISCARD CONSTF u32 VertexCount() const { return checked_cast<u32>(_vertexCount); }
    NODISCARD CONSTF u32 SubPartCount() const { return checked_cast<u32>(_vertices.size()); }

    NODISCARD CONSTF TMemoryView<u32> Indices() { return _indices.MakeView().Cast<u32>(); }
    NODISCARD CONSTF TMemoryView<const u32> Indices() const { return _indices.MakeView().Cast<const u32>(); }

    NODISCARD CONSTF auto Vertices() const { return _vertices.Values(); }

    const Meta::TOptional<FFilename>& SourceFile() const { return _sourceFile; }
    void SetSourceFile(const FFilename& value) { _sourceFile = value; }

    NODISCARD PPE_MESHBUILDER_API FPositions3f Position3f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FPositions3f Position3f_IFP(size_t index) const;
    NODISCARD PPE_MESHBUILDER_API FPositions4f Position4f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FPositions4f Position4f_IFP(size_t index) const;

    NODISCARD PPE_MESHBUILDER_API FTexcoords2f Texcoord2f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FTexcoords2f Texcoord2f_IFP(size_t index) const;
    NODISCARD PPE_MESHBUILDER_API FTexcoords3f Texcoord3f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FTexcoords3f Texcoord3f_IFP(size_t index) const;
    NODISCARD PPE_MESHBUILDER_API FTexcoords4f Texcoord4f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FTexcoords4f Texcoord4f_IFP(size_t index) const;

    NODISCARD PPE_MESHBUILDER_API FColors4f    Color4f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FColors4f    Color4f_IFP(size_t index) const;

    NODISCARD PPE_MESHBUILDER_API FNormals3f   Normal3f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FNormals3f   Normal3f_IFP(size_t index) const;
    NODISCARD PPE_MESHBUILDER_API FNormals4f   Normal4f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FNormals4f   Normal4f_IFP(size_t index) const;

    NODISCARD PPE_MESHBUILDER_API FTangents3f  Tangent3f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FTangents3f  Tangent3f_IFP(size_t index) const;
    NODISCARD PPE_MESHBUILDER_API FTangents4f  Tangent4f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FTangents4f  Tangent4f_IFP(size_t index) const;

    NODISCARD PPE_MESHBUILDER_API FBinormals3f Binormal3f(size_t index);
    NODISCARD PPE_MESHBUILDER_API FBinormals3f Binormal3f_IFP(size_t index) const;

    NODISCARD u32 Index(size_t index) const { return Indices()[index]; }
    void SetIndex(size_t index, u32 value) { Indices()[index] = value; }

    NODISCARD PPE_MESHBUILDER_API uint3 Triangle(size_t index) const NOEXCEPT;
    PPE_MESHBUILDER_API void SetTriangle(size_t index, const uint3& vertices) NOEXCEPT;

    PPE_MESHBUILDER_API void AddIndex(size_t index);
    PPE_MESHBUILDER_API void AddIndices(const TMemoryView<const u32>& indices);
    PPE_MESHBUILDER_API void AddTriangle(size_t i0, size_t i1, size_t i2, size_t offset = 0);
    PPE_MESHBUILDER_API void AddTriangle(const uint3& vertices, size_t offset = 0);

    NODISCARD PPE_MESHBUILDER_API FGenericVertexData& VertexData(const RHI::FVertexID& semantic) const NOEXCEPT;
    NODISCARD PPE_MESHBUILDER_API FGenericVertexData* VertexDataIFP(const RHI::FVertexID& semantic) const NOEXCEPT;
    NODISCARD PPE_MESHBUILDER_API FGenericVertexData* VertexDataIFP(const RHI::FVertexID& semantic, RHI::EVertexFormat format) const NOEXCEPT;

    PPE_MESHBUILDER_API FGenericVertexData& AddVertexData(const RHI::FVertexID& semantic, RHI::EVertexFormat format);
    PPE_MESHBUILDER_API FGenericVertexData& GetOrAddVertexData(const RHI::FVertexID& semantic, RHI::EVertexFormat format);

    PPE_MESHBUILDER_API void RemoveVertexData(const FGenericVertexData* data);
    PPE_MESHBUILDER_API void RemoveVertexData(const RHI::FVertexID& semantic);

    template <typename T>
    void RemoveSubPart(const TGenericVertexSubPart<T>& subPart) {
        RemoveVertexData(subPart.Data());
    }

    PPE_MESHBUILDER_API UGenericVertexData StealVertexData(const FGenericVertexData* data);
    PPE_MESHBUILDER_API UGenericVertexData StealVertexData(const RHI::FVertexID& semantic);

    template <typename T>
    UGenericVertexData StealVertexData(const TGenericVertexSubPart<T>& subPart) {
        return StealVertexData(subPart.Data());
    }

    NODISCARD PPE_MESHBUILDER_API bool AreVertexEquals(size_t v0, size_t v1) const;
    NODISCARD PPE_MESHBUILDER_API hash_t VertexHash(size_t v, hash_t seed = 0) const;

    PPE_MESHBUILDER_API void VertexCopy(size_t dst, size_t src);
    PPE_MESHBUILDER_API void VertexSwap(size_t v0, size_t v1);
    PPE_MESHBUILDER_API void IndexAndVertexSwap(size_t v0, size_t v1);

    PPE_MESHBUILDER_API void Resize(size_t indexCount, size_t vertexCount, bool keepData = true);
    PPE_MESHBUILDER_API void Reserve(size_t indexCount, size_t vertexCount, bool additional = false);

    PPE_MESHBUILDER_API void CleanAndOptimize(const FMeshBuilderSettings& settings, size_t index = 0);

    NODISCARD PPE_MESHBUILDER_API bool ExportIndices(const TMemoryView<u16>& dst) const;
    NODISCARD PPE_MESHBUILDER_API bool ExportIndices(const TMemoryView<u32>& dst) const;
    NODISCARD PPE_MESHBUILDER_API bool ExportIndices(RHI::EIndexFormat format, const FRawMemory& dst) const;
    NODISCARD PPE_MESHBUILDER_API bool ExportVertices(const RHI::FVertexInputState& vdecl, const RHI::FVertexBufferID& vertexBuffer, const FRawMemory& dst) const;

    template <typename _Index, typename _Allocator>
    NODISCARD bool ExportIndices(TRawStorage<_Index, _Allocator>& dst) const {
        dst.Resize_DiscardData(IndexCount());
        return ExportIndices(RHI::IndexAttrib<u16>(), dst.MakeView());
    }

    template <typename _Allocator>
    NODISCARD bool ExportIndices(RHI::EIndexFormat format, TRawStorage<u8, _Allocator>& dst) const {
        dst.Resize_DiscardData(static_cast<size_t>(RHI::EIndexFormat_SizeOf(format)) * IndexCount());
        return ExportIndices(format, dst.MakeView());
    }

    template <typename _Allocator>
    NODISCARD bool ExportVertices(const RHI::FVertexInputState& vdecl, const RHI::FVertexBufferID& vertexBuffer, TRawStorage<u8, _Allocator>& dst) const {
        const RHI::FVertexBufferBinding& bufferBinding = vdecl.BufferBindings[vertexBuffer];
        dst.Resize_DiscardData(static_cast<size_t>(VertexCount()) * bufferBinding.Stride);
        return ExportVertices(vdecl, vertexBuffer, dst.MakeView());
    }

    template <typename T, typename _Allocator>
    NODISCARD bool ExportVertices(const RHI::FVertexInputState& vdecl, const RHI::FVertexBufferID& vertexBuffer, TRawStorage<T, _Allocator>& dst) const {
        Assert_NoAssume(vdecl.BufferBindings[vertexBuffer].Stride == sizeof(T));
        dst.Resize_DiscardData(VertexCount());
        return ExportVertices(vdecl, vertexBuffer, MakeRawView(dst.MakeView()));
    }

    NODISCARD PPE_MESHBUILDER_API bool Validate() const NOEXCEPT;

    PPE_MESHBUILDER_API void Clear();
    PPE_MESHBUILDER_API void ClearIndices();
    PPE_MESHBUILDER_API void ClearVertices();

private:
    FLATMAP_INSITU(MeshBuilder, RHI::FVertexID, UGenericVertexData, 6) _vertices;
    FIndexStream _indices;

    Meta::TOptional<FFilename> _sourceFile;

    size_t _indexCount{ 0 };
    size_t _vertexCount{ 0 };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE

#include "GenericMesh-inl.h"
