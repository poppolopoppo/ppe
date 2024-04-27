// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Mesh/GenericMesh.h"

#include "Mesh/GenericMeshHelpers.h"
#include "Mesh/MeshFormat.h"

#include "RHI/EnumHelpers.h"
#include "RHI/EnumToString.h"
#include "RHI/VertexInputState.h"

#include "Container/Hash.h"
#include "Diagnostic/FeedbackContext.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "Maths/ScalarBoundingBox.h"
#include "Meta/Utility.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGenericMesh::~FGenericMesh() {
    Clear();
}
//----------------------------------------------------------------------------
FGenericMesh& FGenericMesh::operator =(FGenericMesh&& rvalue) NOEXCEPT {
    Clear();

    _vertices = std::move(rvalue._vertices);
    _indices = std::move(rvalue._indices);
    _sourceFile = std::move(rvalue._sourceFile);
    _indexCount = std::move(rvalue._indexCount);
    _vertexCount = std::move(rvalue._vertexCount);

    rvalue._indexCount = rvalue._vertexCount = 0;

    return (*this);
}
//----------------------------------------------------------------------------
void FGenericMesh::AddIndex(size_t index) {
    _indexCount++;
    _indices.WritePOD(checked_cast<u32>(index));
}
//----------------------------------------------------------------------------
void FGenericMesh::AddIndices(const TMemoryView<const u32>& indices) {
    _indexCount += indices.size();
    _indices.WriteView(indices);
}
//----------------------------------------------------------------------------
void FGenericMesh::AddTriangle(const uint3& vertices, size_t offset/* = 0 */) {
    AddTriangle(vertices.x, vertices.y, vertices.z, offset);
}
//----------------------------------------------------------------------------
void FGenericMesh::AddTriangle(size_t i0, size_t i1, size_t i2, size_t offset/* = 0 */) {
    _indexCount += 3;
    _indices.WritePOD(checked_cast<u32>(offset + i0));
    _indices.WritePOD(checked_cast<u32>(offset + i1));
    _indices.WritePOD(checked_cast<u32>(offset + i2));
}
//----------------------------------------------------------------------------
uint3 FGenericMesh::Triangle(size_t index) const NOEXCEPT {
    Assert(index * 3 + 2 < _indexCount);
    const TMemoryView<const u32> indices = Indices();
    return {
        indices[index * 3 + 0],
        indices[index * 3 + 1],
        indices[index * 3 + 2]
    };
}
//----------------------------------------------------------------------------
void FGenericMesh::SetTriangle(size_t index, const uint3& vertices) NOEXCEPT {
    Assert(index * 3 + 2 < _indexCount);
    const TMemoryView<u32> indices = Indices();
    indices[index * 3 + 0] = vertices.x;
    indices[index * 3 + 1] = vertices.y;
    indices[index * 3 + 2] = vertices.z;
}
//----------------------------------------------------------------------------
FGenericVertexData& FGenericMesh::VertexData(const RHI::FVertexID& semantic) const NOEXCEPT {
    FGenericVertexData* const vertices = VertexDataIFP(semantic);
    AssertRelease(nullptr != vertices);
    return (*vertices);
}
//----------------------------------------------------------------------------
FGenericVertexData* FGenericMesh::VertexDataIFP(const RHI::FVertexID& semantic) const NOEXCEPT {
    Assert(semantic.Valid());

    auto it = _vertices.find(semantic);
    if (_vertices.end() != it)
        return it->second.get();

    return nullptr;
}
//----------------------------------------------------------------------------
FGenericVertexData* FGenericMesh::VertexDataIFP(const RHI::FVertexID& semantic, RHI::EVertexFormat format) const NOEXCEPT {
    Assert(semantic.Valid());

    auto it = _vertices.find(semantic);
    if (_vertices.end() != it) {
        if (format == it->second->Format())
            return it->second.get();
    }

    return nullptr;
}
//----------------------------------------------------------------------------
FGenericVertexData& FGenericMesh::AddVertexData(const RHI::FVertexID& semantic, RHI::EVertexFormat format) {
    Assert_NoAssume(nullptr == VertexDataIFP(semantic));

    UGenericVertexData vertex = MakeUnique<FGenericVertexData>(*this, semantic, format);
    FGenericVertexData& result = *vertex;

    _vertices.Emplace_AssertUnique(RHI::FVertexID(semantic), std::move(vertex));
    return result;
}
//----------------------------------------------------------------------------
FGenericVertexData& FGenericMesh::GetOrAddVertexData(const RHI::FVertexID& semantic, RHI::EVertexFormat format) {
    if (FGenericVertexData* result = VertexDataIFP(semantic)) {
        Assert_NoAssume(result->Format() == format);
        return *result;
    }

    return AddVertexData(semantic, format);
}
//----------------------------------------------------------------------------
void FGenericMesh::RemoveVertexData(const FGenericVertexData* data) {
    Assert(data);

#if USE_PPE_ASSERT
    const auto it = _vertices.find(data->Semantic());
    AssertRelease(_vertices.end() != it);
    AssertRelease(it->second.get() == data);

    _vertices.Erase(it);
#else
    RemoveVertexData(data->Semantic());
#endif
}
//----------------------------------------------------------------------------
void FGenericMesh::RemoveVertexData(const RHI::FVertexID& semantic) {
    Assert(semantic.Valid());

    VerifyRelease(_vertices.Erase(semantic));
}
//----------------------------------------------------------------------------
UGenericVertexData FGenericMesh::StealVertexData(const FGenericVertexData* data) {
    Assert(data);

    return StealVertexData(data->Semantic());
}
//----------------------------------------------------------------------------
UGenericVertexData FGenericMesh::StealVertexData(const RHI::FVertexID& semantic) {
    Assert(semantic.Valid());
    UGenericVertexData result;

    const auto it = _vertices.find(semantic);
    if (_vertices.end() != it) {
        result = std::move(it->second);
        _vertices.Erase(it);
    }
    return result;
}
//----------------------------------------------------------------------------
bool FGenericMesh::AreVertexEquals(size_t v0, size_t v1) const {
    Assert(v0 < _vertexCount);
    Assert(v1 < _vertexCount);
    if (v0 == v1)
        return true;

    for (const UGenericVertexData& vertices : _vertices.Values()) {
        Assert_NoAssume(_vertexCount == vertices->VertexCount());
        const size_t strideInBytes = vertices->StrideInBytes();

        const FRawMemoryConst rawData = vertices->MakeView();
        const int cmp = FPlatformMemory::Memcmp(
            rawData.SubRange(v0 * strideInBytes, strideInBytes).data(),
            rawData.SubRange(v1 * strideInBytes, strideInBytes).data(),
            strideInBytes );

        if (cmp)
            return false;
    }

    return true;
}
//----------------------------------------------------------------------------
hash_t FGenericMesh::VertexHash(size_t v, hash_t seed/* = 0 */) const {
    Assert(v < _vertexCount);

    hash_t result = seed;

    for (const UGenericVertexData& vertices : _vertices.Values()) {
        Assert(_vertexCount == vertices->VertexCount());
        const FRawMemoryConst& rawData = vertices->MakeView();
        const size_t strideInBytes = vertices->StrideInBytes();

        hash_combine(result, vertices->Semantic(), vertices->Format(), vertices->VertexCount());

        const auto subPart = rawData.SubRange(v * strideInBytes, strideInBytes);
        result = FPlatformHash::HashMem(result, subPart.data(), subPart.SizeInBytes());
    }

    return result;
}
//----------------------------------------------------------------------------
void FGenericMesh::VertexCopy(size_t dst, size_t src) {
    Assert(dst < _vertexCount);
    Assert(src < _vertexCount);
    if (src == dst)
        return;

    for (const UGenericVertexData& vertices : _vertices.Values()) {
        Assert(_vertexCount == vertices->VertexCount());
        const size_t strideInBytes = vertices->StrideInBytes();
        const FRawMemory rawData = vertices->MakeView();

        FPlatformMemory::Memcpy(
            rawData.SubRange(dst * strideInBytes, strideInBytes).data(),
            rawData.SubRange(src * strideInBytes, strideInBytes).data(),
            strideInBytes );
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::VertexSwap(size_t v0, size_t v1) {
    Assert(v0 < _vertexCount);
    Assert(v1 < _vertexCount);
    if (v0 == v1)
        return;

    for (const UGenericVertexData& vertices : _vertices.Values()) {
        Assert(_vertexCount == vertices->VertexCount());
        const size_t strideInBytes = vertices->StrideInBytes();
        const FRawMemory rawData = vertices->MakeView();

        FPlatformMemory::Memswap(
            rawData.SubRange(v0 * strideInBytes, strideInBytes).data(),
            rawData.SubRange(v1 * strideInBytes, strideInBytes).data(),
            strideInBytes );
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::IndexAndVertexSwap(size_t v0, size_t v1) {
    VertexSwap(v0, v1);

    const u32 iv0 = checked_cast<u32>(v0);
    const u32 iv1 = checked_cast<u32>(v1);

    for (u32& index : _indices.MakeView().Cast<u32>()) {
        if (index == v0)
            index = iv1;
        else if (index == v1)
            index = iv0;
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::Resize(size_t indexCount, size_t vertexCount, bool keepData/* = true */) {
    if (_indexCount != indexCount) {
        _indexCount = indexCount;
        _indices.resize(indexCount * sizeof(u32), keepData);
    }
    if (_vertexCount != vertexCount) {
        _vertexCount = vertexCount;
        for (const UGenericVertexData& vertices : _vertices.Values())
            vertices->Resize(vertexCount, keepData);
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::Reserve(size_t indexCount, size_t vertexCount, bool additional/* = false */) {
    if (additional) {
        indexCount += _indexCount;
        vertexCount += _vertexCount;
    }
    if (_indexCount < indexCount) {
        _indices.reserve(indexCount * sizeof(u32));
    }
    if (_vertexCount < vertexCount) {
        for (const UGenericVertexData& vertices : _vertices.Values())
            vertices->Reserve(vertexCount);
    }
}
//----------------------------------------------------------------------------
void FGenericMesh::CleanAndOptimize(const FMeshBuilderSettings& settings, size_t index) {
    if (!Position3f_IFP(index) && !Position4f_IFP(index))
        return;

    Assert_NoAssume(Validate());
    DEFERRED {
        Assert_NoAssume(Validate());
    };

    FFeedbackProgressBar pbar{"Clean and optimize mesh"_view};

    if (settings.MergeDuplicateVertices) {
        pbar.Print("Remove mesh duplicate vertices"_view);
        const size_t duplicated = MergeDuplicateVertices(*this);
        PPE_SLOG(MeshBuilder, Info, "remove duplicate vertices", {
            {"Removed", duplicated},
            {"Total", _vertexCount},
            {"Percent", (duplicated * 100.f) / _vertexCount}
        });
    }

    if (settings.Transform.has_value()) {
        pbar.Print("Transform mesh"_view);
        Transform(*this, index, *settings.Transform);
    }

    pbar.Print("Compute mesh bounds"_view);
    const FAabb3f bounds = ComputeBounds(*this, index);
    const float minDistance = bounds.Extents().MinComponent() * 0.001f; // 0.01%
    const float minArea = (minDistance * minDistance) / 2.f;
    PPE_SLOG(MeshBuilder, Info, "computed mesh bounds", {
        {"Min", Opaq::Format(bounds.Min())},
        {"Max", Opaq::Format(bounds.Max())},
    });

    if (settings.MergeCloseVertices) {
        pbar.Print("Merge mesh close vertices"_view);
        const size_t close = MergeCloseVertices(*this, index, minDistance);
        PPE_LOG(MeshBuilder, Info, "removed {0}/{1} close vertices (epsilon = {2}) = {3:f2}%", close, _vertexCount, minDistance, close * 100.f / _vertexCount);
    }

    if (settings.RemoveZeroAreaTriangles) {
        pbar.Print("Remove mesh zero area triangles"_view);
        const size_t zero = RemoveZeroAreaTriangles(*this, index, minArea);
        PPE_SLOG(MeshBuilder, Info, "remove zero area triangles", {
            {"Removed", zero},
            {"Total", _indexCount + zero},
            {"Percent", (zero * 100.f) / (_indexCount + zero)},
            {"MinArea", minArea},
        });
    }

    if (settings.RemoveUnusedVertices) {
        pbar.Print("Remove mesh unused vertices"_view);
        const size_t unused = RemoveUnusedVertices(*this);
        PPE_SLOG(MeshBuilder, Info, "remove unused vertices", {
            {"Removed", unused},
            {"Total", _vertexCount},
            {"Percent", (unused * 100.f) / _vertexCount}
        });
    }

    pbar.Print("Compute vertex average cache miss rate"_view);
    const float VACMR_Before = VertexAverageCacheMissRate(*this);

    if (settings.OptimizeIndicesOrder) {
        pbar.Print("Optimize mesh indices order"_view);
        OptimizeIndicesOrder(*this);
    }

    if (settings.OptimizeVerticesOrder) {
        pbar.Print("Optimize mesh vertices order"_view);
        OptimizeVerticesOrder(*this);
    }

    pbar.IncPrint("Compute vertex average cache miss rate"_view);
    const float VACMR_After = VertexAverageCacheMissRate(*this);
    PPE_SLOG(MeshBuilder, Info, "optimize vertex average cache miss rate", {
        {"Before", VACMR_Before},
        {"After", VACMR_After},
        {"Percent", VACMR_Before * 100.f / VACMR_After}
    });
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(const TMemoryView<u16>& dst) const {
    return ExportIndices(RHI::EIndexFormat::UShort, dst.Cast<u8>());
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(const TMemoryView<u32>& dst) const {
    return ExportIndices(RHI::EIndexFormat::UInt, dst.Cast<u8>());
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportIndices(RHI::EIndexFormat format, const FRawMemory& dst) const  {
    Assert_NoAssume(Validate());
    Assert_NoAssume(dst.SizeInBytes() == RHI::EIndexFormat_SizeOf(format) * _indexCount);

    switch (format) {
    case RHI::EIndexFormat::UInt:
        FPlatformMemory::Memcpy(dst.data(), Indices().data(), dst.SizeInBytes());
        break;

    case RHI::EIndexFormat::UShort:
    {
        if (_vertexCount > TNumericLimits<u16>::MaxValue())
            return false;

        const u32* psrc = Indices().data();
        u16* pdst = dst.Cast<u16>().data();
        u16* const pend = pdst + _indexCount;

        while (pend != pdst)
            *pdst++ = checked_cast<u16>(*psrc++);

        break;
    }

    default:
        PPE_SLOG(MeshBuilder, Error, "unexpected index format", {
            {"IndexFormat", Opaq::Format(format)},
            {"GenericIndex", Opaq::Format(RHI::EIndexFormat::UInt)},
        });
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
bool FGenericMesh::ExportVertices(const RHI::FVertexInputState& vdecl, const RHI::FVertexBufferID& vertexBuffer, const FRawMemory& dst) const {
    Assert(vertexBuffer.Valid());
    Assert_NoAssume(Validate());

    const RHI::FVertexBufferBinding& bufferBinding = vdecl.BufferBindings[vertexBuffer];
    Assert_NoAssume(dst.SizeInBytes() == bufferBinding.Stride * _vertexCount);

    for (const auto& [vertexId, input] : vdecl.Vertices) {
        if (input.BufferBinding != bufferBinding.Index)
            continue;

        const FGenericVertexData* const pVertexData = VertexDataIFP(vertexId);
        if (Unlikely(not pVertexData)) {
            PPE_SLOG(MeshBuilder, Error, "vertex semantic not found", {
                {"BufferBinding", input.BufferBinding},
                {"VertexID", Opaq::Format(vertexId)},
            });
            return false;
        }

        const size_t srcStride = pVertexData->StrideInBytes();
        Assert_NoAssume(pVertexData->VertexCount() == _vertexCount);
        Assert_NoAssume(pVertexData->SizeInBytes() == srcStride * _vertexCount);

        // same format => trivial copy, always succeed
        if (pVertexData->Format() == input.Format) {
            forrange(v, 0, _vertexCount) {
                const FRawMemoryConst src = pVertexData->Vertex(v);
                src.CopyTo(dst.SubRange(v * bufferBinding.Stride + input.Offset, srcStride));
            }
        }
        // different format => try promotion, can fail
        else if (RHI::FVertexFormatPromote promote = EVertexFormat_Promote(input.Format, pVertexData->Format())) {
            const size_t dstStride = RHI::EVertexFormat_SizeOf(input.Format);
            forrange(v, 0, _vertexCount) {
                const FRawMemoryConst src = pVertexData->Vertex(v);
                PPE_LOG_CHECK(MeshBuilder, promote(dst.SubRange(v * bufferBinding.Stride + input.Offset, dstStride), src));
            }
        }
        else {
            PPE_SLOG(MeshBuilder, Error, "unexpected vertex format", {
                {"VertexInput", Opaq::Format(input.Format)},
                {"GenericVertex", Opaq::Format(pVertexData->Format())},
            });
            return false;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
bool FGenericMesh::Validate() const NOEXCEPT {
    PPE_LOG_CHECK(MeshBuilder, Indices().size() == _indexCount);

    for (const UGenericVertexData& vertices : _vertices.Values()) {
        PPE_LOG_CHECK(MeshBuilder, vertices->VertexCount() == _vertexCount);
        PPE_LOG_CHECK(MeshBuilder, vertices->SizeInBytes() / vertices->StrideInBytes() == _vertexCount);
    }

    for (u32 index : Indices()) {
        PPE_LOG_CHECK(MeshBuilder, index < _vertexCount);
    }

    return true;
}
//----------------------------------------------------------------------------
void FGenericMesh::Clear() {
    ClearIndices();
    ClearVertices();
}
//----------------------------------------------------------------------------
void FGenericMesh::ClearIndices() {
    _indexCount = 0;
    _indices.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
void FGenericMesh::ClearVertices() {
    _vertexCount = 0;
    _vertices.clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FGenericVertexData::FGenericVertexData(FGenericMesh& owner, const RHI::FVertexID& semantic, RHI::EVertexFormat format) NOEXCEPT
:   _owner(owner), _semantic(semantic), _format(format), _vertexCount(0) {
    Assert(_format != Default);
}
//----------------------------------------------------------------------------
FGenericVertexData::~FGenericVertexData() = default;
//----------------------------------------------------------------------------
size_t FGenericVertexData::StrideInBytes() const NOEXCEPT {
    return RHI::EVertexFormat_SizeOf(_format);
}
//----------------------------------------------------------------------------
void FGenericVertexData::Resize(size_t count, bool keepData/* = true */) {
    if (count != _vertexCount) {
        _vertexCount = count;
        _stream.resize(StrideInBytes() * count, keepData);
    }
}
//----------------------------------------------------------------------------
void FGenericVertexData::Reserve(size_t count) {
    if (count > _vertexCount)
        _stream.reserve(StrideInBytes() * count);
}
//----------------------------------------------------------------------------
void FGenericVertexData::CopyVertex(size_t dst, size_t src) {
    Assert(dst < _vertexCount);
    Assert(src < _vertexCount);

    if (dst == src)
        return;

    const size_t strideInBytes = StrideInBytes();
    const FRawMemory storage = _stream.MakeView();

    FPlatformMemory::MemcpyLarge(&storage[dst * strideInBytes], &storage[src * strideInBytes], strideInBytes);
}
//----------------------------------------------------------------------------
void FGenericVertexData::ReadVertex(size_t v, const FRawMemory& dst) const {
    Vertex(v).CopyTo(dst);
}
//----------------------------------------------------------------------------
void FGenericVertexData::WriteVertex(size_t v, const FRawMemoryConst& src) {
    src.CopyTo(Vertex(v));
}
//----------------------------------------------------------------------------
FRawMemory FGenericVertexData::SubRange(size_t start, size_t count) {
    Assert(start + count <= _vertexCount);

    const size_t strideInBytes = StrideInBytes();
    return _stream.MakeView().SubRange(start * strideInBytes, count * strideInBytes);
}
//----------------------------------------------------------------------------
FRawMemoryConst FGenericVertexData::SubRange(size_t start, size_t count) const {
    Assert(start + count <= _vertexCount);

    const size_t strideInBytes = StrideInBytes();
    return _stream.MakeView().SubRange(start * strideInBytes, count * strideInBytes);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#define GENERICVERTEX_SEMANTIC_DEF(_NAME, _DIM, _TYPE) \
    TGenericVertexSubPart< _TYPE > FGenericMesh::CONCAT(CONCAT(_NAME, _DIM), f)(size_t index) { \
        return TGenericVertexSubPart< _TYPE >( GetOrAddVertexData(VertexID(RHI::EVertexID::_NAME, index), TGenericVertexSubPart< _TYPE >::Format) ); \
    } \
    TGenericVertexSubPart< _TYPE > FGenericMesh::CONCAT(CONCAT(_NAME, _DIM), f_IFP)(size_t index) const { \
        return TGenericVertexSubPart< _TYPE >( VertexDataIFP(VertexID(RHI::EVertexID::_NAME, index), TGenericVertexSubPart< _TYPE >::Format) ); \
    }
//----------------------------------------------------------------------------
GENERICVERTEX_SEMANTIC_DEF(Position,    3, float3)
GENERICVERTEX_SEMANTIC_DEF(Position,    4, float4)
GENERICVERTEX_SEMANTIC_DEF(Texcoord,    2, float2)
GENERICVERTEX_SEMANTIC_DEF(Texcoord,    3, float3)
GENERICVERTEX_SEMANTIC_DEF(Texcoord,    4, float4)
GENERICVERTEX_SEMANTIC_DEF(Color,       4, float4)
GENERICVERTEX_SEMANTIC_DEF(Normal,      3, float3)
GENERICVERTEX_SEMANTIC_DEF(Normal,      4, float4)
GENERICVERTEX_SEMANTIC_DEF(Tangent,     3, float3)
GENERICVERTEX_SEMANTIC_DEF(Tangent,     4, float4)
GENERICVERTEX_SEMANTIC_DEF(Binormal,    3, float3)
//----------------------------------------------------------------------------
#undef GENERICVERTEX_SEMANTIC_DEF
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
