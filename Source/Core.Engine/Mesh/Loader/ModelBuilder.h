#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Color/Color.h"
#include "Core/Container/Vector.h"
#include "Core/IO/FS/Filename.h"
#include "Core/IO/String.h"
#include "Core/Maths/Geometry/ScalarVector_fwd.h"
#include "Core/Maths/Transform/ScalarMatrix.h"

namespace Core {
namespace Engine {
FWD_REFPTR(Model);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ModelBuilder {
public:
    template <typename _Tag, typename _N = u32>
    struct TypedIndex {
        _N Value;
        TypedIndex() : Value(0) {}
        TypedIndex(_N value) : Value(value) {}
        TypedIndex& operator =(_N value) { Value = value; return *this; }
        STATIC_CONST_INTEGRAL(_N, InvalidValue, 0xFEE1DEAD);
        operator _N () const { Assert(InvalidValue != Value); return Value; }
#ifdef WITH_CORE_ASSERT
        static void Uninitialized(TypedIndex (&dst)[4]) { dst[0] = dst[1] = dst[2] = dst[3] = _N(InvalidValue); }
        static void Assign(TypedIndex (&dst)[4], const TypedIndex (&src)[3]) { dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = _N(InvalidValue); }
#else
        static void Uninitialized(TypedIndex (&)[4]) { }
        static void Assign(TypedIndex (&dst)[4], const TypedIndex (&src)[3]) { dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; }
#endif
        static void Assign(TypedIndex (&dst)[4], const TypedIndex (&src)[4]) { dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2]; dst[3] = src[3]; }
    };

    struct PositionTag  {};
    struct NormalTag    {};
    struct TexcoordTag  {};

    using PositionIndex = TypedIndex<PositionTag>;
    using NormalIndex   = TypedIndex<NormalTag>;
    using TexcoordIndex = TypedIndex<TexcoordTag>;

    struct Face {
        PositionIndex P[4];
        NormalIndex N[4];
        TexcoordIndex T[4];

        Face(const PositionIndex (&pos)[3]);
        Face(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3]);
        Face(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3], const NormalIndex (&normals)[3]);
        Face(const PositionIndex (&pos)[3], const NormalIndex (&normals)[3]);

        Face(const PositionIndex (&pos)[4]);
        Face(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4]);
        Face(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4], const NormalIndex (&normals)[4]);
        Face(const PositionIndex (&pos)[4], const NormalIndex (&normals)[4]);
    };

    struct Bone {
        String Name;
        float4x4 Transform;

        Bone(String&& name, const float4x4& transform) : Name(std::move(name)), Transform(transform) { Assert(Name.size()); }
    };

    struct Group {
        enum Flags : u32 {
            Default     = 0,

            Normals     = 1<<0,
            Quad        = 1<<1,
            Smooth      = 1<<2,
            Texcoords   = 1<<3,

            Texcoords_Normals = Texcoords|Normals,

            Triangle = Default,
            Triangle_Normals = Default|Normals,
            Triangle_Texcoords = Default|Texcoords,
            Triangle_Texcoords_Normals = Triangle_Texcoords|Normals,

            Quad_Normals = Quad|Normals,
            Quad_Texcoords = Quad|Texcoords,
            Quad_Texcoords_Normals = Quad_Texcoords|Normals,
        };

        String Name;
        u32 Mode;

        u32 SubPartStart;
        u32 SubPartCount;

        bool HasFlag(Flags flag) const { return flag == (Mode & flag); }
        void SetFlag(Flags flag) { Mode |= flag; }

        void SetMode_CheckCoherency(Flags mode) { Assert(Default == Mode || mode == Mode); Mode = mode; }

        Group(String&& name) : Name(std::move(name)), Mode(Default), SubPartStart(UINT32_MAX), SubPartCount(UINT32_MAX) { Assert(Name.size()); }
    };

    struct Material {
        enum Flags : u32 {
            Default         = 0,

            Ambient         = 1<<0,
            Color           = 1<<1,
            Emissive        = 1<<2,
            Highlight       = 1<<3,
            Reflection      = 1<<4,
            Transparency    = 1<<5,
            Glass           = 1<<6,
            Fresnel         = 1<<7,
            CastShadows     = 1<<8,
        };

        String Name;

        ColorRGBAF AmbientColor;
        ColorRGBAF DiffuseColor;
        ColorRGBAF EmissiveColor;
        ColorRGBAF SpecularColor;

        Filename AlphaMap;
        Filename AmbientMap;
        Filename DiffuseMap;
        Filename DisplacementMap;
        Filename NormalMap;
        Filename SpecularColorMap;
        Filename SpecularPowerMap;

        u32 Mode;

        bool HasFlag(Flags flag) const { return flag == (Mode & flag); }
        void SetFlag(Flags flag) { Mode |= flag; }

        Material(String&& name) : Name(std::move(name)), Mode(Default) { Assert(Name.size()); }
    };

    struct SubPart {
        u32 FaceStart;
        u32 FaceCount;

        u16 Bone;
        u16 Material;

        SubPart() : FaceStart(UINT32_MAX), FaceCount(UINT32_MAX), Bone(UINT16_MAX), Material(UINT16_MAX) {}
    };

    ModelBuilder();
    ~ModelBuilder();

    const String& Name() const { return _name; }

    const VECTOR_THREAD_LOCAL(Mesh, float4)& Positions() const { return _positions; }
    const VECTOR_THREAD_LOCAL(Mesh, ColorRGBAF)& Colors() const { return _colors; }
    const VECTOR_THREAD_LOCAL(Mesh, float3)& Texcoords() const { return _texcoords; }
    const VECTOR_THREAD_LOCAL(Mesh, float3)& Normals() const { return _normals; }

    const VECTOR_THREAD_LOCAL(Mesh, Bone)& Bones() const { return _bones; }
    const VECTOR_THREAD_LOCAL(Mesh, Face)& Faces() const { return _faces; }
    const VECTOR_THREAD_LOCAL(Mesh, Group)& Groups() const { return _groups; }
    const VECTOR_THREAD_LOCAL(Mesh, Material)& Materials() const { return _materials; }
    const VECTOR_THREAD_LOCAL(Mesh, SubPart)& SubParts() const { return _subParts; }

    void SetName(const char *name) { SetName(String(name)); }
    void SetName(String&& name);

    void AddPosition(const float3& xyz);
    void AddPosition(const float4& xyzw);
    void AddColor(const ColorRGBAF& rgba);
    void AddTexCoord(const float2& uv);
    void AddTexCoord(const float3& uvw);
    void AddNormal(const float3& value);

    void AddTriangle(const PositionIndex (&pos)[3]);
    void AddTriangle(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3]);
    void AddTriangle(const PositionIndex (&pos)[3], const TexcoordIndex (&texcoords)[3], const NormalIndex (&normals)[3]);
    void AddTriangle(const PositionIndex (&pos)[3], const NormalIndex (&normals)[3]);

    void AddQuad(const PositionIndex (&pos)[4]);
    void AddQuad(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4]);
    void AddQuad(const PositionIndex (&pos)[4], const TexcoordIndex (&texcoords)[4], const NormalIndex (&normals)[4]);
    void AddQuad(const PositionIndex (&pos)[4], const NormalIndex (&normals)[4]);

    void AddBone(const char *name, const float4x4& transform) { AddBone(String(name), transform); }
    void AddBone(String&& name, const float4x4& transform);

    Group *OpenGroup(const char *name) { return OpenGroup(String(name)); }
    Group *OpenGroup(String&& name);
    const Group& OpenedGroup() const { Assert(_openGroup); return _groups.back(); }
    void CloseGroup(Group *group);

    Material *OpenMaterial(const char *name) { return OpenMaterial(String(name)); }
    Material *OpenMaterial(String&& name);
    const Material& OpenedMaterial() const { Assert(_openMaterial); return _materials.back(); }
    void CloseMaterial(Material *material);

    SubPart *OpenSubPart();
    const SubPart& OpenedSubPart() const { Assert(_openSubPart); Assert(_openGroup); return _subParts.back(); }
    void CloseSubPart(SubPart *material);

    PModel CreateModel();
    void Clear();

private:
    Group& OpenedGroup_() { Assert(_openGroup); return _groups.back(); }
    Material& OpenedMaterial_() { Assert(_openMaterial); return _materials.back(); }
    SubPart& OpenedSubPart_() { Assert(_openSubPart); Assert(_openGroup); return _subParts.back(); }

    String _name;

#ifdef WITH_CORE_ASSERT
    bool _openGroup     : 1;
    bool _openMaterial  : 1;
    bool _openSubPart   : 1;
#endif

    VECTOR_THREAD_LOCAL(Mesh, float4) _positions;
    VECTOR_THREAD_LOCAL(Mesh, ColorRGBAF) _colors;
    VECTOR_THREAD_LOCAL(Mesh, float3) _texcoords;
    VECTOR_THREAD_LOCAL(Mesh, float3) _normals;

    VECTOR_THREAD_LOCAL(Mesh, Bone) _bones;
    VECTOR_THREAD_LOCAL(Mesh, Face) _faces;
    VECTOR_THREAD_LOCAL(Mesh, Group) _groups;
    VECTOR_THREAD_LOCAL(Mesh, Material) _materials;
    VECTOR_THREAD_LOCAL(Mesh, SubPart) _subParts;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
