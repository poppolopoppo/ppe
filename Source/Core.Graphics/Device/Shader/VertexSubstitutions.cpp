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

        const char *nameWithoutIndex = VertexSubPartSemanticToCStr(semantic);
        const char *formatCStr = VertexSubPartFormatToCStr(format);

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

    defines.emplace_back(ShaderSource::AppIn_VertexDefinitionName(), oss.NullTerminatedStr());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
const char *VertexSubPartFormatToShaderFormat(VertexSubPartFormat format) {
    switch (format)
    {
    case Core::Graphics::VertexSubPartFormat::Float:
        return "float";
    case Core::Graphics::VertexSubPartFormat::Float2:
        return "float2";
    case Core::Graphics::VertexSubPartFormat::Float3:
        return "float3";
    case Core::Graphics::VertexSubPartFormat::Float4:
        return "float4";
    case Core::Graphics::VertexSubPartFormat::Byte:
        return "byte";
    case Core::Graphics::VertexSubPartFormat::Byte2:
        return "byte2";
    case Core::Graphics::VertexSubPartFormat::Byte4:
        return "byte4";
    case Core::Graphics::VertexSubPartFormat::UByte:
        return "ubyte";
    case Core::Graphics::VertexSubPartFormat::UByte2:
        return "ubyte2";
    case Core::Graphics::VertexSubPartFormat::UByte4:
        return "ubyte4";
    case Core::Graphics::VertexSubPartFormat::Short:
        return "short";
    case Core::Graphics::VertexSubPartFormat::Short2:
        return "short2";
    case Core::Graphics::VertexSubPartFormat::Short4:
        return "short4";
    case Core::Graphics::VertexSubPartFormat::UShort:
        return "ushort";
    case Core::Graphics::VertexSubPartFormat::UShort2:
        return "ushort2";
    case Core::Graphics::VertexSubPartFormat::UShort4:
        return "ushort4";
    case Core::Graphics::VertexSubPartFormat::Word:
        return "word";
    case Core::Graphics::VertexSubPartFormat::Word2:
        return "word2";
    case Core::Graphics::VertexSubPartFormat::Word3:
        return "word3";
    case Core::Graphics::VertexSubPartFormat::Word4:
        return "word4";
    case Core::Graphics::VertexSubPartFormat::UWord:
        return "uword";
    case Core::Graphics::VertexSubPartFormat::UWord2:
        return "uword2";
    case Core::Graphics::VertexSubPartFormat::UWord3:
        return "uword3";
    case Core::Graphics::VertexSubPartFormat::UWord4:
        return "uword4";
    case Core::Graphics::VertexSubPartFormat::Half:
        return "half";
    case Core::Graphics::VertexSubPartFormat::Half2:
        return "half2";
    case Core::Graphics::VertexSubPartFormat::Half4:
        return "half4";
    case Core::Graphics::VertexSubPartFormat::Byte2N:
        return "float2";
    case Core::Graphics::VertexSubPartFormat::Byte4N:
        return "float4";
    case Core::Graphics::VertexSubPartFormat::UByte2N:
        return "float2";
    case Core::Graphics::VertexSubPartFormat::UByte4N:
        return "float4";
    case Core::Graphics::VertexSubPartFormat::Short2N:
        return "float2";
    case Core::Graphics::VertexSubPartFormat::Short4N:
        return "float4";
    case Core::Graphics::VertexSubPartFormat::UShort2N:
        return "float2";
    case Core::Graphics::VertexSubPartFormat::UShort4N:
        return "float4";
    case Core::Graphics::VertexSubPartFormat::UX10Y10Z10W2N:
        return "float4";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
const char *VertexSubPartSemanticToShaderSemantic(VertexSubPartSemantic semantic) {
    switch (semantic)
    {
    case Core::Graphics::VertexSubPartSemantic::Position:
        return "POSITION";
    case Core::Graphics::VertexSubPartSemantic::TexCoord:
        return "TEXCOORD";
    case Core::Graphics::VertexSubPartSemantic::Color:
        return "COLOR";
    case Core::Graphics::VertexSubPartSemantic::Normal:
        return "NORMAL";
    case Core::Graphics::VertexSubPartSemantic::Tangent:
        return "TANGENT";
    case Core::Graphics::VertexSubPartSemantic::Binormal:
        return "BINORMAL";
    }
    AssertNotImplemented();
    return nullptr;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
