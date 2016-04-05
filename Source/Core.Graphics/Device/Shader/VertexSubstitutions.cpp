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

    const MemoryView<const Pair<VertexSubPartKey, VertexSubPartPOD>> subParts = declaration->SubParts();

    defines.reserve(defines.size() + subParts.size() + 1 /* auto vertex definition */);

    STACKLOCAL_OCSTRSTREAM(oss, 1024);

    oss << "struct {";

    const VertexSubPartKey *pNormalKey = nullptr;
    const VertexSubPartKey *pTangentKey = nullptr;
    const VertexSubPartKey *pBinormalKey = nullptr;

    for (const Pair<VertexSubPartKey, VertexSubPartPOD>& keyedSubPart : declaration->SubParts()) {
        const size_t index = keyedSubPart.first.Index();
        const VertexSubPartFormat format = keyedSubPart.first.Format();
        const VertexSubPartSemantic semantic = keyedSubPart.first.Semantic();

        switch (semantic)
        {
        case VertexSubPartSemantic::Normal: pNormalKey = &keyedSubPart.first; break;
        case VertexSubPartSemantic::Tangent: pTangentKey = &keyedSubPart.first; break;
        case VertexSubPartSemantic::Binormal: pBinormalKey = &keyedSubPart.first; break;
        default: break;
        }

        const StringSlice nameWithoutIndex = VertexSubPartSemanticToCStr(semantic);
        const StringSlice formatCStr = VertexSubPartFormatToCStr(format);

        oss << " " << VertexSubPartFormatToShaderFormat(format)
            << " " << nameWithoutIndex << index
            << " : " << VertexSubPartSemanticToShaderSemantic(semantic) << index
            << ";";

        defines.emplace_back(
            StringFormat("AppIn_Get_{0}{1}(_AppIn)", nameWithoutIndex, index),
            StringFormat("AppIn_Get_{0}X_{2}((_AppIn).{0}{1})", nameWithoutIndex, index, formatCStr) );
    }

    // need tangent space without binormals ?
    if (pNormalKey && pTangentKey && !pBinormalKey) {
        // maybe binormal winding is packed in tangent.w ?
        if (VertexSubPartFormat::UX10Y10Z10W2N == pTangentKey->Format()) {
            Assert(pNormalKey->Index() == pTangentKey->Index());
            defines.emplace_back(
                StringFormat("AppIn_Get_Binormal{0}(_AppIn)", pTangentKey->Index()),
                StringFormat("AppIn_Get_BinormalX_PackedInTangentW(AppIn_Get_Normal{0}(_AppIn), AppIn_Get_Tangent{1}(_AppIn), ((_AppIn).Tangent{1}).w)", pNormalKey->Index(), pTangentKey->Index()) );
        }
    }

    oss << " }";

    defines.emplace_back(ToString(ShaderSource::AppIn_VertexDefinitionName()), oss.NullTerminatedStr());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringSlice VertexSubPartFormatToShaderFormat(VertexSubPartFormat format) {
    switch (format)
    {
    case Core::Graphics::VertexSubPartFormat::Float:
        return MakeStringSlice("float");
    case Core::Graphics::VertexSubPartFormat::Float2:
        return MakeStringSlice("float2");
    case Core::Graphics::VertexSubPartFormat::Float3:
        return MakeStringSlice("float3");
    case Core::Graphics::VertexSubPartFormat::Float4:
        return MakeStringSlice("float4");
    case Core::Graphics::VertexSubPartFormat::Byte:
        return MakeStringSlice("byte");
    case Core::Graphics::VertexSubPartFormat::Byte2:
        return MakeStringSlice("byte2");
    case Core::Graphics::VertexSubPartFormat::Byte4:
        return MakeStringSlice("byte4");
    case Core::Graphics::VertexSubPartFormat::UByte:
        return MakeStringSlice("ubyte");
    case Core::Graphics::VertexSubPartFormat::UByte2:
        return MakeStringSlice("ubyte2");
    case Core::Graphics::VertexSubPartFormat::UByte4:
        return MakeStringSlice("ubyte4");
    case Core::Graphics::VertexSubPartFormat::Short:
        return MakeStringSlice("short");
    case Core::Graphics::VertexSubPartFormat::Short2:
        return MakeStringSlice("short2");
    case Core::Graphics::VertexSubPartFormat::Short4:
        return MakeStringSlice("short4");
    case Core::Graphics::VertexSubPartFormat::UShort:
        return MakeStringSlice("ushort");
    case Core::Graphics::VertexSubPartFormat::UShort2:
        return MakeStringSlice("ushort2");
    case Core::Graphics::VertexSubPartFormat::UShort4:
        return MakeStringSlice("ushort4");
    case Core::Graphics::VertexSubPartFormat::Word:
        return MakeStringSlice("word");
    case Core::Graphics::VertexSubPartFormat::Word2:
        return MakeStringSlice("word2");
    case Core::Graphics::VertexSubPartFormat::Word3:
        return MakeStringSlice("word3");
    case Core::Graphics::VertexSubPartFormat::Word4:
        return MakeStringSlice("word4");
    case Core::Graphics::VertexSubPartFormat::UWord:
        return MakeStringSlice("uword");
    case Core::Graphics::VertexSubPartFormat::UWord2:
        return MakeStringSlice("uword2");
    case Core::Graphics::VertexSubPartFormat::UWord3:
        return MakeStringSlice("uword3");
    case Core::Graphics::VertexSubPartFormat::UWord4:
        return MakeStringSlice("uword4");
    case Core::Graphics::VertexSubPartFormat::Half:
        return MakeStringSlice("half");
    case Core::Graphics::VertexSubPartFormat::Half2:
        return MakeStringSlice("half2");
    case Core::Graphics::VertexSubPartFormat::Half4:
        return MakeStringSlice("half4");
    case Core::Graphics::VertexSubPartFormat::Byte2N:
        return MakeStringSlice("float2");
    case Core::Graphics::VertexSubPartFormat::Byte4N:
        return MakeStringSlice("float4");
    case Core::Graphics::VertexSubPartFormat::UByte2N:
        return MakeStringSlice("float2");
    case Core::Graphics::VertexSubPartFormat::UByte4N:
        return MakeStringSlice("float4");
    case Core::Graphics::VertexSubPartFormat::Short2N:
        return MakeStringSlice("float2");
    case Core::Graphics::VertexSubPartFormat::Short4N:
        return MakeStringSlice("float4");
    case Core::Graphics::VertexSubPartFormat::UShort2N:
        return MakeStringSlice("float2");
    case Core::Graphics::VertexSubPartFormat::UShort4N:
        return MakeStringSlice("float4");
    case Core::Graphics::VertexSubPartFormat::UX10Y10Z10W2N:
        return MakeStringSlice("float4");
    }
    AssertNotImplemented();
    return StringSlice();
}
//----------------------------------------------------------------------------
StringSlice VertexSubPartSemanticToShaderSemantic(VertexSubPartSemantic semantic) {
    switch (semantic)
    {
    case Core::Graphics::VertexSubPartSemantic::Position:
        return MakeStringSlice("POSITION");
    case Core::Graphics::VertexSubPartSemantic::TexCoord:
        return MakeStringSlice("TEXCOORD");
    case Core::Graphics::VertexSubPartSemantic::Color:
        return MakeStringSlice("COLOR");
    case Core::Graphics::VertexSubPartSemantic::Normal:
        return MakeStringSlice("NORMAL");
    case Core::Graphics::VertexSubPartSemantic::Tangent:
        return MakeStringSlice("TANGENT");
    case Core::Graphics::VertexSubPartSemantic::Binormal:
        return MakeStringSlice("BINORMAL");
    }
    AssertNotImplemented();
    return StringSlice();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
