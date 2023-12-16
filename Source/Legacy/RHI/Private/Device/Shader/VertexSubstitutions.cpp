// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "VertexSubstitutions.h"

#include "Device/Geometry/VertexDeclaration.h"
#include "ShaderSource.h"

#include "IO/Stream.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FillVertexSubstitutions(   VECTOR(Shader, TPair<FString COMMA FString>)& defines,
                                const FVertexDeclaration *declaration ) {
    Assert(declaration);

    defines.reserve(defines.size() + declaration->size() + 1 /* auto vertex definition */);

    STACKLOCAL_OCSTRSTREAM(oss, 1024);

    oss << "struct {";

    const FValueField* pNormalKey = nullptr;
    const FValueField* pTangentKey = nullptr;
    const FValueField* pBinormalKey = nullptr;

    for (const FValueField& subPart : declaration->SubParts()) {
        const size_t index = subPart.Index();
        const EValueType type = subPart.Type();
        const Graphics::FName& semantic = subPart.Name();

        if      (semantic == FVertexSemantic::Normal)
            pNormalKey = &subPart;
        else if (semantic == FVertexSemantic::Tangent)
            pTangentKey = &subPart;
        else if (semantic == FVertexSemantic::Binormal)
            pBinormalKey = &subPart;

        const FStringView nameWithoutIndex = semantic.MakeView();

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
        if (EValueType::UX10Y10Z10W2N == pTangentKey->Type()) {
            Assert(pNormalKey->Index() == pTangentKey->Index());
            defines.emplace_back(
                StringFormat("AppIn_Get_Binormal{0}(_AppIn)",
                    pTangentKey->Index()),
                StringFormat("AppIn_Get_BinormalX_PackedInTangentW(AppIn_Get_Normal{0}(_AppIn), AppIn_Get_Tangent{1}(_AppIn), ((_AppIn).Tangent{1}).w)",
                    pNormalKey->Index(), pTangentKey->Index()) );
        }
    }

    oss << " }";

    defines.emplace_back(ToString(FShaderSource::AppIn_VertexDefinitionName()), oss.NullTerminatedStr());

    declaration->FillSubstitutions(defines); // custom substitutions
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView VertexFormatToShaderFormat(FVertexFormat format) {
    switch (format)
    {
    case PPE::Graphics::FVertexFormat::Float:
        return MakeStringView("float");
    case PPE::Graphics::FVertexFormat::Float2:
        return MakeStringView("float2");
    case PPE::Graphics::FVertexFormat::Float3:
        return MakeStringView("float3");
    case PPE::Graphics::FVertexFormat::Float4:
        return MakeStringView("float4");
    case PPE::Graphics::FVertexFormat::Byte:
        return MakeStringView("byte");
    case PPE::Graphics::FVertexFormat::Byte2:
        return MakeStringView("byte2");
    case PPE::Graphics::FVertexFormat::Byte4:
        return MakeStringView("byte4");
    case PPE::Graphics::FVertexFormat::UByte:
        return MakeStringView("ubyte");
    case PPE::Graphics::FVertexFormat::UByte2:
        return MakeStringView("ubyte2");
    case PPE::Graphics::FVertexFormat::UByte4:
        return MakeStringView("ubyte4");
    case PPE::Graphics::FVertexFormat::Short:
        return MakeStringView("short");
    case PPE::Graphics::FVertexFormat::Short2:
        return MakeStringView("short2");
    case PPE::Graphics::FVertexFormat::Short4:
        return MakeStringView("short4");
    case PPE::Graphics::FVertexFormat::UShort:
        return MakeStringView("ushort");
    case PPE::Graphics::FVertexFormat::UShort2:
        return MakeStringView("ushort2");
    case PPE::Graphics::FVertexFormat::UShort4:
        return MakeStringView("ushort4");
    case PPE::Graphics::FVertexFormat::Word:
        return MakeStringView("word");
    case PPE::Graphics::FVertexFormat::Word2:
        return MakeStringView("word2");
    case PPE::Graphics::FVertexFormat::Word3:
        return MakeStringView("word3");
    case PPE::Graphics::FVertexFormat::Word4:
        return MakeStringView("word4");
    case PPE::Graphics::FVertexFormat::UWord:
        return MakeStringView("uword");
    case PPE::Graphics::FVertexFormat::UWord2:
        return MakeStringView("uword2");
    case PPE::Graphics::FVertexFormat::UWord3:
        return MakeStringView("uword3");
    case PPE::Graphics::FVertexFormat::UWord4:
        return MakeStringView("uword4");
    case PPE::Graphics::FVertexFormat::Half:
        return MakeStringView("half");
    case PPE::Graphics::FVertexFormat::Half2:
        return MakeStringView("half2");
    case PPE::Graphics::FVertexFormat::Half4:
        return MakeStringView("half4");
    case PPE::Graphics::FVertexFormat::Byte2N:
        return MakeStringView("float2");
    case PPE::Graphics::FVertexFormat::Byte4N:
        return MakeStringView("float4");
    case PPE::Graphics::FVertexFormat::UByte2N:
        return MakeStringView("float2");
    case PPE::Graphics::FVertexFormat::UByte4N:
        return MakeStringView("float4");
    case PPE::Graphics::FVertexFormat::Short2N:
        return MakeStringView("float2");
    case PPE::Graphics::FVertexFormat::Short4N:
        return MakeStringView("float4");
    case PPE::Graphics::FVertexFormat::UShort2N:
        return MakeStringView("float2");
    case PPE::Graphics::FVertexFormat::UShort4N:
        return MakeStringView("float4");
    case PPE::Graphics::FVertexFormat::UX10Y10Z10W2N:
        return MakeStringView("float4");
    default:
        AssertNotImplemented();
        break;
    }
    return FStringView();
}
//----------------------------------------------------------------------------
FStringView VertexSemanticToShaderSemantic(const Graphics::FName& semantic) {
    if      (semantic == FVertexSemantic::Position)
        return MakeStringView("POSITION");
    else if (semantic == FVertexSemantic::TexCoord)
        return MakeStringView("TEXCOORD");
    else if (semantic == FVertexSemantic::Color)
        return MakeStringView("COLOR");
    else if (semantic == FVertexSemantic::Normal)
        return MakeStringView("NORMAL");
    else if (semantic == FVertexSemantic::Tangent)
        return MakeStringView("TANGENT");
    else if (semantic == FVertexSemantic::Binormal)
        return MakeStringView("BINORMAL");
    else {
        AssertNotImplemented();
        return FStringView();
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
