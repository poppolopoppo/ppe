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

        const StringSlice nameWithoutIndex = semantic.MakeView();

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
StringSlice VertexFormatToShaderFormat(VertexFormat format) {
    switch (format)
    {
    case Core::Graphics::VertexFormat::Float:
        return MakeStringSlice("float");
    case Core::Graphics::VertexFormat::Float2:
        return MakeStringSlice("float2");
    case Core::Graphics::VertexFormat::Float3:
        return MakeStringSlice("float3");
    case Core::Graphics::VertexFormat::Float4:
        return MakeStringSlice("float4");
    case Core::Graphics::VertexFormat::Byte:
        return MakeStringSlice("byte");
    case Core::Graphics::VertexFormat::Byte2:
        return MakeStringSlice("byte2");
    case Core::Graphics::VertexFormat::Byte4:
        return MakeStringSlice("byte4");
    case Core::Graphics::VertexFormat::UByte:
        return MakeStringSlice("ubyte");
    case Core::Graphics::VertexFormat::UByte2:
        return MakeStringSlice("ubyte2");
    case Core::Graphics::VertexFormat::UByte4:
        return MakeStringSlice("ubyte4");
    case Core::Graphics::VertexFormat::Short:
        return MakeStringSlice("short");
    case Core::Graphics::VertexFormat::Short2:
        return MakeStringSlice("short2");
    case Core::Graphics::VertexFormat::Short4:
        return MakeStringSlice("short4");
    case Core::Graphics::VertexFormat::UShort:
        return MakeStringSlice("ushort");
    case Core::Graphics::VertexFormat::UShort2:
        return MakeStringSlice("ushort2");
    case Core::Graphics::VertexFormat::UShort4:
        return MakeStringSlice("ushort4");
    case Core::Graphics::VertexFormat::Word:
        return MakeStringSlice("word");
    case Core::Graphics::VertexFormat::Word2:
        return MakeStringSlice("word2");
    case Core::Graphics::VertexFormat::Word3:
        return MakeStringSlice("word3");
    case Core::Graphics::VertexFormat::Word4:
        return MakeStringSlice("word4");
    case Core::Graphics::VertexFormat::UWord:
        return MakeStringSlice("uword");
    case Core::Graphics::VertexFormat::UWord2:
        return MakeStringSlice("uword2");
    case Core::Graphics::VertexFormat::UWord3:
        return MakeStringSlice("uword3");
    case Core::Graphics::VertexFormat::UWord4:
        return MakeStringSlice("uword4");
    case Core::Graphics::VertexFormat::Half:
        return MakeStringSlice("half");
    case Core::Graphics::VertexFormat::Half2:
        return MakeStringSlice("half2");
    case Core::Graphics::VertexFormat::Half4:
        return MakeStringSlice("half4");
    case Core::Graphics::VertexFormat::Byte2N:
        return MakeStringSlice("float2");
    case Core::Graphics::VertexFormat::Byte4N:
        return MakeStringSlice("float4");
    case Core::Graphics::VertexFormat::UByte2N:
        return MakeStringSlice("float2");
    case Core::Graphics::VertexFormat::UByte4N:
        return MakeStringSlice("float4");
    case Core::Graphics::VertexFormat::Short2N:
        return MakeStringSlice("float2");
    case Core::Graphics::VertexFormat::Short4N:
        return MakeStringSlice("float4");
    case Core::Graphics::VertexFormat::UShort2N:
        return MakeStringSlice("float2");
    case Core::Graphics::VertexFormat::UShort4N:
        return MakeStringSlice("float4");
    case Core::Graphics::VertexFormat::UX10Y10Z10W2N:
        return MakeStringSlice("float4");
    default:
        AssertNotImplemented();
        break;
    }
    return StringSlice();
}
//----------------------------------------------------------------------------
StringSlice VertexSemanticToShaderSemantic(const Graphics::Name& semantic) {
    if      (semantic == VertexSemantic::Position)
        return MakeStringSlice("POSITION");
    else if (semantic == VertexSemantic::Position)
        return MakeStringSlice("TEXCOORD");
    else if (semantic == VertexSemantic::Color)
        return MakeStringSlice("COLOR");
    else if (semantic == VertexSemantic::Normal)
        return MakeStringSlice("NORMAL");
    else if (semantic == VertexSemantic::Tangent)
        return MakeStringSlice("TANGENT");
    else if (semantic == VertexSemantic::Binormal)
        return MakeStringSlice("BINORMAL");
    else {
        AssertNotImplemented();
        return StringSlice();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
