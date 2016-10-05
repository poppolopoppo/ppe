#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Color/Color.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/IO/StringView.h"
#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/ScalarMatrix.h"

namespace Core {
namespace Engine {
FWD_REFPTR(Model);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FModelBuilder {
public:
    template <typename _Tag, typename _N = u32>
    struct TTypedIndex {
        _N FValue;
        TTypedIndex() : FValue(0) {}
        TTypedIndex(_N value) : FValue(value) {}
        TTypedIndex& operator =(_N value) { FValue = value; return *this; }
        STATIC_CONST_INTEGRAL(_N, InvalidValue, 0xFEE1DEAD);
        operator _N () const { Assert(InvalidValue != FValue); return FValue; }
#ifdef WITH_CORE_ASSERT
        static void Uninitialized(TTypedIndex (&dst)[4]) { dst[0] = dst[1] = dst[2] = dst[3] = _N(InvalidValue); }
        static void Assign(TTypedIndex (&dst)[4], const TTypedIndex (&src)[3]) { dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = _N(InvalidValue); }
#else
        static void Uninitialized(TTypedIndex (&)[4]) { }
        static void Assign(TTypedIndex (&dst)[4], const TTypedIndex (&src)[3]) { dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; }
#endif
        static void Assign(TTypedIndex (&dst)[4], const TTypedIndex (&src)[4]) { dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; }
    };

    struct FPositionTag  {};
    struct FNormalTag    {};
    struct FTexcoordTag  {};

    using FPositionIndex = TTypedIndex<FPositionTag>;
    using FNormalIndex   = TTypedIndex<FNormalTag>;
    using FTexcoordIndex = TTypedIndex<FTexcoordTag>;

    struct EFace {
        FPositionIndex   P[4];
        FNormalIndex     N[4];
        FTexcoordIndex   T[4];

        EFace(const FPositionIndex (&pos)[3]);
        EFace(const FPositionIndex (&pos)[3], const FTexcoordIndex (&texcoords)[3]);
        EFace(const FPositionIndex (&pos)[3], const FTexcoordIndex (&texcoords)[3], const FNormalIndex (&normals)[3]);
        EFace(const FPositionIndex (&pos)[3], const FNormalIndex (&normals)[3]);

        EFace(const FPositionIndex (&pos)[4]);
        EFace(const FPositionIndex (&pos)[4], const FTexcoordIndex (&texcoords)[4]);
        EFace(const FPositionIndex (&pos)[4], const FTexcoordIndex (&texcoords)[4], const FNormalIndex (&normals)[4]);
        EFace(const FPositionIndex (&pos)[4], const FNormalIndex (&normals)[4]);
    };

    struct FBone {
        FString FName;
        float4x4 Transform;

        FBone(FString&& name, const float4x4& transform) : FName(std::move(name)), Transform(transform) { Assert(FName.size()); }
    };

    struct FGroup {
        enum EFlags : u32 {
            Default     = 0,

            Normals     = 1<<0,
            Quad        = 1<<1,
            Texcoords   = 1<<2,

            Texcoords_Normals = Texcoords|Normals,

            FTriangle = Default,
            Triangle_Normals = Default|Normals,
            Triangle_Texcoords = Default|Texcoords,
            Triangle_Texcoords_Normals = Triangle_Texcoords|Normals,

            Quad_Normals = Quad|Normals,
            Quad_Texcoords = Quad|Texcoords,
            Quad_Texcoords_Normals = Quad_Texcoords|Normals,
        };

        FString FName;
        u32 EMode;

        u32 FBone;
        u32 FMaterial;

        u32 FaceStart;
        u32 FaceCount;

        bool HasFlag(EFlags flag) const { return flag == (EMode & flag); }
        void SetFlag(EFlags flag) { EMode |= flag; }
        void RemoveFlag(EFlags flag) { EMode &= ~flag; }

        void SetMode_CheckCoherency(EFlags mode) { Assert(Default == EMode || mode == EMode); EMode = mode; }

        FGroup(FString&& name) :  FName(std::move(name)), EMode(Default)
                             ,  FBone(UINT32_MAX), FMaterial(UINT32_MAX)
                             ,  FaceStart(UINT32_MAX), FaceCount(UINT32_MAX) { Assert(FName.size()); }
    };

    struct FMaterial {
        enum EFlags : u32 {
            Default         = 0,

            Ambient         = 1<<0,
            BumpMapping     = 1<<1,
            Color           = 1<<2,
            Emissive        = 1<<3,
            Highlight       = 1<<4,
            Reflection      = 1<<5,
            Refraction      = 1<<6,
            SeparateAlpha   = 1<<7,
            Transparency    = 1<<8,
            Glass           = 1<<9,
            Fresnel         = 1<<10,
            CastShadows     = 1<<11,

            _InUse          = 1<<30,
        };

        FString FName;

        ColorRGBAF AmbientColor;
        ColorRGBAF DiffuseColor;
        ColorRGBAF EmissiveColor;
        ColorRGBAF SpecularColor;

        float NormalDepth;
        float RefractiveIndex;
        float SpecularExponent;

        FFilename AlphaMap;
        FFilename AmbientMap;
        FFilename DiffuseMap;
        FFilename DisplacementMap;
        FFilename EmissiveMap;
        FFilename NormalMap;
        FFilename ReflectionMap;
        FFilename SpecularColorMap;
        FFilename SpecularPowerMap;

        u32 EMode;

        bool HasFlag(EFlags flag) const { return flag == (EMode & flag); }
        void SetFlag(EFlags flag) { EMode |= flag; }
        void RemoveFlag(EFlags flag) { EMode &= ~flag; }

        FMaterial(FString&& name) : FName(std::move(name))
            , AmbientColor(-1)
            , DiffuseColor(-1)
            , EmissiveColor(-1)
            , NormalDepth(-1)
            , SpecularColor(-1)
            , RefractiveIndex(-1)
            , SpecularExponent(-1)
            , EMode(Default) { Assert(FName.size()); }
    };

    FModelBuilder();
    ~FModelBuilder();

    const FString& FName() const { return _name; }

    bool empty() const { return _positions.empty(); }

    const VECTOR_THREAD_LOCAL(MeshGeneration, float4)& Positions() const { return _positions; }
    const VECTOR_THREAD_LOCAL(MeshGeneration, ColorRGBAF)& Colors() const { return _colors; }
    const VECTOR_THREAD_LOCAL(MeshGeneration, float3)& Texcoords() const { return _texcoords; }
    const VECTOR_THREAD_LOCAL(MeshGeneration, float3)& Normals() const { return _normals; }

    const VECTOR_THREAD_LOCAL(MeshGeneration, FBone)& Bones() const { return _bones; }
    const VECTOR_THREAD_LOCAL(MeshGeneration, EFace)& Faces() const { return _faces; }
    const VECTOR_THREAD_LOCAL(MeshGeneration, FGroup)& Groups() const { return _groups; }
    const VECTOR_THREAD_LOCAL(MeshGeneration, FMaterial)& Materials() const { return _materials; }

    void SetName(const char *name) { SetName(FString(name)); }
    void SetName(FString&& name);

    void AddPosition(const float3& xyz);
    void AddPosition(const float4& xyzw);
    void AddColor(const ColorRGBAF& rgba);
    void AddTexCoord(const float2& uv);
    void AddTexCoord(const float3& uvw);
    void AddNormal(const float3& value);

    void AddTriangle(const FPositionIndex (&pos)[3]);
    void AddTriangle(const FPositionIndex (&pos)[3], const FTexcoordIndex (&texcoords)[3]);
    void AddTriangle(const FPositionIndex (&pos)[3], const FTexcoordIndex (&texcoords)[3], const FNormalIndex (&normals)[3]);
    void AddTriangle(const FPositionIndex (&pos)[3], const FNormalIndex (&normals)[3]);

    void AddQuad(const FPositionIndex (&pos)[4]);
    void AddQuad(const FPositionIndex (&pos)[4], const FTexcoordIndex (&texcoords)[4]);
    void AddQuad(const FPositionIndex (&pos)[4], const FTexcoordIndex (&texcoords)[4], const FNormalIndex (&normals)[4]);
    void AddQuad(const FPositionIndex (&pos)[4], const FNormalIndex (&normals)[4]);

    void AddBone(const char *name, const float4x4& transform) { AddBone(FString(name), transform); }
    void AddBone(FString&& name, const float4x4& transform);

    FGroup *OpenGroup(const char *name) { return OpenGroup(FString(name)); }
    FGroup *OpenGroup(FString&& name);
    const FGroup& OpenedGroup() const { Assert(_openGroup); return _groups.back(); }
    void CloseGroup(FGroup *group);

    FMaterial *OpenMaterial(const char *name) { return OpenMaterial(FString(name)); }
    FMaterial *OpenMaterial(FString&& name);
    const FMaterial& OpenedMaterial() const { Assert(_openMaterial); return _materials.back(); }
    void CloseMaterial(FMaterial *material);

    bool MaterialIndexFromName(size_t *pIndex, const FStringView& name) const;
    bool MaterialIndexFromName(size_t *pIndex, const FString& name) const { return MaterialIndexFromName(pIndex, FStringView(name.c_str(), name.size())); }
    bool MaterialIndexFromName(size_t *pIndex, const char *name) const { return MaterialIndexFromName(pIndex, FStringView(name, Length(name)) ); }

    PModel CreateModel();
    void Clear();

private:
    FGroup& OpenedGroup_() { Assert(_openGroup); return _groups.back(); }
    FMaterial& OpenedMaterial_() { Assert(_openMaterial); return _materials.back(); }

    FString _name;

#ifdef WITH_CORE_ASSERT
    bool _openGroup     : 1;
    bool _openMaterial  : 1;
#endif

    VECTOR_THREAD_LOCAL(MeshGeneration, float4) _positions;
    VECTOR_THREAD_LOCAL(MeshGeneration, ColorRGBAF) _colors;
    VECTOR_THREAD_LOCAL(MeshGeneration, float3) _texcoords;
    VECTOR_THREAD_LOCAL(MeshGeneration, float3) _normals;

    VECTOR_THREAD_LOCAL(MeshGeneration, FBone) _bones;
    VECTOR_THREAD_LOCAL(MeshGeneration, EFace) _faces;
    VECTOR_THREAD_LOCAL(MeshGeneration, FGroup) _groups;
    VECTOR_THREAD_LOCAL(MeshGeneration, FMaterial) _materials;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
