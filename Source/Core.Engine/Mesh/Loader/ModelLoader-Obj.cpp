#include "stdafx.h"

#include "ModelLoader.h"

#include "ModelBuilder.h"

#include "Core.Engine/Material/MaterialConstNames.h"

#include "Core.Engine/Mesh/Model.h"
#include "Core.Engine/Mesh/ModelMesh.h"
#include "Core.Engine/Mesh/ModelMeshSubPart.h"

#include "Core/Container/RawStorage.h"
#include "Core/IO/VirtualFileSystem.h"

#include <cctype>

// Object Files (.obj)
// http://paulbourke.net/dataformats/obj/

// MTL material format (Lightwave, OBJ)
// http://paulbourke.net/dataformats/mtl/

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct LineTokens_Obj_ {
    STATIC_CONST_INTEGRAL(size_t, Capacity, 5);
    size_t Count;
    MemoryView<const char> Slices[Capacity];
};
//----------------------------------------------------------------------------
static bool ReadLine_Obj_ReturnEOF_(LineTokens_Obj_ *pTokens, const ModelStream& modelStream, size_t& offset) {
    Assert(pTokens);

    bool comment = false;
    pTokens->Count = 0;

    size_t tokenStart = offset;
    while (modelStream.SizeInBytes() > offset && '\n' != modelStream[offset])
        if (IsSpace(modelStream[offset])) {
            if (!comment && tokenStart != offset) {
                Assert(offset - tokenStart > 0);
                Assert(pTokens->Count < LineTokens_Obj_::Capacity);
                pTokens->Slices[pTokens->Count++] = MemoryView<const char>(modelStream.Pointer() + tokenStart, offset - tokenStart);
            }
            tokenStart = ++offset;
        }
        else if ('#' == modelStream[offset]) {
            comment = true;
            ++offset;
        }
        else {
            ++offset;
        }

    if (!comment && tokenStart != offset) {
        Assert(offset - tokenStart > 0);
        Assert(pTokens->Count < LineTokens_Obj_::Capacity);
        pTokens->Slices[pTokens->Count++] = MemoryView<const char>(modelStream.Pointer() + tokenStart, offset - tokenStart);
    }

    if ('\n' == modelStream[offset])
        ++offset;

    return (modelStream.SizeInBytes() == offset);
}
//----------------------------------------------------------------------------
static size_t ParseFloat4_Obj_ReturnDim_(float4 *pValues, const LineTokens_Obj_& tokens) {
    if      (tokens.Count > 1 && !Atof(&pValues->_data[0], tokens.Slices[1]) )
        return 0;
    else if (tokens.Count > 2 && !Atof(&pValues->_data[1], tokens.Slices[2]) )
        return 0;
    else if (tokens.Count > 3 && !Atof(&pValues->_data[2], tokens.Slices[3]) )
        return 0;
    else if (tokens.Count > 4 && !Atof(&pValues->_data[3], tokens.Slices[4]) )
        return 0;
    else
        return tokens.Count - 1;
}
//----------------------------------------------------------------------------
static size_t ParseU323_Obj_ReturnDim_(u323 *pValues, const MemoryView<const char>& token) {

    size_t index = 0;
    StringSlice slice;

    const char *cstr = token.Pointer();
    size_t length = token.size();

    while (Split(&cstr, &length, '/', slice)) {
        Assert(index < 3);

        u32 v = 0;
        if (!Atoi<10>(&v, slice))
            return 0;

        pValues->_data[index++] = v;
    }

    return index;
}
//----------------------------------------------------------------------------
static bool LoadMaterial_Mtl_(ModelBuilder& builder, const Filename& filename) {

    RAWSTORAGE_THREAD_LOCAL(MeshGeneration, char) materialStream;
    if (!VirtualFileSystem::Instance().ReadAll(filename, materialStream))
        return false;

    ModelBuilder::Material *pMaterialMB = nullptr;

    LineTokens_Obj_ tokens;
    bool eof = false;
    size_t offset = 0;
    do {
        eof = ReadLine_Obj_ReturnEOF_(&tokens, materialStream, offset);
        if (0 == tokens.Count)
            continue;

        const MemoryView<const char>& firstToken = tokens.Slices[0];
        Assert(firstToken.size());

        if (0 == CompareNI("newmtl", firstToken.Pointer(), firstToken.size())) {
            if (pMaterialMB)
                builder.CloseMaterial(pMaterialMB);

            if (tokens.Count != 2  || tokens.Slices[1].empty())
                return false;

            String name(tokens.Slices[1].Pointer(), tokens.Slices[1].size());
            pMaterialMB = builder.OpenMaterial(std::move(name));
        }
        // parameters
        else if (0 == CompareNI("Ka", firstToken.Pointer(), firstToken.size())) {
            float4 color(0,0,0,1);
            if (ParseFloat4_Obj_ReturnDim_(&color, tokens) != 3)
                return false;

            Assert(pMaterialMB);
            pMaterialMB->AmbientColor = color;
        }
        else if (0 == CompareNI("Kd", firstToken.Pointer(), firstToken.size())) {
            float4 color(0,0,0,1);
            if (ParseFloat4_Obj_ReturnDim_(&color, tokens) != 3)
                return false;

            Assert(pMaterialMB);
            pMaterialMB->DiffuseColor = color;
        }
        else if (0 == CompareNI("Ke", firstToken.Pointer(), firstToken.size())) {
            float4 color(0,0,0,1);
            if (ParseFloat4_Obj_ReturnDim_(&color, tokens) != 3)
                return false;

            Assert(pMaterialMB);
            pMaterialMB->EmissiveColor = color;
            pMaterialMB->SetFlag(ModelBuilder::Material::Emissive);
        }
        else if (0 == CompareNI("Ks", firstToken.Pointer(), firstToken.size())) {
            float4 color(0,0,0,1);
            if (ParseFloat4_Obj_ReturnDim_(&color, tokens) != 3)
                return false;

            Assert(pMaterialMB);
            pMaterialMB->SpecularColor = color;
        }
        else if (0 == CompareNI("Ni", firstToken.Pointer(), firstToken.size())) {
            float4 color(0,0,0,1);
            if (ParseFloat4_Obj_ReturnDim_(&color, tokens) != 1)
                return false;

            Assert(pMaterialMB);
            pMaterialMB->RefractiveIndex = color.x();
        }
        else if (0 == CompareNI("Ns", firstToken.Pointer(), firstToken.size())) {
            float4 color(0,0,0,1);
            if (ParseFloat4_Obj_ReturnDim_(&color, tokens) != 1)
                return false;

            Assert(pMaterialMB);
            pMaterialMB->SpecularExponent = color.x();
        }
        else if (0 == CompareNI("Tf", firstToken.Pointer(), firstToken.size())) {
            float4 color(0,0,0,1);
            if (ParseFloat4_Obj_ReturnDim_(&color, tokens) != 3)
                return false;

            Assert(pMaterialMB);
            //pMaterialMB->SpecularExponent = color.x(); // not supported ...
        }
        else if (0 == CompareNI("Tr", firstToken.Pointer(), firstToken.size()) ||
                 0 == CompareNI("d ", firstToken.Pointer(), firstToken.size()) ) {
            float4 color(0,0,0,1);
            if (ParseFloat4_Obj_ReturnDim_(&color, tokens) != 1)
                return false;

            Assert(pMaterialMB);
            pMaterialMB->DiffuseColor.a() = color.x();
        }
        else if (0 == CompareNI("illum", firstToken.Pointer(), firstToken.size()) ) {
            if (tokens.Count != 2  || tokens.Slices[1].empty())
                return false;

            u32 illum;
            if (!Atoi<10>(&illum, tokens.Slices[1]))
                return false;

            u32 mode = 0;
            switch (illum) {
            case 0:
                mode = ModelBuilder::Material::Color;
                break;
            case 1:
                mode = ModelBuilder::Material::Ambient|ModelBuilder::Material::Color;
                break;
            case 2:
                mode = ModelBuilder::Material::Highlight;
                break;
            case 3:
                mode = ModelBuilder::Material::Reflection;
                break;
            case 4:
                mode = ModelBuilder::Material::Glass|ModelBuilder::Material::Reflection|ModelBuilder::Material::Transparency;
                break;
            case 5:
                mode = ModelBuilder::Material::Fresnel|ModelBuilder::Material::Reflection;
                break;
            case 6:
                mode = ModelBuilder::Material::Refraction|ModelBuilder::Material::Transparency;
                break;
            case 7:
                mode = ModelBuilder::Material::Fresnel|ModelBuilder::Material::Reflection|ModelBuilder::Material::Refraction|ModelBuilder::Material::Transparency;
                break;
            case 8:
                mode = ModelBuilder::Material::Reflection;
                break;
            case 9:
                mode = ModelBuilder::Material::Glass|ModelBuilder::Material::Reflection|ModelBuilder::Material::Transparency;
                break;
            case 10:
                mode = ModelBuilder::Material::CastShadows;
                break;
            default:
                return false;
            }

            mode |= ModelBuilder::Material::CastShadows; //default?

            Assert(pMaterialMB);
            pMaterialMB->Mode = mode;
        }
        // textures
        else if (0 == CompareNI("map_Ka", firstToken.Pointer(), firstToken.size())) {
            if (tokens.Count != 2  || tokens.Slices[1].empty())
                return false;

            const WString relfilename = ToWString(tokens.Slices[1]);
            const Filename textureFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());

            Assert(pMaterialMB);
            pMaterialMB->AmbientMap = textureFilename;
        }
        else if (0 == CompareNI("map_Kd", firstToken.Pointer(), firstToken.size())) {
            if (tokens.Count != 2  || tokens.Slices[1].empty())
                return false;

            const WString relfilename = ToWString(tokens.Slices[1]);
            const Filename textureFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());

            Assert(pMaterialMB);
            pMaterialMB->DiffuseMap = textureFilename;
        }
        else if (0 == CompareNI("map_Ke", firstToken.Pointer(), firstToken.size())) {
            if (tokens.Count != 2  || tokens.Slices[1].empty())
                return false;

            const WString relfilename = ToWString(tokens.Slices[1]);
            const Filename textureFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());

            Assert(pMaterialMB);
            pMaterialMB->EmissiveMap = textureFilename;
        }
        else if (0 == CompareNI("map_Ks", firstToken.Pointer(), firstToken.size())) {
            if (tokens.Count != 2  || tokens.Slices[1].empty())
                return false;

            const WString relfilename = ToWString(tokens.Slices[1]);
            const Filename textureFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());

            Assert(pMaterialMB);
            pMaterialMB->SpecularColorMap = textureFilename;
        }
        else if (0 == CompareNI("map_Ns", firstToken.Pointer(), firstToken.size())) {
            if (tokens.Count != 2  || tokens.Slices[1].empty())
                return false;

            const WString relfilename = ToWString(tokens.Slices[1]);
            const Filename textureFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());

            Assert(pMaterialMB);
            pMaterialMB->SpecularPowerMap = textureFilename;
        }
        else if (0 == CompareNI("map_d", firstToken.Pointer(), firstToken.size())) {
            if (tokens.Count != 2  || tokens.Slices[1].empty())
                return false;

            const WString relfilename = ToWString(tokens.Slices[1]);
            const Filename textureFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());
            Assert(!textureFilename.empty());

            Assert(pMaterialMB);
            pMaterialMB->AlphaMap = textureFilename;
            pMaterialMB->SetFlag(ModelBuilder::Material::SeparateAlpha);
        }
        else if (0 == CompareNI("map_bump", firstToken.Pointer(), firstToken.size()) ||
                 0 == CompareNI("bump", firstToken.Pointer(), firstToken.size()) ) {
            if (tokens.Slices[1].empty())
                return false;

            if (2 == tokens.Count) {
                const WString relfilename = ToWString(tokens.Slices[1]);
                const Filename textureFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());

                Assert(pMaterialMB);
                pMaterialMB->NormalMap = textureFilename;
                pMaterialMB->NormalDepth = 1.0f;
            }
            else if (4 == tokens.Count) {
                if (0 != CompareNI("-bm", tokens.Slices[1].Pointer(), tokens.Slices[1].size()))
                    return false;

                float normalDepth;
                if (!Atof(&normalDepth, tokens.Slices[2]))
                    return false;

                const WString relfilename = ToWString(tokens.Slices[3]);
                const Filename textureFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());

                Assert(pMaterialMB);
                pMaterialMB->NormalMap = textureFilename;
                pMaterialMB->NormalDepth = normalDepth;
            }
            else
                return false;

            Assert(!pMaterialMB->NormalMap.empty());
            pMaterialMB->SetFlag(ModelBuilder::Material::BumpMapping);
        }
        else if (0 == CompareNI("disp", firstToken.Pointer(), firstToken.size()) ) {
            if (tokens.Count != 2  || tokens.Slices[1].empty())
                return false;

            const WString relfilename = ToWString(tokens.Slices[1]);
            const Filename textureFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());

            Assert(pMaterialMB);
            pMaterialMB->DisplacementMap = textureFilename;
        }
        else if (0 == CompareNI("refl", firstToken.Pointer(), firstToken.size()) ) {
            if (tokens.Count != 2  || tokens.Slices[1].empty())
                return false;

            const WString relfilename = ToWString(tokens.Slices[1]);
            const Filename textureFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());

            Assert(pMaterialMB);
            pMaterialMB->ReflectionMap = textureFilename;
        }
        else {
            return false; // unknown keyword
        }
    }
    while (!eof);

    if (pMaterialMB)
        builder.CloseMaterial(pMaterialMB);

    return true;
}
//----------------------------------------------------------------------------
static bool LoadModel_Obj_(PModel& pModel, const Filename& filename, const ModelStream& modelStream) {

    ModelBuilder builder;
    {
        String name = ToString(filename.BasenameNoExt().c_str());
        builder.SetName(std::move(name));
        builder.AddBone("noname", float4x4::Identity()); // obj file format does not support animations
    }

    ModelBuilder::Group *pGroupMB = nullptr;

    bool smoothGroup = false;
    u32 selectedSmoothGroup = 0;
    u32 selectedMaterial = UINT32_MAX;

    LineTokens_Obj_ tokens;
    bool eof = false;
    size_t offset = 0;
    do {
        eof = ReadLine_Obj_ReturnEOF_(&tokens, modelStream, offset);
        if (0 == tokens.Count)
            continue;

        const MemoryView<const char>& firstToken = tokens.Slices[0];

        if (1 == firstToken.size()) {
            const char ch = ToLower(firstToken[0]);
            if ('v' == ch) {
                float4 pos(0,0,0,1);
                if (ParseFloat4_Obj_ReturnDim_(&pos, tokens) < 3)
                    return false;

                builder.AddPosition(pos);
            }
            else if ('f' == ch) {
                if (tokens.Count < 4)
                    return false;

                if (!pGroupMB) {
                    pGroupMB = builder.OpenGroup("noname");
                    pGroupMB->Bone = 0;
                    pGroupMB->Material = selectedMaterial;
                }

                u323 v0, v1, v2;

                const size_t dim0 = ParseU323_Obj_ReturnDim_(&v0, tokens.Slices[1]);
                if (0 == dim0)
                    return false;
                const size_t dim1 = ParseU323_Obj_ReturnDim_(&v1, tokens.Slices[2]);
                if (0 == dim1)
                    return false;
                const size_t dim2 = ParseU323_Obj_ReturnDim_(&v2, tokens.Slices[3]);
                if (0 == dim2)
                    return false;

                // offsets start from 1, vectors start from 0 :
                v0 -= 1;
                v1 -= 1;
                v2 -= 1;

                if (5 == tokens.Count) { // quads
                    u323 v3;
                    const size_t dim3 = ParseU323_Obj_ReturnDim_(&v3, tokens.Slices[4]);
                    if (0 == dim3)
                        return false;

                    if (dim0 != dim1 || dim1 != dim2 || dim2 != dim3)
                        return false;

                    v3 -= 1;

                    if (1 == dim0) { // position
                        const ModelBuilder::PositionIndex pos[4] = {v0.x(),v1.x(),v2.x(),v3.x()};
                        builder.AddQuad(pos);
                    }
                    else if (2 == dim0) { // position/texcoords
                        const ModelBuilder::PositionIndex pos[4] = {v0.x(),v1.x(),v2.x(),v3.x()};
                        const ModelBuilder::TexcoordIndex uv[4] = {v0.y(),v1.y(),v2.y(),v3.y()};
                        builder.AddQuad(pos, uv);
                    }
                    else if (3 == dim0) { // position/texcoords/normal
                        const ModelBuilder::PositionIndex pos[4] = {v0.x(),v1.x(),v2.x(),v3.x()};
                        const ModelBuilder::TexcoordIndex uv[4] = {v0.y(),v1.y(),v2.y(),v3.y()};
                        const ModelBuilder::NormalIndex nrm[4] = {v0.z(),v1.z(),v2.z(),v3.z()};
                        builder.AddQuad(pos, uv, nrm);
                    }
                    else
                        return false;
                }
                else { // triangles
                    if (dim0 != dim1 || dim1 != dim2)
                        return false;

                    if (1 == dim0) { // position
                        const ModelBuilder::PositionIndex pos[3] = {v0.x(),v1.x(),v2.x()};
                        builder.AddTriangle(pos);
                    }
                    else if (2 == dim0) { // position/texcoords
                        const ModelBuilder::PositionIndex pos[3] = {v0.x(),v1.x(),v2.x()};
                        const ModelBuilder::TexcoordIndex uv[3] = {v0.y(),v1.y(),v2.y()};
                        builder.AddTriangle(pos, uv);
                    }
                    else if (3 == dim0) { // position/texcoords/normal
                        const ModelBuilder::PositionIndex pos[3] = {v0.x(),v1.x(),v2.x()};
                        const ModelBuilder::TexcoordIndex uv[3] = {v0.y(),v1.y(),v2.y()};
                        const ModelBuilder::NormalIndex nrm[3] = {v0.z(),v1.z(),v2.z()};
                        builder.AddTriangle(pos, uv, nrm);
                    }
                    else
                        return false;
                }

            }
            else if ('o' == ch) {
                if (tokens.Count != 2  || tokens.Slices[1].empty())
                    return false;

                String name(tokens.Slices[1].Pointer(), tokens.Slices[1].size());
                builder.SetName(std::move(name));
            }
            else if ('g' == ch) {
                if (tokens.Count != 2  || tokens.Slices[1].empty())
                    return false;

                if (pGroupMB) {
                    Assert(UINT32_MAX != pGroupMB->Material);
                    builder.CloseGroup(pGroupMB);
                }

                String name(tokens.Slices[1].Pointer(), tokens.Slices[1].size());
                pGroupMB = builder.OpenGroup(std::move(name));
                pGroupMB->Bone = 0;
                pGroupMB->Material = UINT32_MAX;
            }
            else if ('s' == ch) {
                if (tokens.Count != 2 || tokens.Slices[1].empty())
                    return false;

                if (0 == CompareNI("off", tokens.Slices[1].Pointer(), tokens.Slices[1].size())) {
                    smoothGroup = false;
                    selectedSmoothGroup = UINT32_MAX;
                }
                else {
                    smoothGroup = true;
                    if (!Atoi<10>(&selectedSmoothGroup, tokens.Slices[1]))
                        return false;
                }
            }
        }
        else if (2 == firstToken.size()) {
            if ('v' != ToLower(firstToken[0]) )
                return false;

            const char ch = ToLower(firstToken[1]);

            if ('p' == ch) {
                AssertNotImplemented(); // parametrized surfaces are not handled
                return false;
            }
            else if ('t' == ch) {
                float4 uvw(0,0,0,0);
                const size_t dim = ParseFloat4_Obj_ReturnDim_(&uvw, tokens);
                if (1 == dim || 4 == dim)
                    return false;

                uvw.y() = 1 - uvw.y(); // uv.y() needs to be flipped, not the same convention

                if (2 == dim)
                    builder.AddTexCoord(uvw.xy());
                else
                    builder.AddTexCoord(uvw.xyz());
            }
            else if ('n' == ch) {
                float4 nrm(0,0,0,0);
                const size_t dim = ParseFloat4_Obj_ReturnDim_(&nrm, tokens);
                if (3 != dim)
                    return false;

                builder.AddNormal(nrm.xyz());
            }
        }
        else {
            if (0 == CompareNI(tokens.Slices[0].Pointer(), "mtllib", tokens.Slices[0].size())) {
                const WString relfilename = ToWString(tokens.Slices[1]);
                const Filename materialFilename(filename.Dirpath(), relfilename.c_str(), relfilename.size());

                if (!LoadMaterial_Mtl_(builder, materialFilename))
                    return false;
            }
            else if (0 == CompareNI(tokens.Slices[0].Pointer(), "usemtl", tokens.Slices[0].size())) {
                if (tokens.Count != 2)
                    return false;

                size_t m = 0;
                if (!builder.MaterialIndexFromName(&m, tokens.Slices[1]))
                    return false;

                selectedMaterial = checked_cast<u32>(m);
                Assert(UINT32_MAX != selectedMaterial);

                Assert(pGroupMB);
                if (selectedMaterial != pGroupMB->Material) {
                    if (UINT32_MAX != pGroupMB->Material) {
                        builder.CloseGroup(pGroupMB);
                        Assert(pGroupMB->FaceCount > 0);

                        String name(pGroupMB->Name);
                        const size_t bone = pGroupMB->Bone;
                        pGroupMB = builder.OpenGroup(std::move(name));
                        pGroupMB->Bone = checked_cast<u32>(bone);
                    }
                    Assert(UINT32_MAX == pGroupMB->Material);
                    pGroupMB->Material = selectedMaterial;
                }
            }
            else {
                return false; // unknown keyword
            }
        }
    }
    while (!eof);

    if (pGroupMB) {
        Assert(UINT32_MAX != pGroupMB->Material);
        builder.CloseGroup(pGroupMB);
    }

    pModel = builder.CreateModel();

    return (pModel != nullptr);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool LoadModel_Obj(PModel& pModel, const Filename& filename, const ModelStream& modelStream) {
    Assert(!pModel);
    Assert(!filename.empty());
    Assert(modelStream.size());

    return LoadModel_Obj_(pModel, filename, modelStream);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
