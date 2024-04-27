// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Mesh/Format/WaveFrontObj.h"

#include "Mesh/GenericMaterial.h"
#include "Mesh/GenericMesh.h"

#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "IO/ConstNames.h"
#include "IO/Filename.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "IO/TextReader.h"
#include "Maths/ScalarVector.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Memory/MemoryStream.h"
#include "Memory/SharedBuffer.h"

#include "VirtualFileSystem.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FExtname& FWaveFrontObj::Extname() NOEXCEPT {
    return FFSConstNames::Obj();
}
//----------------------------------------------------------------------------
// IMeshFormat
//----------------------------------------------------------------------------
bool FWaveFrontObj::ExportGenericMesh(IStreamWriter* output, const FGenericMesh& mesh) const {
    const FFilename sourceFile = mesh.SourceFile().value_or(Default);
    return UsingBufferedStream(output, [&](TPtrRef<IBufferedStreamWriter> buf) {
        return Save(mesh, sourceFile, buf);
    });
}
//----------------------------------------------------------------------------
FMeshBuilderResult FWaveFrontObj::ImportGenericMesh(const FRawMemoryConst& memory) const {
    FMemoryViewReader reader{ memory };
    return ImportGenericMesh(reader);
}
//----------------------------------------------------------------------------
FMeshBuilderResult FWaveFrontObj::ImportGenericMesh(IStreamReader& input) const {
    FGenericMesh result;
    if (Load(&result, Default, input))
        return Meta::MakeOptional(std::move(result));

    return std::nullopt;
}
//----------------------------------------------------------------------------
// Load
//----------------------------------------------------------------------------
bool FWaveFrontObj::Load(FGenericMesh* dst, const FFilename& filename) {
    EAccessPolicy policy = EAccessPolicy::Binary;
    if (filename.Extname() == FFSConstNames::Z())
        policy = policy + EAccessPolicy::Compress;

    const FUniqueBuffer buf = VFS_ReadAll(filename, policy);
    PPE_LOG_CHECK(MeshBuilder, buf);

    return Load(dst, filename, buf.MakeView().Cast<const char>());
}
//----------------------------------------------------------------------------
bool FWaveFrontObj::Load(FGenericMesh* dst, const FFilename& filename, IStreamReader& reader) {
    STACKLOCAL_POD_ARRAY(u8, raw, reader.SizeInBytes());
    PPE_LOG_CHECK(MeshBuilder, reader.ReadView(raw));
    PPE_LOG_CHECK(MeshBuilder, reader.Eof());
    return Load(dst, filename, raw.Cast<const char>());
}
//----------------------------------------------------------------------------
bool FWaveFrontObj::Load(FGenericMesh* dst, const FFilename& filename, const FStringView& content) {
    Assert(dst);

    if (not filename.empty())
        dst->SetSourceFile(filename);

    struct FVertex_ {
        u32 Position    = UMax;
        u32 Texcoord    = UMax;
        u32 Normal      = UMax;
    };

    VECTOR(MeshBuilder, float3) positions;
    VECTOR(MeshBuilder, float2) texcoords;
    VECTOR(MeshBuilder, float3) normals;
    VECTOR(MeshBuilder, float3) colors;
    VECTOR(MeshBuilder, FVertex_) vertices;

    FFilename mtllib;
    FString meshName;

    FStringView reader{ content };

    while (not reader.empty()) {
        FStringView line = Strip(EatUntil(reader, '\n'));
        EatSpaces(reader);
        if (line.empty() || line.front() == '#')
            continue;

        const FStringView header = EatIdentifier(line);
        EatSpaces(line);

        FMemoryViewReader stream{ line };
        FTextReader r{ &stream };

        if (EqualsI(header, "v")) {
            float3 v;
            if (r >> &v.x && r >> &v.y && r >> &v.z)
                positions.push_back(v);
            else
                return false;

            // optional color
            if (r >> &v.x && r >> &v.y && r >> &v.z)
                colors.push_back(v);
        }
        else if (EqualsI(header, "vt")) {
            float2 v;
            if (r >> &v.x && r >> &v.y)
                texcoords.push_back(v);
            else
                return false;
        }
        else if (EqualsI(header, "vn")) {
            float3 v;
            if (r >> &v.x && r >> &v.y && r >> &v.z)
                normals.push_back(SafeNormalize(v));
            else
                return false;
        }
        else if (EqualsI(header, "f")) {
            forrange(i, 0, 3) {
                FVertex_ v;
                if (r >> &v.Position && r.Expect('/') &&
                    r >> &v.Texcoord && r.Expect('/') &&
                    r >> &v.Normal)
                    vertices.push_back(v);
                else
                    return false;
            }
        }
        else if (EqualsI(header, "mtllib")) {
            mtllib = FFilename{UTF_8_TO_WCHAR(line)};
        }
        else if (EqualsI(header, "usemtl")) {
            PPE_LOG(MeshBuilder, Warning, "no support for wavefront obj material sections: {} {}", header, line);
        }
        else if (EqualsI(header, "o")) {
            meshName = line;
        }
        else {
            PPE_LOG(MeshBuilder, Warning, "ignored unknown wavefront obj header: {} {}", header, line);
        }
    }

    const bool hasPositions = (positions.size() > 0);
    const bool hasTexcoords = (texcoords.size() > 0);
    const bool hasNormals = (normals.size() > 0);
    const bool hasColors = (colors.size() > 0);

    if (not hasPositions)
        return false;

    const size_t index = 0;
    const size_t vertexCount = vertices.size();

    const TMemoryView<float3> sp_position3f = dst->Position3f(index).Append(vertexCount);

    TMemoryView<float2> sp_texcoord2f;
    if (hasTexcoords)
        sp_texcoord2f = dst->Texcoord2f(index).Append(vertexCount);

    TMemoryView<float3> sp_normal3f;
    if (hasNormals)
        sp_normal3f = dst->Normal3f(index).Append(vertexCount);

    TMemoryView<float4> sp_color4f;
    if (hasColors)
        sp_color4f = dst->Color4f(index).Append(vertexCount);

    for (size_t vertexIndex = 0; vertexIndex < vertexCount; vertexIndex += 3) {
        const FVertex_& v0 = vertices[vertexIndex + 0];
        const FVertex_& v1 = vertices[vertexIndex + 1];
        const FVertex_& v2 = vertices[vertexIndex + 2];

        sp_position3f[vertexIndex + 0] = positions[v0.Position - 1];
        sp_position3f[vertexIndex + 1] = positions[v1.Position - 1];
        sp_position3f[vertexIndex + 2] = positions[v2.Position - 1];

        if (hasTexcoords) {
            sp_texcoord2f[vertexIndex + 0] = texcoords[v0.Texcoord - 1];
            sp_texcoord2f[vertexIndex + 1] = texcoords[v1.Texcoord - 1];
            sp_texcoord2f[vertexIndex + 2] = texcoords[v2.Texcoord - 1];
        }

        if (hasNormals) {
            sp_normal3f[vertexIndex + 0] = normals[v0.Normal - 1];
            sp_normal3f[vertexIndex + 1] = normals[v1.Normal - 1];
            sp_normal3f[vertexIndex + 2] = normals[v2.Normal - 1];
        }

        if (hasColors) {
            sp_color4f[vertexIndex + 0] = float4(colors[v0.Position - 1], 1.f);
            sp_color4f[vertexIndex + 1] = float4(colors[v1.Position - 1], 1.f);
            sp_color4f[vertexIndex + 2] = float4(colors[v2.Position - 1], 1.f);
        }

        dst->AddTriangle(vertexIndex + 0, vertexIndex + 1, vertexIndex + 2);
    }

    return true;
}
//----------------------------------------------------------------------------
// Save
//----------------------------------------------------------------------------
bool FWaveFrontObj::Save(const FGenericMesh& src, const FFilename& filename) {
    MEMORYSTREAM(MeshBuilder) writer;
    if (false == Save(src, filename, writer))
        return false;

    EAccessPolicy policy = EAccessPolicy::Truncate_Binary;
    if (filename.Extname() == FFSConstNames::Z())
        policy = policy + EAccessPolicy::Compress;

    return VFS_WriteAll(filename, writer.MakeView(), policy);
}
//----------------------------------------------------------------------------
bool FWaveFrontObj::Save(const FGenericMesh& src, const FFilename& filename, IBufferedStreamWriter& writer) {
    FTextWriter oss{ writer };

    oss << "# Generated by PPE/MeshBuilder/FWaveFrontObj" << Eol;
    oss << "o " << filename.BasenameNoExt() << Eol;

    const size_t index = 0;

    const TGenericVertexSubPart<float3> sp_position3f = src.Position3f_IFP(index);
    const TGenericVertexSubPart<float4> sp_position4f = src.Position4f_IFP(index);
    if (!sp_position3f && !sp_position4f) {
        AssertNotReached();
        return 0;
    }

    const TGenericVertexSubPart<float4> sp_color4f = src.Color4f_IFP(index);

    if (sp_position3f) {
        const TMemoryView<const float3> positions = sp_position3f.MakeView();
        const TMemoryView<const float4> colors = sp_color4f.MakeView();
        forrange(v, 0, src.VertexCount()) {
            const float3& p = positions[v];
            oss << "v " << p.x << ' ' << p.y << ' ' << p.z;

            if (sp_color4f) {
                const float4& c = colors[v];
                oss << " " << c.x << ' ' << c.y << ' ' << c.z << Eol;
            }
            else {
                oss << Eol;
            }
        }
    }
    else {
        const TMemoryView<const float4> positions = sp_position4f.MakeView();
        const TMemoryView<const float4> colors = sp_color4f.MakeView();
        forrange(v, 0, src.VertexCount()) {
            const float4& p = positions[v];
            oss << "v " << p.x << ' ' << p.y << ' ' << p.z;

            if (sp_color4f) {
                const float4& c = colors[v];
                oss << " " << c.x << ' ' << c.y << ' ' << c.z << Eol;
            }
            else {
                oss << Eol;
            }
        }
    }

    if (const TGenericVertexSubPart<float2> sp_texcoord2f = src.Texcoord2f_IFP(index))
        for (const float2& uv : sp_texcoord2f.MakeView())
            oss << "vt " << uv.x << ' ' << uv.y << Eol;

    if (const TGenericVertexSubPart<float3> sp_normal3f = src.Normal3f_IFP(index))
        for (const float3& n : sp_normal3f.MakeView())
            oss << "vn " << n.x << ' ' << n.y << ' ' << n.z << Eol;

    const TMemoryView<const u32> indices = src.Indices();
    for (size_t i = 0; i < indices.size(); i += 3) {
        const uint32_t v0 = indices[i + 0] + 1;
        const uint32_t v1 = indices[i + 1] + 1;
        const uint32_t v2 = indices[i + 2] + 1;
        oss << "f " << v0 << '/' << v0 << '/' << v0
            <<  ' ' << v1 << '/' << v1 << '/' << v1
            <<  ' ' << v2 << '/' << v2 << '/' << v2
            <<  Eol;
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
