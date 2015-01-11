#include "stdafx.h"

#include "GenericVertexExport.h"

#include "GenericVertex.h"
#include "GeometricPrimitives.h"

#include "Core/Memory/UniqueView.h"

#include "Core/Maths/MathHelpers.h"
#include "Core/Maths/Geometry/ScalarVector.h"
#include "Core/Maths/Geometry/ScalarVectorHelpers.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void ExportVertices(
    GenericVertex& vertices,

    const GeometricPrimitive::Positions *positions0,
    const GeometricPrimitive::Positions *positions1,
    const GeometricPrimitive::Positions *positions2,

    const GeometricPrimitive::Colors *colors0,
    const GeometricPrimitive::Colors *colors1,
    const GeometricPrimitive::Colors *colors2,

    const GeometricPrimitive::TexCoords *texCoords0,
    const GeometricPrimitive::TexCoords *texCoords1,
    const GeometricPrimitive::TexCoords *texCoords2,

    const GeometricPrimitive::Normals *normals0,
    const GeometricPrimitive::Normals *normals1,
    const GeometricPrimitive::Normals *normals2,

    const GeometricPrimitive::Tangents *tangents0,
    const GeometricPrimitive::Tangents *tangents1,
    const GeometricPrimitive::Tangents *tangents2,

    const GeometricPrimitive::Binormals *binormals0,
    const GeometricPrimitive::Binormals *binormals1,
    const GeometricPrimitive::Binormals *binormals2
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

    const GeometricPrimitive::Positions *positions0,
    const GeometricPrimitive::Colors *colors0,
    const GeometricPrimitive::TexCoords *texCoords0,
    const GeometricPrimitive::Normals *normals0,
    const GeometricPrimitive::Tangents *tangents0,
    const GeometricPrimitive::Binormals *binormals0 ) {
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
} //!namespace Engine
} //!namespace Core
