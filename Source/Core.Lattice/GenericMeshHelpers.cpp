#include "stdafx.h"

#include "GenericMeshHelpers.h"

#include "GenericMesh.h"

#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Container/Hash.h"
#include "Core/Container/Vector.h"
#include "Core/Maths/PNTriangle.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"
#include "Core/Memory/UniqueView.h"

#include <algorithm>

namespace Core {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// http://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/
// https://github.com/rygorous/simple-ib-compress/blob/master/main.cpp
struct VertexCache {
    static const size_t Size = 32;
    static const size_t MaxValence = 15;

    struct Entry {
        u32 CachePos;           // its position in the cache (UINT32_MAX if not in)
        u32 Score;              // its score (higher=better)
        u32 TrianglesLeft;      // # of not-yet-used tris
        u32 *TriangleList;      // list of triangle indices
        u32 OpenPos;            // position in "open vertex" list
    };

    struct Triangle {
        u32 Score;              // current score (UINT32_MAX if already done)
        u32 Indices[3];         // vertex indices
    };
};
//----------------------------------------------------------------------------
static float3 ComputeNormal_(const float3& p0, const float3& p1, const float3& p2) {
    const float d01 = Length3(p1 - p0);
    const float d12 = Length3(p2 - p1);
    const float d20 = Length3(p0 - p2);

    const float3 c012 = Cross((p1 - p0)/d01,  (p2  - p0)/d20);
    const float3 c102 = Cross((p0 - p1)/d01,  (p2  - p1)/d12);
    const float3 c201 = Cross((p0 - p2)/d20,  (p1  - p2)/d12);

    const float d012 = LengthSq3(c012);
    const float d102 = LengthSq3(c102);
    const float d201 = LengthSq3(c201);

    return (d012 > d102)
        ? (d012 > d201 ? Normalize3(c012) : Normalize3(c201))
        : (d102 > d201 ? Normalize3(c102) : Normalize3(c201));
}
//----------------------------------------------------------------------------
static void ComputeTriangleBasis_(
    float3 *pTangent,
    float3 *pBinormal,
    const float3& pos0, const float3& pos1, const float3& pos2,
    const float2& tex0, const float2& tex1, const float2& tex2 ) {
    // triangle 0, 1, 2
    const float3 pos10 = pos1 - pos0;
    const float3 pos20 = pos2 - pos0;

    const float2 tex10 = tex1 - tex0;
    const float2 tex20 = tex2 - tex0;

    const float f012 = (tex10.x() * tex20.y() -
                        tex10.y() * tex20.x() );

    // triangle 1, 0, 2
    const float3 pos01 = pos0 - pos1;
    const float3 pos21 = pos2 - pos1;

    const float2 tex01 = tex0 - tex1;
    const float2 tex21 = tex2 - tex1;

    const float f102 = (tex01.x() * tex21.y() -
                        tex01.y() * tex21.x() );

    // triangle 2, 0, 1
    const float3 pos02 = pos0 - pos2;
    const float3 pos12 = pos1 - pos2;

    const float2 tex02 = tex0 - tex2;
    const float2 tex12 = tex1 - tex2;

    const float f201 = (tex02.x() * tex12.y() -
                        tex02.y() * tex12.x() );

    if (fabs(f012) > fabs(f102)) {
        if (fabs(f012) > fabs(f201)) {
            const float f = (fabs(f012) < F_EpsilonSQ) ? 1.0f : 1.0f/f012;
            *pTangent = ((pos10 * tex20.y()) - (pos20 * tex10.y())) * f;
            *pBinormal = ((pos20 * tex10.x()) - (pos10 * tex20.x())) * f;
        }
        else {
            const float f = (fabs(f201) < F_EpsilonSQ) ? 1.0f : 1.0f/f201;
            *pTangent = ((pos01 * tex21.y()) - (pos21 * tex01.y())) * f;
            *pBinormal = ((pos21 * tex01.x()) - (pos01 * tex21.x())) * f;
        }
    }
    else {
        if (fabs(f102) > fabs(f201)) {
            const float f = (fabs(f102) < F_EpsilonSQ) ? 1.0f : 1.0f/f102;
            *pTangent = ((pos01 * tex21.y()) - (pos21 * tex01.y())) * f;
            *pBinormal = ((pos21 * tex01.x()) - (pos01 * tex21.x())) * f;
        }
        else {
            const float f = (fabs(f201) < F_EpsilonSQ) ? 1.0f : 1.0f/f201;
            *pTangent = ((pos02 * tex12.y()) - (pos12 * tex02.y())) * f;
            *pBinormal = ((pos12 * tex02.x()) - (pos02 * tex12.x())) * f;
        }
    }
}
//----------------------------------------------------------------------------
}//!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void ComputeNormals_(
    const GenericMesh& mesh,
    const Positions3f& sp_position3f,
    const Positions4f& sp_position4f,
    const Normals3f& sp_normal3f ) {
    Assert((!sp_position3f) ^ (!sp_position4f));

    const size_t indexCount = mesh.IndexCount();
    const size_t vertexCount = mesh.VertexCount();

    Assert(!sp_position3f || sp_position3f.size() == vertexCount);
    Assert(!sp_position4f || sp_position4f.size() == vertexCount);

    const MemoryView<const u32> indices = mesh.Indices();
    const MemoryView<float3> normals3f = sp_normal3f.Resize(vertexCount);

    const float3 zero = float3::Zero();
    for (float3& n : normals3f)
        n = zero;

    if (sp_position4f) {
        Assert(!sp_position3f);
        const MemoryView<const float4> positions4f = sp_position4f.MakeView();
        Assert(positions4f.size() == mesh.VertexCount());

        for (size_t t = 0; t < indexCount; t += 3) {
            const size_t i0 = indices[t + 0];
            const size_t i1 = indices[t + 1];
            const size_t i2 = indices[t + 2];

            const float3 n = ComputeNormal_(
                positions4f[i0].xyz(),
                positions4f[i1].xyz(),
                positions4f[i2].xyz() );

            normals3f[i0] += n;
            normals3f[i1] += n;
            normals3f[i2] += n;
        }
    }
    else {
        Assert(!sp_position4f);
        const MemoryView<const float3> positions3f = sp_position3f.MakeView();

        for (size_t t = 0; t < indexCount; t += 3) {
            const size_t i0 = indices[t + 0];
            const size_t i1 = indices[t + 1];
            const size_t i2 = indices[t + 2];

            const float3 n = ComputeNormal_(
                positions3f[i0],
                positions3f[i1],
                positions3f[i2] );

            normals3f[i0] += n;
            normals3f[i1] += n;
            normals3f[i2] += n;
        }
    }

    for (float3& n : normals3f)
        n = Normalize3(n);
}
} //!namespace
//----------------------------------------------------------------------------
bool ComputeNormals(GenericMesh& mesh, size_t index) {
    const GenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index);
    const GenericVertexSubPart<float4> sp_position4f = mesh.Position4f_IFP(index);
    if (!sp_position3f && !sp_position4f)
        return false;

    GenericVertexSubPart<float3> sp_normal3f = mesh.Normal3f(index);
    Assert(sp_normal3f);

    ComputeNormals_(mesh, sp_position3f, sp_position4f, sp_normal3f);

    return true;
}
//----------------------------------------------------------------------------
void ComputeNormals(const GenericMesh& mesh, const Positions3f& positions, const Normals3f& normals) {
    Assert(positions);
    Assert(normals);

    ComputeNormals_(mesh, positions, Positions4f(), normals);
}
//----------------------------------------------------------------------------
void ComputeNormals(const GenericMesh& mesh, const Positions4f& positions, const Normals3f& normals) {
    Assert(positions);
    Assert(normals);

    ComputeNormals_(mesh, Positions3f(), positions, normals);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
static void ComputeTangentSpace_(
    const GenericMesh& mesh,
    const Positions3f& sp_position3f,
    const Positions4f& sp_position4f,
    const TexCoords2f& sp_texcoord2f,
    const Normals3f& sp_normal3f,
    const Tangents3f& sp_tangent3f,
    const Tangents4f& sp_tangent4f,
    const Binormals3f& sp_binormal3f ) {
    Assert((!sp_position3f) ^ (!sp_position4f));
    Assert((!sp_tangent4f) ^ (!sp_tangent3f));

    const size_t indexCount = mesh.IndexCount();
    const size_t vertexCount = mesh.VertexCount();

    Assert(!sp_position3f || sp_position3f.size() == vertexCount);
    Assert(!sp_position4f || sp_position4f.size() == vertexCount);
    Assert(sp_texcoord2f.size() == vertexCount);
    Assert(sp_normal3f.size() == vertexCount);

    const MemoryView<const u32> indices = mesh.Indices();

    const MemoryView<const float3> positions3f = sp_position3f.MakeView();
    const MemoryView<const float4> positions4f = sp_position4f.MakeView();
    const MemoryView<const float2> texcoords2f = sp_texcoord2f.MakeView();
    const MemoryView<const float3> normals3f = sp_normal3f.MakeView();

    VECTOR_THREAD_LOCAL(GenericMesh, float3) tmpTangents3f;
    VECTOR_THREAD_LOCAL(GenericMesh, float3) tmpBinormals3f;

    MemoryView<float3> tangents3f;
    MemoryView<float4> tangents4f;
    MemoryView<float3> binormals3f;
    if (sp_tangent4f) {
        tmpTangents3f.resize_AssumeEmpty(vertexCount);
        tmpBinormals3f.reserve_AssumeEmpty(vertexCount);

        tangents3f = tmpTangents3f.MakeView();
        binormals3f = tmpBinormals3f.MakeView();

        tangents4f = sp_tangent4f.Resize(vertexCount);
    }
    else {
        tangents3f = sp_tangent3f.Resize(vertexCount);
        binormals3f = sp_binormal3f.Resize(vertexCount);
    }

    const float3 zero = float3::Zero();
    for (size_t i = 0; i < vertexCount; ++i)
        tangents3f[i] = binormals3f[i] = zero;

    for (size_t i = 0; i < indexCount; i += 3) {
        const size_t i0 = indices[i + 0];
        const size_t i1 = indices[i + 1];
        const size_t i2 = indices[i + 2];

        float3 tangent;
        float3 binormal;

        if (sp_position3f) {
            ComputeTriangleBasis_(
                &tangent, &binormal,
                positions3f[i0], positions3f[i1], positions3f[i2],
                texcoords2f[i0], texcoords2f[i1], texcoords2f[i2] );
        }
        else {
            ComputeTriangleBasis_(
                &tangent, &binormal,
                positions4f[i0].xyz(), positions4f[i1].xyz(), positions4f[i2].xyz(),
                texcoords2f[i0], texcoords2f[i1], texcoords2f[i2] );
        }

        tangents3f[i0] += tangent;
        tangents3f[i1] += tangent;
        tangents3f[i2] += tangent;

        binormals3f[i0] += binormal;
        binormals3f[i1] += binormal;
        binormals3f[i2] += binormal;
    }

    for (size_t i = 0; i < vertexCount; ++i) {
        float3& tangent = tangents3f[i];
        float3& binormal = binormals3f[i];

        const float3& normal = normals3f[i];
        if (float3(0) == tangent && float3(0) == binormal) {
            tangent = float3(1, 0, 0);
            binormal = Cross(normal, tangent);
        }
        else if (float3(0) == tangent) {
            Assert(float3(0) != binormal);
            binormal = Normalize3(binormal);
            tangent = Cross(normal, binormal);
        }
        else if (float3(0) == binormal) {
            Assert(float3(0) != tangent);
            tangent = Normalize3(tangent);
            binormal = Cross(normal, tangent);
        }
        else {
            tangent = Normalize3(tangent);
            binormal = Normalize3(binormal);
        }

        //Gram-Schmidt orthogonalization
        tangent = Normalize3(tangent - normal * Dot3(normal, tangent));

        //Right handed TBN space ?
        const bool leftHanded = (Dot3(Cross(tangent, binormal), normal) < 0);

        //Handedness packing IFP
        if (leftHanded) {
            binormal = -binormal;

            if (sp_tangent4f)
                tangents4f[i] = float4(tangent, leftHanded ? 1.0f : 0.0f);
        }
    }
}
} //!namespace
//----------------------------------------------------------------------------
bool ComputeTangentSpace(GenericMesh& mesh, size_t index, bool packHandedness/* = false */) {
    const GenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index);
    const GenericVertexSubPart<float4> sp_position4f = mesh.Position4f_IFP(index);
    if (!sp_position3f && !sp_position4f)
        return false;

    Assert(!sp_position3f ^ !sp_position4f);

    const GenericVertexSubPart<float2> sp_texcoord2f = mesh.TexCoord2f_IFP(index);
    if (!sp_texcoord2f)
        return false;

    const GenericVertexSubPart<float3> sp_normal3f = mesh.Normal3f_IFP(index);
    if (!sp_normal3f)
        return false;

    GenericVertexSubPart<float3> sp_tangent3f;
    GenericVertexSubPart<float4> sp_tangent4f;
    GenericVertexSubPart<float3> sp_binormal3f;

    if (packHandedness) {
        sp_tangent4f = mesh.Tangent4f(index);

        Assert(sp_tangent4f);
    }
    else {
        sp_tangent3f = mesh.Tangent3f(index);
        sp_binormal3f = mesh.Binormal3f(index);

        Assert(sp_tangent3f);
        Assert(sp_binormal3f);
    }

    ComputeTangentSpace_(mesh, sp_position3f, sp_position4f, sp_texcoord2f, sp_normal3f, sp_tangent3f, sp_tangent4f, sp_binormal3f);

    return true;
}
//----------------------------------------------------------------------------
void ComputeTangentSpace(const GenericMesh& mesh, const Positions3f& positions, const TexCoords2f& uv, const Normals3f& normals, const Tangents3f& tangents, const Binormals3f& binormals) {
    ComputeTangentSpace_(mesh, positions, Positions4f(), uv, normals, tangents, Tangents4f(), binormals);
}
//----------------------------------------------------------------------------
void ComputeTangentSpace(const GenericMesh& mesh, const Positions4f& positions, const TexCoords2f& uv, const Normals3f& normals, const Tangents3f& tangents, const Binormals3f& binormals) {
    ComputeTangentSpace_(mesh, Positions3f(), positions, uv, normals, tangents, Tangents4f(), binormals);
}
//----------------------------------------------------------------------------
void ComputeTangentSpace(const GenericMesh& mesh, const Positions3f& positions, const TexCoords2f& uv, const Normals3f& normals, const Tangents4f& tangents) {
    ComputeTangentSpace_(mesh, positions, Positions4f(), uv, normals, Tangents3f(), tangents, Binormals3f());
}
//----------------------------------------------------------------------------
void ComputeTangentSpace(const GenericMesh& mesh, const Positions4f& positions, const TexCoords2f& uv, const Normals3f& normals, const Tangents4f& tangents) {
    ComputeTangentSpace_(mesh, Positions3f(), positions, uv, normals, Tangents3f(), tangents, Binormals3f());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool MergeDuplicateVertices(GenericMesh& mesh) {
    const size_t vertexCount = mesh.VertexCount();
    if (0 == vertexCount)
        return false;

    Assert(vertexCount > 1);

    STACKLOCAL_POD_ARRAY(hash_t, hashes, vertexCount);
    STACKLOCAL_POD_ARRAY(u32, tmp, vertexCount*2);
    const MemoryView<u32> sorteds = tmp.SubRange(0*vertexCount, vertexCount);
    const MemoryView<u32> reindexation = tmp.SubRange(1*vertexCount, vertexCount);

    // Init vertex hash value and sort index
    forrange(v, 0, vertexCount) {
        sorteds[v] = checked_cast<u32>(v);
        hashes[v] = CORE_HASH_VALUE_SEED;
    }

    // Compute each vertex hash value subpart by supart
    for (const GenericVertexData& subpart : mesh.Vertices()) {
        Assert(subpart.VertexCount() == vertexCount);

        const MemoryView<const u8> rawData = subpart.MakeView();
        const Graphics::ValueType type = subpart.Type();
        const size_t strideInBytes = subpart.StrideInBytes();

        forrange(v, 0, vertexCount) {
            const auto block = rawData.SubRange(v * strideInBytes, strideInBytes);
            hash_combine(hashes[v], Graphics::ValueHash(type, block));
        }
    }

    // Sort by hash value and by index
    std::sort(sorteds.begin(), sorteds.end(), [&hashes](u32 lhs, u32 rhs) {
        return (hashes[lhs] == hashes[rhs]
            ? lhs < rhs // preserve vertex order when duplicated (we want the first occurence)
            : hashes[lhs] < hashes[rhs] );
    });

    // Search for equal hash values <=> duplicates and remap vertices
    u32 finalCount = 1;
    u32 v0 = sorteds[0];
    reindexation[v0] = v0;
    forrange(i, 1, vertexCount) {
        const u32 v1 = sorteds[i];

        if (mesh.AreVertexEquals(v0, v1)) {
            reindexation[v1] =  reindexation[v0];
        }
        else {
            reindexation[v1] =  v1;
            finalCount++;
        }

        v0 = v1;
    }

    // No collisions <=> no duplicates
    if (finalCount == vertexCount)
        return false;
    Assert(finalCount < vertexCount);

    // Remap all indices to new merged indices
    for (u32& index : mesh.Indices())
        index = reindexation[index];

    // Sort the reindexation to get the new vertices order
    std::sort(reindexation.begin(), reindexation.end());
    Assert(0 == reindexation[0]); // the first vertex can't be replaced : it's always the first occurence

    // Compact reindexation by removing duplicates
    {
        u32 n = 1;
        forrange(v, 1, vertexCount) {
            if (reindexation[v] != reindexation[v-1])
                reindexation[n++] = reindexation[v];
        }
        Assert(n == finalCount);
    }

    // Compact each subpart data with the reindexation
    for (GenericVertexData& subpart : mesh.Vertices()) {
        forrange(i, 1, finalCount) {
            if (i != reindexation[i]) {
                Assert(i < reindexation[i]);
                subpart.CopyVertex(i, reindexation[i]);
            }
        }
    }

    mesh.Resize(mesh.Indices().size(), vertexCount, true);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
// PN-Triangles tesselation :
static void PNTesselateRecursive_(
    u32* pBaseIndex,
    u32* pBaseVertex,
    const MemoryView<u32>& indices,
    const MemoryView<float3>& vertices,
    const float3& uvw00, u32 i00,
    const float3& uvw11, u32 i11,
    const float3& uvw22, u32 i22,
    size_t n ) {
    if (n) {
        n--;

        const u32 j01 = *pBaseVertex++;
        const u32 j12 = *pBaseVertex++;
        const u32 j20 = *pBaseVertex++;

        const float3 uvw01 = (uvw00 + uvw11) * 0.5f;
        const float3 uvw12 = (uvw11 + uvw22) * 0.5f;
        const float3 uvw20 = (uvw22 + uvw00) * 0.5f;

        PNTesselateRecursive_(pBaseIndex, pBaseVertex, indices, vertices, uvw00,i00, vertices[j01],j01, vertices[j20],j20, n);
        PNTesselateRecursive_(pBaseIndex, pBaseVertex, indices, vertices, vertices[j01],j01, uvw11,i11, vertices[j12],j12, n);
        PNTesselateRecursive_(pBaseIndex, pBaseVertex, indices, vertices, vertices[j12],j12, vertices[j20],j20, vertices[j01],j01, n);
        PNTesselateRecursive_(pBaseIndex, pBaseVertex, indices, vertices, vertices[j20],j20, vertices[j12],j12, uvw22,i22, n);
    }
    else {
        indices[*pBaseIndex++] = i00;
        indices[*pBaseIndex++] = i11;
        indices[*pBaseIndex++] = i22;
    }
}
//----------------------------------------------------------------------------
// The generated indices order will be very inefficient, better call OptimizeIndices() afterwards
static void PNTriangles_(
    GenericMesh& mesh,
    const Positions3f& sp_position3f,
    const Normals3f& sp_normal3f,
    size_t recursions ) {
    if (0 == recursions)
        return;

    FixedSizeStack<GenericVertexData*, 6> sp_lerps;
    for (GenericVertexData& vertices : mesh.Vertices()) {
        if (&vertices != sp_position3f.Data() &&
            &vertices != sp_normal3f.Data() ) {
            sp_lerps.Push(&vertices);
        }
    }

    const u32 baseIndex = checked_cast<u32>(mesh.IndexCount());
    const u32 baseVertex = checked_cast<u32>(mesh.VertexCount());
    Assert(0 == (baseIndex % 3));

    // Space allocated per triangle perfect quadtree, geometric series : (4^depth - 1)/3)
    const size_t triangleCount = (baseIndex / 3);
    const size_t indicesPerTriangle = Pow(4, recursions) * 3;
    const size_t verticesPerTriangle = (indicesPerTriangle / 3 - 1);

    STACKLOCAL_POD_ARRAY(float3, barycentrics, verticesPerTriangle);
    STACKLOCAL_POD_ARRAY(u32, baseIndices, baseIndex);
    Copy(baseIndices, mesh.Indices());

    const size_t nextIndexCount = indicesPerTriangle * triangleCount;
    const size_t nextVertexCount = baseVertex + verticesPerTriangle * triangleCount;
    mesh.Resize(nextIndexCount, nextVertexCount);

    const MemoryView<float3> positions3f = sp_position3f.MakeView();
    const MemoryView<float3> normals3f = sp_normal3f.MakeView();
    Assert(positions3f.size() == nextVertexCount);
    Assert(normals3f.size() == nextVertexCount);

    PNTriangle pn;

    u32 nextIndex = 0;
    u32 nextVertex = baseVertex;

    for (size_t i = 0; i < baseIndex; i += 3) {
        const u32 i0 = baseIndices[i + 0];
        const u32 i1 = baseIndices[i + 1];
        const u32 i2 = baseIndices[i + 2];

        const u32 startIndex = nextIndex;
        const u32 startVertex = nextVertex;

        PNTesselateRecursive_(
            &nextIndex,
            &nextVertex,
            mesh.Indices(),
            barycentrics,
            float3(1,0,0), i0,
            float3(0,1,0), i1,
            float3(0,0,1), i2,
            recursions );

        PNTriangle::FromTriangle(pn,
            positions3f[i0], normals3f[i0],
            positions3f[i1], normals3f[i1],
            positions3f[i2], normals3f[i2] );

        Assert(indicesPerTriangle == (nextIndex - startIndex));
        Assert(verticesPerTriangle == (nextVertex - startVertex));

        forrange(j, startVertex, nextVertex)
            positions3f[j] = pn.LerpPosition(barycentrics[j]);

        forrange(j, startVertex, nextVertex)
            normals3f[j] = pn.LerpNormal(barycentrics[j]);

        for (GenericVertexData* pData : sp_lerps) {
            Graphics::ValueBarycentricLerpArray(
                pData->Type(),
                pData->SubRange(startVertex, verticesPerTriangle),
                pData->StrideInBytes(),
                pData->VertexView(i0),
                pData->VertexView(i1),
                pData->VertexView(i2),
                barycentrics );
        }

        Assert(nextIndex == (i/3) * indicesPerTriangle);
        Assert(nextVertex == baseVertex + (i/3) * verticesPerTriangle);
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
bool PNTriangles(GenericMesh& mesh, size_t index, size_t recursions) {
    const GenericVertexSubPart<float3> sp_position3f = mesh.Position3f_IFP(index);
    if (!sp_position3f)
        return false;

    const GenericVertexSubPart<float3> sp_normal3f = mesh.Normal3f_IFP(index);
    if (!sp_normal3f)
        return false;

    // You have to generate those after tesselation,
    // or these will linearly interpolated (instead of quadratrically, like normals3f)
    // and you would get a wrong TBN basis.
    // Better call ComputeTangentSpace() after this pass.
    Assert(!mesh.Tangent3f_IFP(index));
    Assert(!mesh.Tangent4f_IFP(index));
    Assert(!mesh.Binormal3f_IFP(index));

    PNTriangles_(mesh, sp_position3f, sp_normal3f, recursions);

    return true;
}
//----------------------------------------------------------------------------
void PNTriangles(GenericMesh& mesh, const Positions3f& positions, const Normals3f& normals, size_t recursions) {
    PNTriangles_(mesh, positions, normals, recursions);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void OptimizeIndicesOrder(const MemoryView<u32>& indices, size_t vertexCount) {
    Assert(indices.size() >= 3);
    Assert(0 == (indices.size() % 3));

    // prepare triangles
    STACKLOCAL_POD_ARRAY(VertexCache::Entry, entries, vertexCount);
    for (VertexCache::Entry& entry : entries) {
        entry.CachePos = UINT32_MAX;
        entry.Score = 0;
        entry.TrianglesLeft = 0;
        entry.TriangleList = nullptr;
        entry.OpenPos = UINT32_MAX;
    }

    // alloc space for entry triangle indices
    const size_t triangleCount = indices.size() / 3;
    STACKLOCAL_POD_ARRAY(VertexCache::Triangle, triangles, triangleCount);
    {
        const u32 *pIndex = &indices[0];
        for (size_t i = 0; i < triangleCount; ++i) {
            VertexCache::Triangle& triangle = triangles[i];
            triangle.Score = 0;
            for (size_t j = 0; j < 3; ++j) {
                const u32 index = *pIndex++;
                triangle.Indices[j] = index;
                ++entries[index].TrianglesLeft;
            }
        }
    }

    STACKLOCAL_POD_ARRAY(u32, adjacencies, triangleCount*3);
    {
        u32 *pTriList = adjacencies.Pointer();
        const u32 *pTriListEnd = adjacencies.Pointer() + adjacencies.size();

        for (size_t i = 0; i < vertexCount; ++i) {
            VertexCache::Entry& entry = entries[i];
            entry.TriangleList = pTriList;
            pTriList += entry.TrianglesLeft;
            Assert(pTriList <= pTriListEnd);
            entry.TrianglesLeft = 0;
        }

        for (size_t i = 0; i < triangleCount; ++i) {
            const VertexCache::Triangle& triangle = triangles[i];
            for (size_t j = 0; j < 3; ++j) {
                const u32 index = triangle.Indices[j];
                VertexCache::Entry& entry = entries[index];
                entry.TriangleList[entry.TrianglesLeft++] = u32(i);
            }
        }
    }

    // open vertices
    STACKLOCAL_POD_ARRAY(u32, openVertices, vertexCount);
    size_t openCount = 0;

    // the cache
    u32 cache[VertexCache::Size + 3] = {UINT32_MAX};
    u32 pos2score[VertexCache::Size];
    u32 val2score[VertexCache::Size + 1];

    for (size_t i = 0; i < VertexCache::Size; ++i) {
        const float score = (i < 3) ? 0.75f : std::pow(1.0f - (i - 3)/float(VertexCache::Size - 3), 1.5f);
        pos2score[i] = static_cast<u32>(score * 65536.0f + 0.5f);
    }

    val2score[0] = 0;
    for(int i=1;i<16;i++)
    {
        const float score = 2.0f / std::sqrt(float(i));
        val2score[i] = static_cast<u32>(score * 65536.0f + 0.5f);
    }

    // outer loop: find triangle to start with
    u32 *pIndex = &indices[0];
    u32 seedPos = 0;

    for (;;) {
        u32 seedScore = 0;
        u32 seedTriangle = UINT32_MAX;

        // if there are open vertices, search them for the seed triangle
        // which maximum score.
        for (size_t i = 0; i < openCount; ++i) {
            const VertexCache::Entry &entry = entries[openVertices[i]];

            for (size_t j = 0; j < entry.TrianglesLeft; ++j) {
                const u32 index = entry.TriangleList[j];
                const VertexCache::Triangle& triangle = triangles[index];

                if (triangle.Score > seedScore) {
                    seedScore = triangle.Score;
                    seedTriangle = index;
                }
            }
        }

        // if we haven't found a seed triangle yet, there are no open
        // vertices and we can pick any triangle
        if (UINT32_MAX == seedTriangle) {
            while (seedPos < triangleCount && triangles[seedPos].Score == UINT32_MAX)
                ++seedPos;

            if (seedPos == triangleCount) // no triangle left, we're done!
                break;

            seedTriangle = seedPos;
        }

        u32 bestTriangle = seedTriangle;
        while (bestTriangle != UINT32_MAX) {
            VertexCache::Triangle& triangle = triangles[bestTriangle];

            // mark this triangle as used, remove it from the "remaining tris"
            // list of the vertices it uses, and add it to the index buffer.
            triangle.Score = UINT32_MAX;

            for (size_t j = 0; j < 3; ++j) {
                const u32 index = triangle.Indices[j];
                *pIndex++ = index;

                VertexCache::Entry& entry = entries[index];

                // find this triangles' entry
                u32 k = 0;
                while(entry.TriangleList[k] != bestTriangle) {
                    Assert(k < entry.TrianglesLeft);
                    ++k;
                }

                // swap it to the end and decrement # of tris left
                if(--entry.TrianglesLeft) {
                    std::swap(entry.TriangleList[k], entry.TriangleList[entry.TrianglesLeft]);
                }
                else if(entry.OpenPos != UINT32_MAX) {
                    Assert(openCount);
                    std::swap(openVertices[entry.OpenPos], openVertices[--openCount]);
                }
            }

            // update cache status
            cache[VertexCache::Size] = cache[VertexCache::Size + 1] = cache[VertexCache::Size + 2] = UINT32_MAX;

            for(size_t j = 0; j < 3 ; ++j) {
                const u32 index = triangle.Indices[j];
                cache[VertexCache::Size + 2] = index;

                // find vertex index
                u32 pos = 0;
                while (index != cache[pos])
                    ++pos;

                // move to front
                for(int k = pos; k > 0; k--)
                    cache[k] = cache[k - 1];

                cache[0] = index;

                // remove sentinel if it wasn't used
                if(pos != VertexCache::Size + 2)
                    cache[VertexCache::Size + 2] = UINT32_MAX;
            }

            // update vertex scores
            for (size_t i = 0; i < VertexCache::Size + 3; ++i) {
                const u32 index = cache[i];
                if (UINT32_MAX == index)
                    continue;

                VertexCache::Entry& entry = entries[index];

                entry.Score = val2score[std::min(entry.TrianglesLeft, u32(VertexCache::MaxValence))];
                if (i < VertexCache::Size) {
                    entry.CachePos = checked_cast<u32>(i);
                    entry.Score += pos2score[i];
                }
                else {
                    entry.CachePos = UINT32_MAX;

                    // also add to open vertices list if the vertex is indeed open
                    if (UINT32_MAX == entry.OpenPos && entry.TrianglesLeft) {
                        entry.OpenPos = checked_cast<u32>(openCount);
                        openVertices[openCount++] = index;
                    }
                }
            }

            // update triangle scores, find new best triangle
            u32 bestScore = 0;
            bestTriangle = UINT32_MAX;

            for (size_t i = 0; i < VertexCache::Size; ++i) {
                if (cache[i] == UINT32_MAX)
                    continue;

                const VertexCache::Entry& entry = entries[cache[i]];

                for (size_t j = 0; j < entry.TrianglesLeft; ++j) {
                    const u32 index = entry.TriangleList[j];
                    VertexCache::Triangle& tri = triangles[index];

                    Assert(tri.Score != UINT32_MAX);

                    u32 score = 0;
                    for (size_t k = 0; k < 3; ++k)
                        score += entries[tri.Indices[k]].Score;

                    tri.Score = score;
                    if (score > bestScore) {
                        bestScore = score;
                        bestTriangle = index;
                    }
                }
            }
        } //!while (bestTriangle != UINT32_MAX)
    } //!for (;;)
}
//----------------------------------------------------------------------------
void OptimizeIndicesOrder(GenericMesh& mesh) {
    OptimizeIndicesOrder(mesh.Indices(), mesh.VertexCount());
}
//----------------------------------------------------------------------------
void OptimizeVerticesOrder(GenericMesh& mesh) { // also removes unused vertices as a side effect
    const size_t vertexCount = mesh.VertexCount();

    STACKLOCAL_POD_ARRAY(u32, tmpData, vertexCount*3);
    const MemoryView<u32> old2new = tmpData.SubRange(0, vertexCount);
    const MemoryView<u32> reindexation = tmpData.SubRange(vertexCount, vertexCount);
    const MemoryView<u32> tmp = tmpData.SubRange(2*vertexCount, vertexCount);
    memset(old2new.Pointer(), 0xFF, old2new.SizeInBytes());

    u32 sortedCount = 0;

    // Construct mapping and new VB
    for (u32& index : mesh.Indices()) {
        if (vertexCount <= old2new[index]) {
            reindexation[sortedCount] = index;
            old2new[index] = sortedCount++;
        }
        index = old2new[index];
    }

    // Handle unused vertices
    if (sortedCount < vertexCount) {
        u32 n = sortedCount;
        for (u32& index : old2new) {
            if (vertexCount <= index)
                index = n++;
        }
    }
    Assert(sortedCount <= vertexCount);

    // Reorder each vertex subpart
    Graphics::Value x;
    for (GenericVertexData& subpart : mesh.Vertices()) {
        Copy(tmp, reindexation);

        forrange(i, 0, vertexCount) {
            subpart.ReadVertex(i, x);

            size_t j = i;
            while (true) {
                const size_t k = tmp[j];
                tmp[j] = u32(j);
                if (k == i)
                    break;

                subpart.CopyVertex(j, k);
                j = k;
            }

            subpart.WriteVertex(j, x);
        }
    }

    // Remove unused vertices
    if (sortedCount < vertexCount)
        mesh.Resize(mesh.IndexCount(), sortedCount);
}
//----------------------------------------------------------------------------
void OptimizeIndicesAndVerticesOrder(GenericMesh& mesh) {
    OptimizeIndicesOrder(mesh);
    OptimizeVerticesOrder(mesh);
}
//----------------------------------------------------------------------------
float VertexAverageCacheMissRate(const MemoryView<const u32>& indices, bool fifo/* = true */, size_t cacheSize/* = 16 */) {
    Assert(cacheSize > 1);

    if (indices.empty())
        return 0.0f;

    const size_t indexCount = indices.size();

    size_t misses = 0;
    size_t wrPos = 0;

    STACKLOCAL_POD_ARRAY(u32, cache, cacheSize+1);

    // initialize cache (we simulate a FIFO here)
    for (size_t i = 0; i < cacheSize; ++i)
        cache[i] = UINT32_MAX;

    // simulate
    for (size_t i = 0; i < indexCount; ++i) {
        const u32 index = indices[i];
        cache[cacheSize] = index;

        // find in cache
        size_t cachePos = 0;
        while (cache[cachePos] != index)
            ++cachePos;

        misses += (cachePos == cacheSize);

        if (fifo) {
            cache[wrPos] = index;
            if (++wrPos == cacheSize)
                wrPos = 0;
        }
        else {
            // move to front
            for (size_t j = cachePos; j > 0; --j)
                cache[j] = cache[j - 1];

            cache[0] = index;
        }
    }

    const float ACMR = misses * 3.0f / indexCount;
    return ACMR;
}
//----------------------------------------------------------------------------
float VertexAverageCacheMissRate(const GenericMesh& mesh, bool fifo/* = true */, size_t cacheSize/* = 16 */) {
    return VertexAverageCacheMissRate(mesh.Indices(), fifo, cacheSize);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace Core
