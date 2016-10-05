#include "stdafx.h"

#include "MeshLoader.h"

#include "Core.Graphics/Device/DeviceAPI.h"

#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/IO/VFS/VirtualFileSystemStream.h"
#include "Core/IO/VirtualFileSystem.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace PLY {
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
enum class EToken : u32 {
    Ply = 0,
    Format,
    Comment,
    FElement,
    Vertex,
    VertexIndices,
    EFace,
    EProperty,
    TList,
    EndHeader,

    Char,
    UChar,
    Short,
    UShort,
    Int,
    UInt,
    Float,
    Double,

    Ascii,
    BinaryLittleEndian,

    X,
    Y,
    Z,

    NX,
    NY,
    NZ,

    Red,
    Green,
    Blue,
    Alpha,

    TextureU,
    TextureV,
};
//----------------------------------------------------------------------------
static const char *gTokenToCStr[] = {
    "ply",
    "format",
    "comment",
    "element",
    "vertex",
    "vertex_indices",
    "face",
    "property",
    "list",
    "end_header",

    "char",
    "uchar",
    "short",
    "ushort",
    "int",
    "uint",
    "float",
    "double",

    "ascii",
    "binary_little_endian",

    "x",
    "y",
    "z",

    "nx",
    "ny",
    "nz",

    "red",
    "green",
    "blue",
    "alpha",

    "texture_u",
    "texture_v",
};
//----------------------------------------------------------------------------
enum class EProperty : u32 {
    PositionX   = 1<<0,
    PositionY   = 1<<1,
    PositionZ   = 1<<2,

    NormalX     = 1<<3,
    NormalY     = 1<<4,
    NormalZ     = 1<<5,

    ColorR      = 1<<6,
    ColorG      = 1<<7,
    ColorB      = 1<<8,
    ColorA      = 1<<9,

    TextureU    = 1<<10,
    TextureV    = 1<<11,

    PositionXYZ = PositionX|PositionY|PositionZ,
    NormalXYZ   = NormalX|NormalY|NormalZ,
    ColorRGBA   = ColorR|ColorG|ColorB|ColorA,
    TextureUV   = TextureU|TextureV,

    Position_Normal_Color = PositionXYZ|NormalXYZ|ColorRGBA,
    Position_Normal_Color_UV = PositionXYZ|NormalXYZ|ColorRGBA|TextureUV,
};
//----------------------------------------------------------------------------
static bool ScalarTypeSizeInBytes(u32 *size, EToken type) {
    Assert(size);
    switch (type)
    {
    case EToken::Char:
        *size = 1;
        return true;
    case EToken::UChar:
        *size = 1;
        return true;
    case EToken::Short:
        *size = 2;
        return true;
    case EToken::UShort:
        *size = 2;
        return true;
    case EToken::Int:
        *size = 4;
        return true;
    case EToken::UInt:
        *size = 4;
        return true;
    case EToken::Float:
        *size = 4;
        return true;
    case EToken::Double:
        *size = 8;
        return true;

    default:
        break;
    }
    return false;
}
//----------------------------------------------------------------------------
static bool ReadToken(EToken *token, IVirtualFileSystemIStream *stream) {
    Assert(token);
    Assert(stream);

    const std::streamoff offsetI = stream->TellI();

    char cstr[32];
    const size_t length = checked_cast<size_t>(stream->ReadWord(cstr));
    if (0 == length || length + 1 >= lengthof(cstr))
        return false;

    for (size_t i = 0; i + 1 < length; ++i)
        cstr[i] = ToLower(cstr[i]);

    for (size_t i = 0; i < lengthof(gTokenToCStr); ++i)
        if (0 == CompareNI(cstr, gTokenToCStr[i], length - 1)) {
            *token = EToken(i);
            return true;
        }

    stream->SeekI(offsetI);
    return false;
}
//----------------------------------------------------------------------------
static bool ReadToken_SkipCommentIFN(EToken *token, IVirtualFileSystemIStream *stream) {
    if (!ReadToken(token, stream))
        return false;

    if (*token != EToken::Comment)
        return true;

    if (!stream->SeekI_FirstOf('\n')) // skip the rest of the line
        return false;

    return ReadToken_SkipCommentIFN(token, stream);
}
//----------------------------------------------------------------------------
static bool ReadInt(u32 *value, IVirtualFileSystemIStream *stream) {
    char cstr[80];
    const size_t length = checked_cast<size_t>(stream->ReadWord(cstr));

    if (0 == length || lengthof(cstr) == length + 1)
        return false;

    if (!Atoi<10>(value, cstr, length - 1))
        return false;

    return true;
}
//----------------------------------------------------------------------------
struct FVertex_Position_Normal_Color {
    float3 Position;
    float3 Normal;
    ColorRGBA Color;
};
STATIC_ASSERT(sizeof(FVertex_Position_Normal_Color) == 3*4+3*4+4);
//----------------------------------------------------------------------------
struct FVertex_Position_Normal_Color_UV {
    float3 Position;
    float3 Normal;
    ColorRGBA Color;
    float2 UV;
};
STATIC_ASSERT(sizeof(FVertex_Position_Normal_Color_UV) == 3*4+3*4+4+2*4);
//----------------------------------------------------------------------------
struct FFace_Triangle {
    u32 A;
    u32 B;
    u32 C;
};
STATIC_ASSERT(sizeof(FFace_Triangle) == 3*4);
//----------------------------------------------------------------------------
static void SanitizeNormal3_(float3& normal) {
    const float length = Length3(normal);
    normal = (std::abs(length) < F_Epsilon)
         ? float3(0, 1, 0)
         : normal / length;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
bool ReadMeshHeader(FMeshHeader& header, IVirtualFileSystemIStream *stream) {
    Assert(stream);

    EToken token;

    if (!ReadToken(&token, stream) || EToken::Ply != token)
        return false;

    if (!ReadToken_SkipCommentIFN(&token, stream) || EToken::Format != token)
        return false;
    if (!ReadToken(&token, stream) || EToken::BinaryLittleEndian != token)
        return false;
    if (!stream->SeekI_FirstOf('\n'))
        return false;

    if (!ReadToken_SkipCommentIFN(&token, stream) || EToken::FElement != token)
        return false;
    if (!ReadToken(&token, stream) || EToken::Vertex != token)
        return false;
    if (!ReadInt(&header.VertexCount, stream))
        return false;

    header.VertexStride = 0;
    EProperty vertexProperties = EProperty(0);
    for (;;) {
        if (!ReadToken_SkipCommentIFN(&token, stream))
            return false;

        if (EToken::EProperty != token) {
            Assert(EToken::FElement == token);
            break;
        }

        EToken propertyType;
        u32 propertySize;
        if (!ReadToken(&propertyType, stream) || !ScalarTypeSizeInBytes(&propertySize, propertyType))
            return false;

        if (!ReadToken(&token, stream))
            return false;

        EProperty propertySemantic;
        switch (token)
        {
        case EToken::X:
            AssertRelease(propertyType == EToken::Float);
            propertySemantic = EProperty::PositionX;
            break;
        case EToken::Y:
            AssertRelease(propertyType == EToken::Float);
            propertySemantic = EProperty::PositionY;
            break;
        case EToken::Z:
            AssertRelease(propertyType == EToken::Float);
            propertySemantic = EProperty::PositionZ;
            break;
        case EToken::NX:
            AssertRelease(propertyType == EToken::Float);
            propertySemantic = EProperty::NormalX;
            break;
        case EToken::NY:
            AssertRelease(propertyType == EToken::Float);
            propertySemantic = EProperty::NormalY;
            break;
        case EToken::NZ:
            AssertRelease(propertyType == EToken::Float);
            propertySemantic = EProperty::NormalZ;
            break;
        case EToken::Red:
            AssertRelease(propertyType == EToken::UChar);
            propertySemantic = EProperty::ColorR;
            break;
        case EToken::Green:
            AssertRelease(propertyType == EToken::UChar);
            propertySemantic = EProperty::ColorG;
            break;
        case EToken::Blue:
            AssertRelease(propertyType == EToken::UChar);
            propertySemantic = EProperty::ColorB;
            break;
        case EToken::Alpha:
            AssertRelease(propertyType == EToken::UChar);
            propertySemantic = EProperty::ColorA;
            break;
        case EToken::TextureU:
            AssertRelease(propertyType == EToken::Float);
            propertySemantic = EProperty::TextureU;
            break;
        case EToken::TextureV:
            AssertRelease(propertyType == EToken::Float);
            propertySemantic = EProperty::TextureV;
            break;
        default:
            return false;
        }

        header.VertexStride += propertySize;
        vertexProperties = EProperty(u32(vertexProperties)|u32(propertySemantic));
    }

    AssertRelease( (sizeof(FVertex_Position_Normal_Color) == header.VertexStride &&
                        EProperty::Position_Normal_Color == vertexProperties) ||
                   (sizeof(FVertex_Position_Normal_Color_UV) == header.VertexStride &&
                        EProperty::Position_Normal_Color_UV == vertexProperties ));

    if (EToken::FElement != token)
        return false;
    if (!ReadToken(&token, stream) || EToken::EFace != token)
        return false;
    u32 faceCount;
    if (!ReadInt(&faceCount, stream))
        return false;

    header.IndexCount = faceCount * 3 /* assuming triangles */;
    header.IndexStride = u32(Graphics::IndexElementSize::ThirtyTwoBits);

    if (!ReadToken_SkipCommentIFN(&token, stream) || EToken::EProperty != token)
        return false;
    if (!ReadToken(&token, stream) || EToken::TList != token)
        return false;
    if (!ReadToken(&token, stream) || EToken::UChar != token)
        return false;
    if (!ReadToken(&token, stream) || EToken::Int != token)
        return false;
    if (!ReadToken(&token, stream) || EToken::VertexIndices != token)
        return false;

    if (!ReadToken_SkipCommentIFN(&token, stream) || EToken::EndHeader != token)
        return false;

    return true;
}
//----------------------------------------------------------------------------
bool ReadMeshData(const FMeshHeader& header, const TMemoryView<u8>& indices, FGenericVertex& vertices, IVirtualFileSystemIStream *stream) {
    Assert(stream);
    Assert(indices.SizeInBytes() == header.IndexCount * header.IndexStride);
    Assert(vertices.VertexCountRemaining() == header.VertexCount);
    Assert( sizeof(FVertex_Position_Normal_Color) == header.VertexStride ||
            sizeof(FVertex_Position_Normal_Color_UV) == header.VertexStride );

    const FGenericVertex::FSubPart sp_positions0 = vertices.Position3f(0);
    const FGenericVertex::FSubPart sp_colors0 = vertices.Color4b(0);
    const FGenericVertex::FSubPart sp_normals0 = vertices.Normal3f(0);

    Assert(sp_positions0);
    Assert(sp_colors0);
    Assert(sp_normals0);

    if (sizeof(FVertex_Position_Normal_Color) == header.VertexStride) {
        FVertex_Position_Normal_Color vertexData;
        for (size_t i = 0; i < header.VertexCount; ++i) {
            if (!stream->ReadPOD(&vertexData))
                return false;

            SanitizeNormal3_(vertexData.Normal); // beurk

            sp_positions0.WriteValue(vertices, vertexData.Position);
            sp_colors0.WriteValue(vertices, vertexData.Color);
            sp_normals0.WriteValue(vertices, vertexData.Normal);

            if (!vertices.NextVertex())
                Assert(i + 1 == header.VertexCount);
        }
    }
    else {
        const FGenericVertex::FSubPart sp_texcoord0 = vertices.TexCoord2f(0);
        Assert(sp_texcoord0);

        FVertex_Position_Normal_Color_UV vertexData;
        for (size_t i = 0; i < header.VertexCount; ++i) {
            if (!stream->ReadPOD(&vertexData))
                return false;

            SanitizeNormal3_(vertexData.Normal); // beurk

            vertexData.UV.y() = 1 - vertexData.UV.y(); // double-beurk

            sp_positions0.WriteValue(vertices, vertexData.Position);
            sp_colors0.WriteValue(vertices, vertexData.Color);
            sp_texcoord0.WriteValue(vertices, vertexData.UV);
            sp_normals0.WriteValue(vertices, vertexData.Normal);

            if (!vertices.NextVertex())
                Assert(i + 1 == header.VertexCount);
        }
    }

    Assert(header.IndexStride == u32(Graphics::IndexElementSize::ThirtyTwoBits));
    const TMemoryView<u32> indicesU32 = indices.Cast<u32>();
    Assert(indicesU32.size() == header.IndexCount);

    FFace_Triangle faceData;

    for (size_t i = 0; i < header.IndexCount; i += 3) {
        u8 faceCount;
        if (!stream->ReadPOD(&faceCount))
            return false;

        Assert(3 == faceCount); // assuming triangle topology

        if (!stream->ReadPOD(&faceData))
            return false;

        Assert(faceData.A < header.VertexCount);
        Assert(faceData.B < header.VertexCount);
        Assert(faceData.C < header.VertexCount);

        indicesU32[i + 0] = faceData.A;
        indicesU32[i + 1] = faceData.B;
        indicesU32[i + 2] = faceData.C;
    }

    return true;
}
//----------------------------------------------------------------------------
} //!namespace PLY
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
