#include "stdafx.h"

#include "GeometricPrimitives.h"

#include "GenericVertex.h"

#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Container/HashMap.h"
#include "Core/Container/Pair.h"
#include "Core/Maths/Plane.h"

#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarVectorHelpers.h"

namespace Core {
namespace Engine {
namespace GeometricPrimitive {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void Triangle_(Indices& indices, u32 a, u32 b, u32 c) {
    indices.emplace_back(a);
    indices.emplace_back(b);
    indices.emplace_back(c);
}
//----------------------------------------------------------------------------
static void TriangleOffset_(Indices& indices, u32 offset, u32 a, u32 b, u32 c) {
    indices.emplace_back(a + offset);
    indices.emplace_back(b + offset);
    indices.emplace_back(c + offset);
}
//----------------------------------------------------------------------------
struct Edge_ {
    u32 A, B;

    bool operator ==(const Edge_& other) const { return reinterpret_cast<const u64&>(*this) == reinterpret_cast<const u64&>(other); }
    bool operator !=(const Edge_& other) const { return !operator ==(other); }
};
//----------------------------------------------------------------------------
void swap(Edge_& lhs, Edge_& rhs) {
    std::swap(reinterpret_cast<u64&>(lhs), reinterpret_cast<u64&>(rhs));
}
//----------------------------------------------------------------------------
hash_t hash_value(const Edge_& value) {
    return Core::hash_value(reinterpret_cast<const u64&>(value));
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Cube(Indices& indices, Positions& uvw) {
    const u32 offset = checked_cast<u32>(uvw.size());

    uvw.reserve(uvw.size() + 24);

    uvw.emplace_back(-1.0f, 1.0f, -1.0f);
    uvw.emplace_back(1.0f, 1.0f, -1.0f);
    uvw.emplace_back(1.0f, 1.0f, 1.0f);
    uvw.emplace_back(-1.0f, 1.0f, 1.0f);

    uvw.emplace_back(-1.0f, -1.0f, -1.0f);
    uvw.emplace_back(1.0f, -1.0f, -1.0f);
    uvw.emplace_back(1.0f, -1.0f, 1.0f);
    uvw.emplace_back(-1.0f, -1.0f, 1.0f);

    uvw.emplace_back(-1.0f, -1.0f, 1.0f);
    uvw.emplace_back(-1.0f, -1.0f, -1.0f);
    uvw.emplace_back(-1.0f, 1.0f, -1.0f);
    uvw.emplace_back(-1.0f, 1.0f, 1.0f);

    uvw.emplace_back(1.0f, -1.0f, 1.0f);
    uvw.emplace_back(1.0f, -1.0f, -1.0f);
    uvw.emplace_back(1.0f, 1.0f, -1.0f);
    uvw.emplace_back(1.0f, 1.0f, 1.0f);

    uvw.emplace_back(-1.0f, -1.0f, -1.0f);
    uvw.emplace_back(1.0f, -1.0f, -1.0f);
    uvw.emplace_back(1.0f, 1.0f, -1.0f);
    uvw.emplace_back(-1.0f, 1.0f, -1.0f);

    uvw.emplace_back(-1.0f, -1.0f, 1.0f);
    uvw.emplace_back(1.0f, -1.0f, 1.0f);
    uvw.emplace_back(1.0f, 1.0f, 1.0f);
    uvw.emplace_back(-1.0f, 1.0f, 1.0f);

    indices.reserve(indices.size() + 12 * 3);

    TriangleOffset_(indices, offset, 3,1,0);
    TriangleOffset_(indices, offset, 2,1,3);

    TriangleOffset_(indices, offset, 6,4,5);
    TriangleOffset_(indices, offset, 7,4,6);

    TriangleOffset_(indices, offset, 11,9,8);
    TriangleOffset_(indices, offset, 10,9,11);

    TriangleOffset_(indices, offset, 14,12,13);
    TriangleOffset_(indices, offset, 15,12,14);

    TriangleOffset_(indices, offset, 19,17,16);
    TriangleOffset_(indices, offset, 18,17,19);

    TriangleOffset_(indices, offset, 22,20,21);
    TriangleOffset_(indices, offset, 23,20,22);
}
//----------------------------------------------------------------------------
void Pyramid(Indices& indices, Positions& uvw) {
    const u32 offset = checked_cast<u32>(uvw.size());

    uvw.reserve(uvw.size() + 5);

    uvw.emplace_back( 0.0f, 1.0f,  0.0f);  // 0 top
    uvw.emplace_back( 0.0f, 0.0f, -1.0f); // 1 front
    uvw.emplace_back( 1.0f, 0.0f,  0.0f);  // 2 right
    uvw.emplace_back( 0.0f, 0.0f,  1.0f);  // 3 back
    uvw.emplace_back(-1.0f, 0.0f,  0.0f); // 4 left

    indices.reserve(indices.size() + 4 * 3);

    TriangleOffset_(indices, offset, 0, 1, 2); // top front-right face
    TriangleOffset_(indices, offset, 0, 2, 3); // top back-right face
    TriangleOffset_(indices, offset, 0, 3, 4); // top back-left face
    TriangleOffset_(indices, offset, 0, 4, 1); // top front-left face
}
//----------------------------------------------------------------------------
void Octahedron(Indices& indices, Positions& uvw) {
    const u32 offset = checked_cast<u32>(uvw.size());

    uvw.reserve(uvw.size() + 6);

    uvw.emplace_back( 0.0f,  1.0f,  0.0f); // 0 top
    uvw.emplace_back( 0.0f,  0.0f, -1.0f); // 1 front
    uvw.emplace_back( 1.0f,  0.0f,  0.0f); // 2 right
    uvw.emplace_back( 0.0f,  0.0f,  1.0f); // 3 back
    uvw.emplace_back(-1.0f,  0.0f,  0.0f); // 4 left
    uvw.emplace_back( 0.0f, -1.0f,  0.0f); // 5 bottom

    indices.reserve(indices.size() + 8 * 3);

    TriangleOffset_(indices, offset, 0, 1, 2); // top front-right face
    TriangleOffset_(indices, offset, 0, 2, 3); // top back-right face
    TriangleOffset_(indices, offset, 0, 3, 4); // top back-left face
    TriangleOffset_(indices, offset, 0, 4, 1); // top front-left face
    TriangleOffset_(indices, offset, 5, 1, 4); // bottom front-left face
    TriangleOffset_(indices, offset, 5, 4, 3); // bottom back-left face
    TriangleOffset_(indices, offset, 5, 3, 2); // bottom back-right face
    TriangleOffset_(indices, offset, 5, 2, 1); // bottom front-right face
}
//----------------------------------------------------------------------------
void Icosahedron(Indices& indices, Positions& uvw) {
    const u32 offset = checked_cast<u32>(uvw.size());

    uvw.reserve(uvw.size() + 12);

    const float X = 0.525731112119133606f;
    const float Z = 0.850650808352039932f;

    uvw.emplace_back(-X, 0.0f, Z);
    uvw.emplace_back(X, 0.0f, Z);
    uvw.emplace_back(-X, 0.0f, -Z);
    uvw.emplace_back(X, 0.0f, -Z);
    uvw.emplace_back(0.0f, Z, X);
    uvw.emplace_back(0.0f, Z, -X);
    uvw.emplace_back(0.0f, -Z, X);
    uvw.emplace_back(0.0f, -Z, -X);
    uvw.emplace_back(Z, X, 0.0f);
    uvw.emplace_back(-Z, X, 0.0f);
    uvw.emplace_back(Z, -X, 0.0f);
    uvw.emplace_back(-Z, -X, 0.0f);

    indices.reserve(indices.size() + 20 * 3);

    TriangleOffset_(indices, offset, 0, 4, 1);
    TriangleOffset_(indices, offset, 0, 9, 4);
    TriangleOffset_(indices, offset, 9, 5, 4);
    TriangleOffset_(indices, offset, 4, 5, 8);
    TriangleOffset_(indices, offset, 4, 8, 1);
    TriangleOffset_(indices, offset, 8, 10, 1);
    TriangleOffset_(indices, offset, 8, 3, 10);
    TriangleOffset_(indices, offset, 5, 3, 8);
    TriangleOffset_(indices, offset, 5, 2, 3);
    TriangleOffset_(indices, offset, 2, 7, 3);
    TriangleOffset_(indices, offset, 7, 10, 3);
    TriangleOffset_(indices, offset, 7, 6, 10);
    TriangleOffset_(indices, offset, 7, 11, 6);
    TriangleOffset_(indices, offset, 11, 0, 6);
    TriangleOffset_(indices, offset, 0, 1, 6);
    TriangleOffset_(indices, offset, 6, 1, 10);
    TriangleOffset_(indices, offset, 9, 0, 11);
    TriangleOffset_(indices, offset, 9, 11, 2);
    TriangleOffset_(indices, offset, 9, 2, 5);
    TriangleOffset_(indices, offset, 7, 2, 11);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void Geosphere(size_t divisions, Indices& indices, Positions& uvw) {
    Assert(divisions);
    Assert(indices.empty());
    Assert(uvw.empty());

    indices.reserve(20 * 3 * (4 << divisions));
    uvw.reserve(12 * (1 << divisions));

    Icosahedron(indices, uvw);

    Indices indicesTmp;
    indicesTmp.reserve(indices.capacity());

    while (divisions-- >= 1)
    {
        DivideTriangles(indicesTmp, indices, uvw);
        std::swap(indicesTmp, indices);

        indicesTmp.clear();
    }

    for (float3& pos : uvw)
        pos = Normalize3(pos);
}
//----------------------------------------------------------------------------
void HemiGeosphere(size_t divisions, Indices& indices, Positions& uvw) {
    Assert(divisions);
    Assert(indices.empty());
    Assert(uvw.empty());

    indices.reserve(4 * 3 * (4 << divisions));
    uvw.reserve(5 * (1 << divisions));

    Pyramid(indices, uvw);

    Indices indicesTmp;
    indicesTmp.reserve(indices.capacity());

    while (divisions-- >= 1)
    {
        DivideTriangles(indicesTmp, indices, uvw);
        std::swap(indicesTmp, indices);

        indicesTmp.clear();
    }

    for (float3& pos : uvw)
        pos = Normalize3(pos);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void DivideTriangles(Indices& dstIdx, const Indices& srcIdx, Positions& uvw) {
    Assert(3 <= srcIdx.size());
    Assert(3 <= uvw.size());
    Assert(dstIdx.empty());

    dstIdx.reserve(srcIdx.size() * 4);

    HASHMAP_THREAD_LOCAL(Geometry, Edge_, u32) splits;
    splits.reserve(srcIdx.size()/(2/* edge */ * 2/* only half since A,B <=> B,A */));

    for (size_t t = 0; t < srcIdx.size(); t += 3) {
        const u32 ia = srcIdx[t + 0];
        const u32 ib = srcIdx[t + 1];
        const u32 ic = srcIdx[t + 2];

        const Edge_ edgeAB = ia < ib ? Edge_{ia, ib} : Edge_{ib, ia};
        const Edge_ edgeBC = ib < ic ? Edge_{ib, ic} : Edge_{ic, ib};
        const Edge_ edgeCA = ic < ia ? Edge_{ic, ia} : Edge_{ia, ic};

        u32 splitAB, splitBC, splitCA;

        if (!TryGetValue(splits, edgeAB, &splitAB))
        {
            splitAB = (splits[edgeAB] = checked_cast<u32>(uvw.size()));
            uvw.emplace_back((uvw[edgeAB.A] + uvw[edgeAB.B]) * 0.5f);
        }

        if (!TryGetValue(splits, edgeBC, &splitBC))
        {
            splitBC = (splits[edgeBC] = checked_cast<u32>(uvw.size()));
            uvw.emplace_back((uvw[edgeBC.A] + uvw[edgeBC.B]) * 0.5f);
        }

        if (!TryGetValue(splits, edgeCA, &splitCA))
        {
            splitCA = (splits[edgeCA] = checked_cast<u32>(uvw.size()));
            uvw.emplace_back((uvw[edgeCA.A] + uvw[edgeCA.B]) * 0.5f);
        }

        Triangle_(dstIdx, ia, splitAB, splitCA);
        Triangle_(dstIdx, splitAB, ib, splitBC);
        Triangle_(dstIdx, splitAB, splitBC, splitCA);
        Triangle_(dstIdx, splitCA, splitBC, ic);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void SmoothNormals(Normals& normals, const Indices& indices, const Positions& uvw) {
    Assert(indices.size() >= 3);
    Assert(uvw.size() >= 3);
    Assert(normals.empty());

    normals.resize(uvw.size(), float3(0));

    for (size_t i = 0; i < indices.size(); i += 3) {
        const u32 ia = indices[i + 0];
        const u32 ib = indices[i + 1];
        const u32 ic = indices[i + 2];

        const float3& pa = uvw[ia];
        const float3& pb = uvw[ib];
        const float3& pc = uvw[ic];

        const float3 normal = Cross(pb - pa, pc - pa);

        normals[ia] += normal;
        normals[ib] += normal;
        normals[ic] += normal;
    }

    for (float3& normal : normals)
        normal = Normalize3(normal);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ExportVertices(
    GenericVertex& vertices,

    const Positions *positions0,
    const Positions *positions1,
    const Positions *positions2,

    const Colors *colors0,
    const Colors *colors1,
    const Colors *colors2,

    const TexCoords *texCoords0,
    const TexCoords *texCoords1,
    const TexCoords *texCoords2,

    const Normals *normals0,
    const Normals *normals1,
    const Normals *normals2,

    const Tangents *tangents0,
    const Tangents *tangents1,
    const Tangents *tangents2,

    const Binormals *binormals0,
    const Binormals *binormals1,
    const Binormals *binormals2
    ) {
    Assert(vertices.VertexCountRemaining());
    Assert( positions0||positions1||positions2||
            colors0||colors1||colors2||
            texCoords0||texCoords1||texCoords2||
            normals0||normals1||normals2||
            tangents0||tangents1||tangents2||
            binormals0||binormals1||binormals2 );

    const GenericVertex::SubPart sp_positions0 = positions0 ? vertices.Position3f(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_positions1 = positions1 ? vertices.Position3f(1) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_positions2 = positions2 ? vertices.Position3f(2) : GenericVertex::SubPart::Null();

    const GenericVertex::SubPart sp_colors0 = colors0 ? vertices.Color4b(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_colors1 = colors1 ? vertices.Color4b(1) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_colors2 = colors2 ? vertices.Color4b(2) : GenericVertex::SubPart::Null();

    const GenericVertex::SubPart sp_texcoords0 = texCoords0 ? vertices.TexCoord2f(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_texcoords1 = texCoords1 ? vertices.TexCoord2f(1) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_texcoords2 = texCoords2 ? vertices.TexCoord2f(2) : GenericVertex::SubPart::Null();

    const GenericVertex::SubPart sp_normals0 = normals0 ? vertices.Normal3f(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_normals1 = normals1 ? vertices.Normal3f(1) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_normals2 = normals2 ? vertices.Normal3f(2) : GenericVertex::SubPart::Null();

    const GenericVertex::SubPart sp_tangents0 = tangents0 ? vertices.Tangent3f(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_tangents1 = tangents1 ? vertices.Tangent3f(1) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_tangents2 = tangents2 ? vertices.Tangent3f(2) : GenericVertex::SubPart::Null();

    const GenericVertex::SubPart sp_birnormals0 = binormals0 ? vertices.Binormal3f(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_birnormals1 = binormals1 ? vertices.Binormal3f(1) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_birnormals2 = binormals2 ? vertices.Binormal3f(2) : GenericVertex::SubPart::Null();

    size_t i = 0;
    do {
        vertices.ZeroMemory_CurrentVertex();

        if (sp_positions0) sp_positions0.WriteValue(vertices, (*positions0)[i]);
        if (sp_positions1) sp_positions1.WriteValue(vertices, (*positions1)[i]);
        if (sp_positions2) sp_positions2.WriteValue(vertices, (*positions2)[i]);

        if (sp_colors0) sp_colors0.WriteValue(vertices, (*colors0)[i]);
        if (sp_colors1) sp_colors1.WriteValue(vertices, (*colors1)[i]);
        if (sp_colors2) sp_colors2.WriteValue(vertices, (*colors2)[i]);

        if (sp_texcoords0) sp_texcoords0.WriteValue(vertices, (*texCoords0)[i]);
        if (sp_texcoords1) sp_texcoords1.WriteValue(vertices, (*texCoords1)[i]);
        if (sp_texcoords2) sp_texcoords2.WriteValue(vertices, (*texCoords2)[i]);

        if (sp_normals0) sp_normals0.WriteValue(vertices, (*normals0)[i]);
        if (sp_normals1) sp_normals1.WriteValue(vertices, (*normals1)[i]);
        if (sp_normals2) sp_normals2.WriteValue(vertices, (*normals2)[i]);

        if (sp_tangents0) sp_tangents0.WriteValue(vertices, (*tangents0)[i]);
        if (sp_tangents1) sp_tangents1.WriteValue(vertices, (*tangents1)[i]);
        if (sp_tangents2) sp_tangents2.WriteValue(vertices, (*tangents2)[i]);

        if (sp_birnormals0) sp_birnormals0.WriteValue(vertices, (*binormals0)[i]);
        if (sp_birnormals1) sp_birnormals1.WriteValue(vertices, (*binormals1)[i]);
        if (sp_birnormals2) sp_birnormals2.WriteValue(vertices, (*binormals2)[i]);

        ++i;
    }
    while (vertices.NextVertex());

    Assert(0 == vertices.VertexCountRemaining());
}
//----------------------------------------------------------------------------
void ExportVertices(
    GenericVertex& vertices,

    const Positions *positions0,
    const Colors *colors0,
    const TexCoords *texCoords0,
    const Normals *normals0,
    const Tangents *tangents0,
    const Binormals *binormals0 ) {
    Assert(vertices.VertexCountRemaining());
    Assert( positions0||colors0||texCoords0||normals0||tangents0||binormals0 );

    const GenericVertex::SubPart sp_positions0 = positions0 ? vertices.Position3f(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_colors0 = colors0 ? vertices.Color4b(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_texcoords0 = texCoords0 ? vertices.TexCoord2f(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_normals0 = normals0 ? vertices.Normal3f(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_tangents0 = tangents0 ? vertices.Tangent3f(0) : GenericVertex::SubPart::Null();
    const GenericVertex::SubPart sp_birnormals0 = binormals0 ? vertices.Binormal3f(0) : GenericVertex::SubPart::Null();

    size_t i = 0;
    do {
        vertices.ZeroMemory_CurrentVertex();

        if (sp_positions0) sp_positions0.WriteValue(vertices, (*positions0)[i]);
        if (sp_colors0) sp_colors0.WriteValue(vertices, (*colors0)[i]);
        if (sp_texcoords0) sp_texcoords0.WriteValue(vertices, (*texCoords0)[i]);
        if (sp_normals0) sp_normals0.WriteValue(vertices, (*normals0)[i]);
        if (sp_tangents0) sp_tangents0.WriteValue(vertices, (*tangents0)[i]);
        if (sp_birnormals0) sp_birnormals0.WriteValue(vertices, (*binormals0)[i]);

        ++i;
    }
    while (vertices.NextVertex());

    Assert(0 == vertices.VertexCountRemaining());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace GeometricPrimitive
} //!namespace Engine
} //!namespace Core
