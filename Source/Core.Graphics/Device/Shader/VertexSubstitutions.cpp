#include "stdafx.h"

#include "VertexSubstitutions.h"

#include "Device/Geometry/VertexDeclaration.h"
#include "ShaderSource.h"

#include "Core/IO/Stream.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FillVertexSubstitutions(   VECTOR_THREAD_LOCAL(Shader, Pair<String COMMA String>)& defines,
                                const VertexDeclaration *declaration ) {
    Assert(declaration);

    defines.reserve(defines.size() + declaration->size() + 1 /* auto vertex definition */);

    STACKLOCAL_OCSTRSTREAM(oss, 1024);

    oss << "struct {";

    const ValueBlock::Field* pNormalKey = nullptr;
    const ValueBlock::Field* pTangentKey = nullptr;
    const ValueBlock::Field* pBinormalKey = nullptr;

    for (const ValueBlock::Field& subPart : declaration->SubParts()) {
        const size_t index = subPart.Index();
        const ValueType type = subPart.Type();
        const Graphics::Name& semantic = subPart.Name();

        if      (semantic == VertexSemantic::Normal)
            pNormalKey = &subPart;
        else if (semantic == VertexSemantic::Tangent)
            pTangentKey = &subPart;
        else if (semantic == VertexSemantic::Binormal)
            pBinormalKey = &subPart;

        const StringView nameWithoutIndex = semantic.MakeView();

        oss << " " << VertexFormatToShaderFormat(type)
            << " " << nameWithoutIndex << index
            << " : " << VertexSemanticToShaderSemantic(semantic) << index
            << ";";

        defines.emplace_back(
            StringFormat("AppIn_Get_{0}{1}(_AppIn)",
                nameWithoutIndex, index),
            StringFormat("AppIn_Get_{0}X_{2}((_AppIn).{0}{1})",
                nameWithoutIndex, index, ValueTypeToCStr(type)) );
    }

    // need tangent space without binormals ?
    if (pNormalKey && pTangentKey && !pBinormalKey) {
        // maybe binormal winding is packed in tangent.w ?
        if (ValueType::UX10Y10Z10W2N == pTangentKey->Type()) {
            Assert(pNormalKey->Index() == pTangentKey->Index());
            defines.emplace_back(
                StringFormat("AppIn_Get_Binormal{0}(_AppIn)",
                    pTangentKey->Index()),
                StringFormat("AppIn_Get_BinormalX_PackedInTangentW(AppIn_Get_Normal{0}(_AppIn), AppIn_Get_Tangent{1}(_AppIn), ((_AppIn).Tangent{1}).w)",
                    pNormalKey->Index(), pTangentKey->Index()) );
        }
    }

    oss << " }";

    defines.emplace_back(ToString(ShaderSource::AppIn_VertexDefinitionName()), oss.NullTerminatedStr());

    declaration->FillSubstitutions(defines); // custom substitutions
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView VertexFormatToShaderFormat(VertexFormat format) {
    switch (format)
    {
    case Core::Graphics::VertexFormat::Float:
        return MakeStringView("float");
    case Core::Graphics::VertexFormat::Float2:
        return MakeStringView("float2");
    case Core::Graphics::VertexFormat::Float3:
        return MakeStringView("float3");
    case Core::Graphics::VertexFormat::Float4:
        return MakeStringView("float4");
    case Core::Graphics::VertexFormat::Byte:
        return MakeStringView("byte");
    case Core::Graphics::VertexFormat::Byte2:
        return MakeStringView("byte2");
    case Core::Graphics::VertexFormat::Byte4:
        return MakeStringView("byte4");
    case Core::Graphics::VertexFormat::UByte:
        return MakeStringView("ubyte");
    case Core::Graphics::VertexFormat::UByte2:
        return MakeStringView("ubyte2");
    case Core::Graphics::VertexFormat::UByte4:
        return MakeStringView("ubyte4");
    case Core::Graphics::VertexFormat::Short:
        return MakeStringView("short");
    case Core::Graphics::VertexFormat::Short2:
        return MakeStringView("short2");
    case Core::Graphics::VertexFormat::Short4:
        return MakeStringView("short4");
    case Core::Graphics::VertexFormat::UShort:
        return MakeStringView("ushort");
    case Core::Graphics::VertexFormat::UShort2:
        return MakeStringView("ushort2");
    case Core::Graphics::VertexFormat::UShort4:
        return MakeStringView("ushort4");
    case Core::Graphics::VertexFormat::Word:
        return MakeStringView("word");
    case Core::Graphics::VertexFormat::Word2:
        return MakeStringView("word2");
    case Core::Graphics::VertexFormat::Word3:
        return MakeStringView("word3");
    case Core::Graphics::VertexFormat::Word4:
        return MakeStringView("word4");
    case Core::Graphics::VertexFormat::UWord:
        return MakeStringView("uword");
    case Core::Graphics::VertexFormat::UWord2:
        return MakeStringView("uword2");
    case Core::Graphics::VertexFormat::UWord3:
        return MakeStringView("uword3");
    case Core::Graphics::VertexFormat::UWord4:
        return MakeStringView("uword4");
    case Core::Graphics::VertexFormat::Half:
        return MakeStringView("half");
    case Core::Graphics::VertexFormat::Half2:
        return MakeStringView("half2");
    case Core::Graphics::VertexFormat::Half4:
        return MakeStringView("half4");
    case Core::Graphics::VertexFormat::Byte2N:
        return MakeStringView("float2");
    case Core::Graphics::VertexFormat::Byte4N:
        return MakeStringView("float4");
    case Core::Graphics::VertexFormat::UByte2N:
        return MakeStringView("float2");
    case Core::Graphics::VertexFormat::UByte4N:
        return MakeStringView("float4");
    case Core::Graphics::VertexFormat::Short2N:
        return MakeStringView("float2");
    case Core::Graphics::VertexFormat::Short4N:
        return MakeStringView("float4");
    case Core::Graphics::VertexFormat::UShort2N:
        return MakeStringView("float2");
    case Core::Graphics::VertexFormat::UShort4N:
        return MakeStringView("float4");
    case Core::Graphics::VertexFormat::UX10Y10Z10W2N:
        return MakeStringView("float4");
    default:
        AssertNotImplemented();
        break;
    }
    return StringView();
}
//----------------------------------------------------------------------------
StringView VertexSemanticToShaderSemantic(const Graphics::Name& semantic) {
    if      (semantic == VertexSemantic::Position)
        return MakeStringView("POSITION");
    else if (semantic == VertexSemantic::Position)
        return MakeStringView("TEXCOORD");
    else if (semantic == VertexSemantic::Color)
        return MakeStringView("COLOR");
    else if (semantic == VertexSemantic::Normal)
        return MakeStringView("NORMAL");
    else if (semantic == VertexSemantic::Tangent)
        return MakeStringView("TANGENT");
    else if (semantic == VertexSemantic::Binormal)
        return MakeStringView("BINORMAL");
    else {
        AssertNotImplemented();
        return StringView();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
