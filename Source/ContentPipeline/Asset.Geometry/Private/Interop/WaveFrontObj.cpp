#include "stdafx.h"

#include "WaveFrontObj.h"

#include "GenericMaterial.h"
#include "GenericMesh.h"

#include "Container/Vector.h"
#include "IO/FS/ConstNames.h"
#include "IO/String.h"
#include "IO/VirtualFileSystem.h"
#include "Maths/ScalarVector.h"
#include "Memory/MemoryStream.h"

namespace PPE {
namespace Lattice {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FExtname& FWaveFrontObj::Ext = FFSConstNames::Obj();
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FWaveFrontObj::Load(FGenericMesh* dst, const FFilename& filename) {
    EAccessPolicy policy = EAccessPolicy::Binary;
    if (filename.Extname() == FFSConstNames::Objz())
        policy = policy + EAccessPolicy::Compress;

    RAWSTORAGE(FileSystem, u8) content;
    if (false == VFS_ReadAll(&content, filename, policy))
        return false;

    return Load(dst, filename, content.MakeView().Cast<const char>());
}
//----------------------------------------------------------------------------
bool FWaveFrontObj::Load(FGenericMesh* dst, const FFilename& filename, const FStringView& content) {
    Assert(dst);
    if (content.empty())
        return false;

    struct FVertex_ {
        u32 Position = (u32)-1;
        u32 TexCoord = (u32)-1;
        u32 Normal   = (u32)-1;
    };

    VECTOR(WaveFrontObj, float3) positions;
    VECTOR(WaveFrontObj, float2) texcoords;
    VECTOR(WaveFrontObj, float3) normals;
    VECTOR(WaveFrontObj, float3) colors;
    VECTOR(WaveFrontObj, FVertex_) vertices;

    FICStrStream iss(content);

    FString header;
    iss >> header;
    while (iss.good()) {
        bool readNext = true;
        while (iss.good() && header == "v") {
            float3 p;
            iss >> p._data[0] >> p._data[1] >> p._data[2];
            positions.push_back(p);

            const std::streampos offset = iss.tellg();
            float3 c;
            iss >> c._data[0] >> c._data[1] >> c._data[2];

            if (iss.fail()) {
                iss.seekg(offset);
                iss.clear();
            }
            else {
                colors.push_back(c);
            }

            iss >> header;
            readNext = false;
        }
        while (iss.good() && header == "vt") {
            float2 uv;
            iss >> uv._data[0] >> uv._data[1];
            texcoords.push_back(uv);
            iss >> header;
            readNext = false;
        }
        while (iss.good() && header == "vn") {
            float3 n;
            iss >> n._data[0] >> n._data[1] >> n._data[2];
            if (float len = Length3(n))
                n /= len;
            normals.push_back(n);
            iss >> header;
            readNext = false;
        }
        while (iss.good() && header == "f") {
            for (int i = 0; i < 3; ++i) {
                Assert(!iss.bad());

                FVertex_& vertex = vertices.push_back_Default();

                iss >> vertex.Position;
                iss.get(); // skip '/'
                iss >> vertex.TexCoord;
                iss.get(); // skip '/'
                iss >> vertex.Normal;

                vertex.Position--;
                vertex.TexCoord--;
                vertex.Normal--;
            }
            iss >> header;
            readNext = false;
        }

        if (readNext) {
            while ('\n' != iss.get() && iss.good());
            iss >> header;
        }
    }

    if (iss.bad())
        return false;

    const bool hasPositions = (positions.size() > 0);
    const bool hasTexCoords = (texcoords.size() > 0);
    const bool hasNormals = (normals.size() > 0);
    const bool hasColors = (colors.size() > 0);

    if (not hasPositions)
        return false;

    const size_t index = 0;
    const size_t vertexCount = vertices.size();

    const TMemoryView<float3> sp_position3f = dst->Position3f(index).Append(vertexCount);

    TMemoryView<float2> sp_texcoord2f;
    if (hasTexCoords)
        sp_texcoord2f = dst->TexCoord2f(index).Append(vertexCount);

    TMemoryView<float3> sp_normal3f;
    if (hasNormals)
        sp_normal3f = dst->Normal3f(index).Append(vertexCount);

    TMemoryView<float4> sp_color4f;
    if (hasColors)
        sp_color4f = dst->Color4f(index).Append(vertexCount);

    for (u32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex += 3) {
        const FVertex_& v0 = vertices[vertexIndex + 0];
        const FVertex_& v1 = vertices[vertexIndex + 1];
        const FVertex_& v2 = vertices[vertexIndex + 2];

        sp_position3f[vertexIndex + 0] = positions[v0.Position];
        sp_position3f[vertexIndex + 1] = positions[v1.Position];
        sp_position3f[vertexIndex + 2] = positions[v2.Position];

        if (hasTexCoords) {
            sp_texcoord2f[vertexIndex + 0] = texcoords[v0.TexCoord];
            sp_texcoord2f[vertexIndex + 1] = texcoords[v1.TexCoord];
            sp_texcoord2f[vertexIndex + 2] = texcoords[v2.TexCoord];
        }

        if (hasNormals) {
            sp_normal3f[vertexIndex + 0] = normals[v0.Normal];
            sp_normal3f[vertexIndex + 1] = normals[v1.Normal];
            sp_normal3f[vertexIndex + 2] = normals[v2.Normal];
        }

        if (hasColors) {
            sp_color4f[vertexIndex + 0] = colors[v0.Position].OneExtend();
            sp_color4f[vertexIndex + 1] = colors[v1.Position].OneExtend();
            sp_color4f[vertexIndex + 2] = colors[v2.Position].OneExtend();
        }

        dst->AddTriangle(vertexIndex + 0, vertexIndex + 1, vertexIndex + 2);
    }

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FWaveFrontObj::Save(const FGenericMesh* src, const FFilename& filename) {
    MEMORYSTREAM(Image) writer;
    if (false == Save(src, filename, &writer))
        return false;

    EAccessPolicy policy = EAccessPolicy::Truncate_Binary;
    if (filename.Extname() == FFSConstNames::Objz())
        policy = policy + EAccessPolicy::Compress;

    return VFS_WriteAll(filename, writer.MakeView(), policy);
}
//----------------------------------------------------------------------------
bool FWaveFrontObj::Save(const FGenericMesh* src, const FFilename& filename, IBufferedStreamWriter* writer) {
    Assert(src);
    Assert(writer);

    FStreamWriterOStream oss(writer);

    oss << "# Generated by Core::Lattice::FWaveFrontObj" << eol;
    oss << "o " << filename.BasenameNoExt() << eol;

    const size_t index = 0;

    const TGenericVertexSubPart<float3> sp_position3f = src->Position3f_IFP(index);
    const TGenericVertexSubPart<float4> sp_position4f = src->Position4f_IFP(index);
    if (!sp_position3f && !sp_position4f) {
        AssertNotReached();
        return 0;
    }

    const TGenericVertexSubPart<float4> sp_color4f = src->Color4f_IFP(index);

    if (sp_position3f) {
        const TMemoryView<const float3> positions = sp_position3f.MakeView();
        const TMemoryView<const float4> colors = sp_color4f.MakeView();
        forrange(v, 0, src->VertexCount()) {
            const float3& p = positions[v];
            oss << "v " << p.x() << ' ' << p.y() << ' ' << p.z();

            if (sp_color4f) {
                const float4& c = colors[v];
                oss << " " << c.x() << ' ' << c.y() << ' ' << c.z() << eol;
            }
            else {
                oss << eol;
            }
        }
    }
    else {
        const TMemoryView<const float4> positions = sp_position4f.MakeView();
        const TMemoryView<const float4> colors = sp_color4f.MakeView();
        forrange(v, 0, src->VertexCount()) {
            const float4& p = positions[v];
            oss << "v " << p.x() << ' ' << p.y() << ' ' << p.z();

            if (sp_color4f) {
                const float4& c = colors[v];
                oss << " " << c.x() << ' ' << c.y() << ' ' << c.z() << eol;
            }
            else {
                oss << eol;
            }
        }
    }

    if (const TGenericVertexSubPart<float2> sp_texcoord2f = src->TexCoord2f_IFP(index))
        for (const float2& uv : sp_texcoord2f.MakeView())
            oss << "vt " << uv.x() << ' ' << uv.y() << eol;

    if (const TGenericVertexSubPart<float3> sp_normal3f = src->Normal3f_IFP(index))
        for (const float3& n : sp_normal3f.MakeView())
            oss << "vn " << n.x() << ' ' << n.y() << ' ' << n.z() << eol;

    const TMemoryView<const u32> indices = src->Indices();
    for (uint32_t i = 0; i < indices.size(); i += 3) {
        const uint32_t v0 = indices[i + 0] + 1;
        const uint32_t v1 = indices[i + 1] + 1;
        const uint32_t v2 = indices[i + 2] + 1;
        oss << "f " << v0 << '/' << v0 << '/' << v0
            <<  ' ' << v1 << '/' << v1 << '/' << v1
            <<  ' ' << v2 << '/' << v2 << '/' << v2
            <<  eol;
    }

    return (oss.good());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Lattice
} //!namespace PPE

