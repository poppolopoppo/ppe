// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Mesh/Format/PolygonFileFormat.h"

#include "Mesh/GenericMaterial.h"
#include "Mesh/GenericMesh.h"

#include "RHI/VertexDesc.h"

#include "HAL/PlatformEndian.h"

#include "Diagnostic/Logger.h"
#include "IO/BufferedStream.h"
#include "IO/ConstNames.h"
#include "IO/Filename.h"
#include "IO/FormatHelpers.h"
#include "IO/String.h"
#include "IO/StringView.h"
#include "IO/TextReader.h"
#include "Maths/ScalarVector.h"
#include "Memory/MemoryStream.h"
#include "Memory/SharedBuffer.h"
#include "Meta/AutoEnum.h"
#include "VirtualFileSystem.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const FExtname& FPolygonFileFormat::Extname() NOEXCEPT {
    return FFSConstNames::Ply();
}
//----------------------------------------------------------------------------
namespace PLY {
PPE_DEFINE_AUTOENUM(EFormat, u8,
    Ascii = 0,
    Binary_little_endian,
    Binary_big_endian);
PPE_DEFINE_AUTOENUM(EToken, u8,
    Format,
    Comment,
    Element,
    Property,
    End_header);
PPE_DEFINE_AUTOENUM(EElement, i8,
    Unknown = -1,
    Edge = 0,
    Face,
    Material,
    Vertex);
PPE_DEFINE_AUTOENUM(EProperty, u32,
    X                   = 1<<0,
    Y                   = 1<<1,
    Z                   = 1<<2,
    NX                  = 1<<3,
    NY                  = 1<<4,
    NZ                  = 1<<5,
    Red                 = 1<<6,
    Green               = 1<<7,
    Blue                = 1<<8,
    Alpha               = 1<<9,
    Material_Index      = 1<<10,
    Texcoord            = 1<<11,
    Texcoord_u          = 1<<12,
    Texcoord_v          = 1<<13,
    Texcoord_w          = 1<<14,
    Vertex_Indices      = 1<<15,
    Vertex1             = 1<<16,
    Vertex2             = 1<<17,
    Texture_u           = Texcoord_u,
    Texture_v           = Texcoord_v,
    Texture_w           = Texcoord_w );
ENUM_FLAGS(EProperty);
inline constexpr EProperty EProperty_Position{ EProperty::X | EProperty::Y | EProperty::Z };
inline constexpr EProperty EProperty_Normal{ EProperty::NX | EProperty::NY | EProperty::NZ };
inline constexpr EProperty EProperty_Color{ EProperty::Red | EProperty::Green | EProperty::Blue | EProperty::Alpha };
inline constexpr EProperty EProperty_Edge{ EProperty::Vertex1 | EProperty::Vertex2 };
inline constexpr EProperty EProperty_TexcoordUVW{ EProperty::Texcoord_u | EProperty::Texcoord_v | EProperty::Texcoord_w };
PPE_DEFINE_AUTOENUM(EType, i8,
    List = -1,
    Unknown = 0,
    Char,
    UChar,
    Short,
    UShort,
    Int,
    UInt,
    Float,
    Double)
inline constexpr size_t EType_MaxSizeInBytes = EVertexFormat_SizeOf(RHI::EVertexFormat::Double);
inline constexpr RHI::EVertexFormat EList_VertexFormat(EType value) {
    switch (value) {
    case EType::Char:
        return RHI::EVertexFormat::Byte;
    case EType::UChar:
        return RHI::EVertexFormat::UByte;
    case EType::Short:
        return RHI::EVertexFormat::Short;
    case EType::UShort:
        return RHI::EVertexFormat::UShort;
    case EType::Int:
        return RHI::EVertexFormat::Int;
    case EType::UInt:
        return RHI::EVertexFormat::UInt;

    case EType::Float: FALLTHROUGH();
    case EType::Double:
        break;

    case EType::List: FALLTHROUGH();
    case EType::Unknown:
        AssertNotReached();
    }
    return RHI::EVertexFormat::Unknown;
}
inline constexpr RHI::EVertexFormat EType_VertexFormat(EType value) {
    switch (value) {
    case EType::Char:
        return RHI::EVertexFormat::Byte_Norm;
    case EType::UChar:
        return RHI::EVertexFormat::UByte_Norm;
    case EType::Short:
        return RHI::EVertexFormat::Short_Norm;
    case EType::UShort:
        return RHI::EVertexFormat::UShort_Norm;
    case EType::Int:
        return RHI::EVertexFormat::Int;
    case EType::UInt:
        return RHI::EVertexFormat::UInt;
    case EType::Float:
        return RHI::EVertexFormat::Float;
    case EType::Double:
        return RHI::EVertexFormat::Double;

    case EType::List: FALLTHROUGH();
    case EType::Unknown:
        AssertNotReached();
    }
    return RHI::EVertexFormat::Unknown;
}
inline constexpr size_t EType_SizeOf(EType type) {
    switch (type) {
    case EType::Char: FALLTHROUGH();
    case EType::UChar:
        return 1;
    case EType::Short: FALLTHROUGH();
    case EType::UShort:
        return 2;
    case EType::Int: FALLTHROUGH();
    case EType::UInt: FALLTHROUGH();
    case EType::Float:
        return 4;
    case EType::Double:
        return 8;

    case EType::List: FALLTHROUGH();
    case EType::Unknown:
        AssertNotReached();
    }
    return 0;
}
inline void EType_SwapEndianness(EType type, const FRawMemory& value) NOEXCEPT {
    switch (type) {
    case EType::Char:
        FPlatformEndian::SwapInPlace(value.Cast<i8>().data());
        break;
    case EType::UChar:
        FPlatformEndian::SwapInPlace(value.Cast<u8>().data());
        break;
    case EType::Short:
        FPlatformEndian::SwapInPlace(value.Cast<i16>().data());
        break;
    case EType::UShort:
        FPlatformEndian::SwapInPlace(value.Cast<u16>().data());
        break;
    case EType::Int:
        FPlatformEndian::SwapInPlace(value.Cast<i32>().data());
        break;
    case EType::UInt:
        FPlatformEndian::SwapInPlace(value.Cast<u32>().data());
        break;
    case EType::Float:
        FPlatformEndian::SwapInPlace(value.Cast<float>().data());
        break;
    case EType::Double:
        FPlatformEndian::SwapInPlace(value.Cast<double>().data());
        break;

    case EType::List: FALLTHROUGH();
    case EType::Unknown:
        AssertNotReached();
    }
}
struct FProperty {
    RHI::FVertexFormatPromote FloatPromote{ nullptr };
    RHI::FVertexFormatPromote UIntPromote{ nullptr };
    RHI::FVertexFormatPromote ListPromote{ nullptr };

    EProperty Name{ Default };
    EType Type{ Default };
    EType List{ EType::Unknown };

    NODISCARD constexpr bool IsList() const {
        return (List != EType::Unknown);
    }
};
struct FElement {
    EElement Name{ Default };
    size_t Num{ 0 };
    VECTORMINSIZE(MeshBuilder, FProperty, 8) Properties;

    NODISCARD Meta::TOptionalReference<const FProperty> Find(EProperty name) const NOEXCEPT {
        for (const FProperty& it : Properties) {
            if (it.Name == name)
                return &it;
        }
        return nullptr;
    }

    NODISCARD bool Any(EProperty mask) const NOEXCEPT {
        for (const FProperty& it : Properties) {
            if (it.Name ^ mask)
                return true;
        }
        return false;
    }
};
struct FFormat {
    EFormat Format{ Default };
    bool (*Read)(IBufferedStreamReader& reader, const FRawMemory& dst, EType type);
    bool (*Write)(IBufferedStreamWriter& writer, EType type, const FRawMemoryConst& src);

    static constexpr FFormat Ascii() NOEXCEPT {
        return {
            EFormat::Ascii,
            [](IBufferedStreamReader& reader, const FRawMemory& dst, EType type) -> bool {
                //Assert_NoAssume(dst.SizeInBytes() == EType_SizeOf(type)); // too expansive!
                FTextReader text{ &reader };
                text.SkipSpaces();

                switch (type) {
                case EType::Char:
                    return (text >> dst.Cast<i8>().data());
                case EType::UChar:
                    return (text >> dst.Cast<u8>().data());
                case EType::Short:
                    return (text >> dst.Cast<i16>().data());
                case EType::UShort:
                    return (text >> dst.Cast<u16>().data());
                case EType::Int:
                    return (text >> dst.Cast<i32>().data());
                case EType::UInt:
                    return (text >> dst.Cast<u32>().data());
                case EType::Float:
                    return (text >> dst.Cast<float>().data());
                case EType::Double:
                    return (text >> dst.Cast<double>().data());

                case EType::List:
                case EType::Unknown:
                    AssertNotReached();
                }
                return false;
            },
            [](IBufferedStreamWriter& writer, EType type, const FRawMemoryConst& src) -> bool {
                //Assert_NoAssume(src.SizeInBytes() == EType_SizeOf(type)); // too expansive!
                FTextWriter text{ &writer };

                switch (type) {
                case EType::Char:
                    text << src.Cast<const i8>().data();
                    break;
                case EType::UChar:
                    text << src.Cast<const u8>().data();
                    break;
                case EType::Short:
                    text << src.Cast<const i16>().data();
                    break;
                case EType::UShort:
                    text << src.Cast<const u16>().data();
                    break;
                case EType::Int:
                    text << src.Cast<const i32>().data();
                    break;
                case EType::UInt:
                    text << src.Cast<const u32>().data();
                    break;
                case EType::Float:
                    text << src.Cast<const float>().data();
                    break;
                case EType::Double:
                    text << src.Cast<const double>().data();
                    break;

                case EType::List:
                case EType::Unknown:
                    AssertNotReached();
                }

                text << ' ';
                return true;
            },
        };
    }
    static constexpr FFormat Binary_little_endian() NOEXCEPT {
        STATIC_ASSERT(FPlatformEndian::Endianness == EEndianness::LittleEndian);
        return {
            EFormat::Binary_little_endian,
            [](IBufferedStreamReader& reader, const FRawMemory& dst, EType type) -> bool {
                //Assert_NoAssume(dst.SizeInBytes() == EType_SizeOf(type)); // too expansive!
                Unused(type);

                return reader.ReadView(dst);
            },
            [](IBufferedStreamWriter& writer, EType type, const FRawMemoryConst& src) -> bool {
                //Assert_NoAssume(src.SizeInBytes() == EType_SizeOf(type)); // too expansive!
                Unused(type);

                return writer.Write(src.data(), checked_cast<std::streamsize>(src.SizeInBytes()));
            },
        };
    }
    static constexpr FFormat Binary_big_endian() NOEXCEPT {
        STATIC_ASSERT(FPlatformEndian::Endianness == EEndianness::LittleEndian);
        return {
            EFormat::Binary_big_endian,
            [](IBufferedStreamReader& reader, const FRawMemory& dst, EType type) -> bool {
                //Assert_NoAssume(dst.SizeInBytes() == EType_SizeOf(type)); // too expansive!
                Unused(type);

                if (reader.ReadView(dst)) {
                    EType_SwapEndianness(type, dst);
                    return true;
                }
                return false;
            },
            [](IBufferedStreamWriter& writer, EType type, const FRawMemoryConst& src) -> bool {
                //Assert_NoAssume(src.SizeInBytes() == EType_SizeOf(type)); // too expansive!
                Unused(type);

                STACKLOCAL_POD_ARRAY(u8, tmp, src.SizeInBytes());
                src.CopyTo(tmp);
                EType_SwapEndianness(type, tmp);

                return writer.Write(tmp.data(), checked_cast<std::streamsize>(tmp.SizeInBytes()));
            },
        };
    }
};

struct FHeader {
    FFormat Format;
    FStringView Version;
    VECTORMINSIZE(MeshBuilder, FElement, 4) Elements;

    TPtrRef<const FElement> Get(EElement name) const {
        for (const FElement& it : Elements) {
            if (it.Name == name)
                return it;
        }
        return Default;
    }
};
} //!namespace PLY
//----------------------------------------------------------------------------
// Load
//----------------------------------------------------------------------------
bool FPolygonFileFormat::Load(FGenericMesh* dst, const FFilename& filename) {
    EAccessPolicy policy = EAccessPolicy::Binary;
    if (filename.Extname() == FFSConstNames::Z())
        policy = policy + EAccessPolicy::Compress;

    const FUniqueBuffer buf = VFS_ReadAll(filename, policy);
    PPE_LOG_CHECK(MeshBuilder, buf);

    return Load(dst, filename, buf.MakeView().Cast<const char>());
}
//----------------------------------------------------------------------------
bool FPolygonFileFormat::Load(FGenericMesh* dst, const FFilename& filename, const FStringView& content) {
    FMemoryViewReader reader{ content };
    return Load(dst, filename, reader);
}
//----------------------------------------------------------------------------
bool FPolygonFileFormat::Load(FGenericMesh* dst, const FFilename& filename, IBufferedStreamReader& input) {
    Assert(dst);
    dst->SetSourceFile(filename);

    PLY::FHeader header;

    FTextReader reader{ &input };
    PPE_LOG_CHECK(MeshBuilder, reader.ExpectWord("ply"_view));
    reader.SkipSpaces();

    bool headerEnd = false;
    while (not headerEnd and not reader.Eof()) {
        PLY::EToken token;
        PPE_LOG_CHECK(MeshBuilder, reader >> &token);

        switch (token) {
        case PLY::EToken::Format:
        {
            PLY::EFormat format;
            PPE_LOG_CHECK(MeshBuilder, reader >> &format);
            PPE_LOG_CHECK(MeshBuilder, reader.ExpectWord("1.0"_view));

            switch (format) {
            case PLY::EFormat::Ascii:
                header.Format = PLY::FFormat::Ascii();
                break;
            case PLY::EFormat::Binary_big_endian:
                header.Format = PLY::FFormat::Binary_big_endian();
                break;
            case PLY::EFormat::Binary_little_endian:
                header.Format = PLY::FFormat::Binary_little_endian();
                break;
            }
            break;
        }
        case PLY::EToken::Element:
        {
            PLY::FElement element;
            PPE_LOG_CHECK(MeshBuilder, reader >> &element.Name);
            PPE_LOG_CHECK(MeshBuilder, reader >> &element.Num);

            header.Elements.push_back(std::move(element));
            break;
        }
        case PLY::EToken::Property:
        {
            PPE_LOG_CHECK(MeshBuilder, not header.Elements.empty());

            PLY::FProperty property;
            PPE_LOG_CHECK(MeshBuilder, reader >> &property.Type);
            if (property.Type == PLY::EType::List) {
                PPE_LOG_CHECK(MeshBuilder, reader >> &property.List);
                PPE_LOG_CHECK(MeshBuilder, reader >> &property.Type);

                property.ListPromote = EVertexFormat_Promote(RHI::EVertexFormat::UInt, EList_VertexFormat(property.List));
                PPE_LOG_CHECK(MeshBuilder, !!property.ListPromote);
            }
            PPE_LOG_CHECK(MeshBuilder, reader >> &property.Name);

            property.FloatPromote = EVertexFormat_Promote(RHI::EVertexFormat::Float, EType_VertexFormat(property.Type));
            property.UIntPromote = EVertexFormat_Promote(RHI::EVertexFormat::UInt, EType_VertexFormat(property.Type));
            PPE_LOG_CHECK(MeshBuilder, !!property.UIntPromote || !!property.FloatPromote);

            header.Elements.back().Properties.push_back(std::move(property));
            break;
        }
        case PLY::EToken::Comment:
        {
            // ignore
            break;
        }
        case PLY::EToken::End_header:
            headerEnd = true;
            break;
        default:
            AssertNotReached();
        }

        PPE_LOG_CHECK(MeshBuilder, reader.ReadLine(nullptr));
    }
    PPE_LOG_CHECK(MeshBuilder, headerEnd);

    const TPtrRef<const PLY::FElement> face = header.Get(PLY::EElement::Face);
    const TPtrRef<const PLY::FElement> vertex = header.Get(PLY::EElement::Vertex);
    PPE_LOG_CHECK(MeshBuilder, vertex);

    FPositions3f positions0;
    if (vertex->Any(PLY::EProperty_Position))
        positions0 = dst->Position3f(0);

    FNormals3f normals0;
    if (vertex->Any(PLY::EProperty_Normal))
        normals0 = dst->Normal3f(0);

    FColors4f colors0;
    if (vertex->Any(PLY::EProperty_Color))
        colors0 = dst->Color4f(0);

    FTexcoords2f texcoords2;
    FTexcoords3f texcoords3;
    if (face && face->Find(PLY::EProperty::Texcoord)) {
        texcoords2 = dst->Texcoord2f(0); // assuming texcoord 2f when UV are defined by face instead of by vertex
    }
    else if (vertex->Any(PLY::EProperty_TexcoordUVW)) {
        if (vertex->Any(PLY::EProperty::Texcoord_w)) {
            texcoords3 = dst->Texcoord3f(0);
        } else {
            texcoords2 = dst->Texcoord2f(0);
        }
    }

    size_t indexCount{0}, vertexCount{0};
    if (face)
        indexCount = face->Num * 3; // #TODO: remove abusive triangle assumption
    if (vertex)
        vertexCount = vertex->Num;

    dst->Resize(indexCount, vertexCount, false);

    for (const PLY::FElement& element : header.Elements) {
        switch (element.Name) {
        case PLY::EElement::Face:
        {
            u32 index = 0;
            u8 raw[PLY::EType_MaxSizeInBytes];

            forrange(f, 0, element.Num) {
                for (const PLY::FProperty& property : element.Properties) {
                    PPE_LOG_CHECK(MeshBuilder, property.IsList());

                    FRawMemory tmp = MakeView(raw).CutBefore(EType_SizeOf(property.List));
                    PPE_LOG_CHECK(MeshBuilder, header.Format.Read(input, tmp, property.List));

                    u32 num = 0;
                    PPE_LOG_CHECK(MeshBuilder, property.ListPromote(MakePodView(num), tmp));

                    tmp = MakeView(raw).CutBefore(EType_SizeOf(property.Type));

                    forrange(n, 0, num) {
                        PPE_LOG_CHECK(MeshBuilder, header.Format.Read(input, tmp, property.Type));

                        switch (property.Name) {
                        case PLY::EProperty::Vertex_Indices:
                        {
                            PPE_LOG_CHECK(MeshBuilder, property.UIntPromote(MakePodView(dst->Indices()[index++]), tmp));
                            break;
                        }
                        case PLY::EProperty::Texcoord:
                        {
                            const u32 vertexIndex = dst->Indices()[index];

                            TPtrRef<float> pValue;
                            if (texcoords3) {
                                pValue = texcoords3[vertexIndex][num % 3];
                                if (num && ((num + 1) % 3) == 0)
                                    index++;
                            }
                            else if (texcoords2) {
                                pValue = texcoords3[vertexIndex][num % 2];
                                if (num && ((num + 1) % 2) == 0)
                                    index++;
                            }
                            else {
                                PPE_SLOG(MeshBuilder, Error, "unexpected texcoord property", {
                                    {"Element", Opaq::Format(element.Name)},
                                    {"Property", Opaq::Format(property.Name)},
                                    {"List", Opaq::Format(property.List)},
                                    {"Type", Opaq::Format(property.Type)},
                                    {"SourceFile", Opaq::Format(filename)},
                                });
                                return false;
                            }

                            PPE_LOG_CHECK(MeshBuilder, property.UIntPromote(MakePodView(pValue), tmp));
                            break;
                        }
                        default:
                            PPE_SLOG(MeshBuilder, Error, "unsupported property in face element", {
                                {"Element", Opaq::Format(element.Name)},
                                {"Property", Opaq::Format(property.Name)},
                                {"List", Opaq::Format(property.List)},
                                {"Type", Opaq::Format(property.Type)},
                                {"SourceFile", Opaq::Format(filename)},
                            });
                            return false;
                        }
                    }
                }
            }
            break;
        }
        case PLY::EElement::Vertex:
        {
            u8 raw[PLY::EType_MaxSizeInBytes];

            forrange(v, 0, element.Num) {
                for (const PLY::FProperty& property : element.Properties) {
                    PPE_LOG_CHECK(MeshBuilder, not property.IsList());

                    FRawMemory dst;
                    switch (property.Name) {
                    // position
                    case PLY::EProperty::X:
                        dst = MakePodView(positions0[v].x);
                        break;
                    case PLY::EProperty::Y:
                        dst = MakePodView(positions0[v].y);
                        break;
                    case PLY::EProperty::Z:
                        dst = MakePodView(positions0[v].z);
                        break;
                    // normal
                    case PLY::EProperty::NX:
                        dst = MakePodView(normals0[v].x);
                        break;
                    case PLY::EProperty::NY:
                        dst = MakePodView(normals0[v].y);
                        break;
                    case PLY::EProperty::NZ:
                        dst = MakePodView(normals0[v].z);
                        break;
                    // texcoord
                    case PLY::EProperty::Texcoord_u:
                        dst = MakePodView(texcoords3 ? texcoords3[v].x : texcoords2[v].x);
                        break;
                    case PLY::EProperty::Texcoord_v:
                        dst = MakePodView(texcoords3 ? texcoords3[v].y : texcoords2[v].y);
                        break;
                    case PLY::EProperty::Texcoord_w:
                        dst = MakePodView(texcoords3[v].z);
                        break;
                    // color
                    case PLY::EProperty::Red:
                        dst = MakePodView(colors0[v].x);
                        break;
                    case PLY::EProperty::Green:
                        dst = MakePodView(colors0[v].y);
                        break;
                    case PLY::EProperty::Blue:
                        dst = MakePodView(colors0[v].z);
                        break;
                    case PLY::EProperty::Alpha:
                        dst = MakePodView(colors0[v].w);
                        break;

                    default:
                        PPE_SLOG(MeshBuilder, Error, "unsupported property in vertex element", {
                            {"Element", Opaq::Format(element.Name)},
                            {"Property", Opaq::Format(property.Name)},
                            {"List", Opaq::Format(property.List)},
                            {"Type", Opaq::Format(property.List)},
                            {"SourceFile", Opaq::Format(filename)},
                        });
                        return false;
                    }

                    if (property.Type == PLY::EType::Float) {
                        // direct copy
                        PPE_LOG_CHECK(MeshBuilder, header.Format.Read(input, dst, property.Type));
                    }
                    else {
                        // need intermediate copy and promotion
                        FRawMemory tmp = MakeView(raw).CutBefore(EType_SizeOf(property.Type));
                        PPE_LOG_CHECK(MeshBuilder, header.Format.Read(input, tmp, property.Type));
                        PPE_LOG_CHECK(MeshBuilder, property.FloatPromote(dst, tmp));
                    }
                }
            }
            break;
        }
        case PLY::EElement::Edge: FALLTHROUGH();
        case PLY::EElement::Material:
        {
            PPE_SLOG(MeshBuilder, Error, "unsupported PLY element", {
                {"Element", Opaq::Format(element.Name)},
                {"SourceFile", Opaq::Format(filename)},
            });
            return false;
        }
        default:
            AssertNotReached();
        }
    }

    return true;
}
//----------------------------------------------------------------------------
// Save
//----------------------------------------------------------------------------
bool FPolygonFileFormat::Save(const FGenericMesh& src, const FFilename& filename) {
    MEMORYSTREAM(MeshBuilder) writer;
    if (false == Save(src, filename, writer))
        return false;

    EAccessPolicy policy = EAccessPolicy::Truncate_Binary;
    if (filename.Extname() == FFSConstNames::Z())
        policy = policy + EAccessPolicy::Compress;

    return VFS_WriteAll(filename, writer.MakeView(), policy);
}
//----------------------------------------------------------------------------
bool FPolygonFileFormat::Save(const FGenericMesh& src, const FFilename& filename, IBufferedStreamWriter& writer) {
    const FPositions3f positions0 = src.Position3f_IFP(0);
    const FNormals3f normals0 = src.Normal3f_IFP(0);
    const FColors4f colors0 = src.Color4f_IFP(0);
    const FTexcoords2f texcoords2 = src.Texcoord2f_IFP(0);
    const FTexcoords3f texcoords3 = src.Texcoord3f_IFP(0);

    PPE_LOG_CHECK(MeshBuilder, !texcoords2 ^ !texcoords3);

    const PLY::FFormat format = PLY::FFormat::Binary_little_endian();

    FTextWriter header{ &writer };
    header << "ply" << Eol
        << "comment Generated by PPE/MeshBuilder/PolygonFileFormat" << Eol
        << "comment Source file: " << filename << Eol
        << "format " << format.Format << " 1.0" << Eol
        << "element vertex " << src.VertexCount() << Eol
        << Fmt::Formator<char>([&](FTextWriter& oss) {
            if (positions0) {
                oss << "property " << PLY::EType::Float << " " << PLY::EProperty::X << Eol
                    << "property " << PLY::EType::Float << " " << PLY::EProperty::Y << Eol
                    << "property " << PLY::EType::Float << " " << PLY::EProperty::Z << Eol;
            }
            if (normals0) {
                oss << "property " << PLY::EType::Float << " " << PLY::EProperty::NX << Eol
                    << "property " << PLY::EType::Float << " " << PLY::EProperty::NY << Eol
                    << "property " << PLY::EType::Float << " " << PLY::EProperty::NZ << Eol;
            }
            if (colors0) {
                oss << "property " << PLY::EType::Float << " " << PLY::EProperty::Red << Eol
                    << "property " << PLY::EType::Float << " " << PLY::EProperty::Green << Eol
                    << "property " << PLY::EType::Float << " " << PLY::EProperty::Blue << Eol
                    << "property " << PLY::EType::Float << " " << PLY::EProperty::Alpha << Eol;
            }
            if (texcoords2 || texcoords3) {
                oss << "property " << PLY::EType::Float << " " << PLY::EProperty::Texcoord_u << Eol
                    << "property " << PLY::EType::Float << " " << PLY::EProperty::Texcoord_v << Eol;
                if (texcoords3)
                    oss << "property " << PLY::EType::Float << " " << PLY::EProperty::Texcoord_w << Eol;
            }
        })
        << "element face " << src.TriangleCount() << Eol
        << "property list uchar uint vertex_indices" << Eol
        << "end_header" << Eol;

    forrange(v, 0, src.VertexCount()) {
        bool written = true;
        if (positions0) {
            written &= format.Write(writer, PLY::EType::Float, MakePodView(positions0[v].x));
            written &= format.Write(writer, PLY::EType::Float, MakePodView(positions0[v].y));
            written &= format.Write(writer, PLY::EType::Float, MakePodView(positions0[v].z));
        }
        if (normals0) {
            written &= format.Write(writer, PLY::EType::Float, MakePodView(normals0[v].x));
            written &= format.Write(writer, PLY::EType::Float, MakePodView(normals0[v].y));
            written &= format.Write(writer, PLY::EType::Float, MakePodView(normals0[v].z));
        }
        if (colors0) {
            written &= format.Write(writer, PLY::EType::Float, MakePodView(colors0[v].x));
            written &= format.Write(writer, PLY::EType::Float, MakePodView(colors0[v].y));
            written &= format.Write(writer, PLY::EType::Float, MakePodView(colors0[v].z));
            written &= format.Write(writer, PLY::EType::Float, MakePodView(colors0[v].w));
        }
        if (texcoords2) {
            written &= format.Write(writer, PLY::EType::Float, MakePodView(texcoords2[v].x));
            written &= format.Write(writer, PLY::EType::Float, MakePodView(texcoords2[v].y));
        }
        if (texcoords3) {
            written &= format.Write(writer, PLY::EType::Float, MakePodView(texcoords3[v].x));
            written &= format.Write(writer, PLY::EType::Float, MakePodView(texcoords3[v].y));
            written &= format.Write(writer, PLY::EType::Float, MakePodView(texcoords3[v].z));
        }
        PPE_LOG_CHECK(MeshBuilder, !!written);
    }

    forrange(t, 0, src.TriangleCount()) {
        const uint3 vertices = src.Triangle(t);

        bool written = true;
        written &= format.Write(writer, PLY::EType::UChar, MakePodConstView(3_u8));
        written &= format.Write(writer, PLY::EType::UInt, MakePodConstView(vertices.x));
        written &= format.Write(writer, PLY::EType::UInt, MakePodConstView(vertices.y));
        written &= format.Write(writer, PLY::EType::UInt, MakePodConstView(vertices.z));

        PPE_LOG_CHECK(MeshBuilder, !!written);
    }

    return true;
}
//----------------------------------------------------------------------------
// IMeshFormat
//----------------------------------------------------------------------------
bool FPolygonFileFormat::ExportGenericMesh(IStreamWriter* output, const FGenericMesh& mesh) const {
    const FFilename sourceFile = mesh.SourceFile().value_or(Default);
    return UsingBufferedStream(output, [&](TPtrRef<IBufferedStreamWriter> buf) {
        return Save(mesh, sourceFile, buf);
    });
}
//----------------------------------------------------------------------------
FMeshBuilderResult FPolygonFileFormat::ImportGenericMesh(const FRawMemoryConst& memory) const {
    FMemoryViewReader reader{ memory };
    return ImportGenericMesh(reader);
}
//----------------------------------------------------------------------------
FMeshBuilderResult FPolygonFileFormat::ImportGenericMesh(IStreamReader& input) const {
    return UsingBufferedStream(&input, [](TPtrRef<IBufferedStreamReader> reader) -> FMeshBuilderResult {
        FGenericMesh result;
        if (Load(&result, Default, reader))
            return Meta::MakeOptional(std::move(result));

        return std::nullopt;
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
