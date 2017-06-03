#include "stdafx.h"

#include "DX11VertexDeclaration.h"

#include "DirectX11/DX11DeviceAPIEncapsulator.h"

#include "Device/DeviceAPI.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FDX11VertexDeclaration::FDX11VertexDeclaration(IDeviceAPIEncapsulator *device, FVertexDeclaration* owner)
:   FDeviceAPIDependantVertexDeclaration(device, owner) {
    const size_t count = owner->size();
    Assert(count < _layout.capacity());

    ::D3D11_INPUT_ELEMENT_DESC *const subPartDescs = _layout.Allocate(count);
    ::SecureZeroMemory(subPartDescs, sizeof(::D3D11_INPUT_ELEMENT_DESC) * count);

    for (size_t i = 0; i < count; ++i) {
        const FValueField& subPart = owner->SubPartByIndex(i);

        ::D3D11_INPUT_ELEMENT_DESC& subPartDesc = subPartDescs[i];

        subPartDesc.SemanticName = VertexSubPartSemanticToDX11SemanticName(subPart.Name());
        subPartDesc.SemanticIndex = checked_cast<UINT>(subPart.Index());
        subPartDesc.Format = VertexFormatToDXGIFormat(subPart.Type());
        subPartDesc.AlignedByteOffset = checked_cast<UINT>(subPart.Offset());

        subPartDesc.InputSlot  = 0;
        subPartDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        subPartDesc.InstanceDataStepRate = 0;
    }
}
//----------------------------------------------------------------------------
FDX11VertexDeclaration::~FDX11VertexDeclaration() {}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, FDX11VertexDeclaration, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DXGI_FORMAT VertexFormatToDXGIFormat(FVertexFormat value) {
    switch (value)
    {
    case Core::Graphics::FVertexFormat::Float:
        return DXGI_FORMAT_R32_FLOAT;
    case Core::Graphics::FVertexFormat::Float2:
        return DXGI_FORMAT_R32G32_FLOAT;
    case Core::Graphics::FVertexFormat::Float3:
        return DXGI_FORMAT_R32G32B32_FLOAT;
    case Core::Graphics::FVertexFormat::Float4:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case Core::Graphics::FVertexFormat::Byte:
        return DXGI_FORMAT_R8_SINT;
    case Core::Graphics::FVertexFormat::Byte2:
        return DXGI_FORMAT_R8G8_SINT;
    case Core::Graphics::FVertexFormat::Byte4:
        return DXGI_FORMAT_R8G8B8A8_SINT;
    case Core::Graphics::FVertexFormat::UByte:
        return DXGI_FORMAT_R8_UINT;
    case Core::Graphics::FVertexFormat::UByte2:
        return DXGI_FORMAT_R8G8_UINT;
    case Core::Graphics::FVertexFormat::UByte4:
        return DXGI_FORMAT_R8G8B8A8_UINT;
    case Core::Graphics::FVertexFormat::Short:
        return DXGI_FORMAT_R16_SINT;
    case Core::Graphics::FVertexFormat::Short2:
        return DXGI_FORMAT_R16G16_SINT;
    case Core::Graphics::FVertexFormat::Short4:
        return DXGI_FORMAT_R16G16B16A16_SINT;
    case Core::Graphics::FVertexFormat::UShort:
        return DXGI_FORMAT_R16_UINT;
    case Core::Graphics::FVertexFormat::UShort2:
        return DXGI_FORMAT_R16G16_UINT;
    case Core::Graphics::FVertexFormat::UShort4:
        return DXGI_FORMAT_R16G16B16A16_UINT;
    case Core::Graphics::FVertexFormat::Word:
        return DXGI_FORMAT_R32_SINT;
    case Core::Graphics::FVertexFormat::Word2:
        return DXGI_FORMAT_R32G32_SINT;
    case Core::Graphics::FVertexFormat::Word3:
        return DXGI_FORMAT_R32G32B32_SINT;
    case Core::Graphics::FVertexFormat::Word4:
        return DXGI_FORMAT_R32G32B32A32_SINT;
    case Core::Graphics::FVertexFormat::UWord:
        return DXGI_FORMAT_R32_UINT;
    case Core::Graphics::FVertexFormat::UWord2:
        return DXGI_FORMAT_R32G32_UINT;
    case Core::Graphics::FVertexFormat::UWord3:
        return DXGI_FORMAT_R32G32B32_UINT;
    case Core::Graphics::FVertexFormat::UWord4:
        return DXGI_FORMAT_R32G32B32A32_UINT;
    case Core::Graphics::FVertexFormat::Half:
        return DXGI_FORMAT_R16_FLOAT;
    case Core::Graphics::FVertexFormat::Half2:
        return DXGI_FORMAT_R16G16_FLOAT;
    case Core::Graphics::FVertexFormat::Half4:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case Core::Graphics::FVertexFormat::Byte2N:
        return DXGI_FORMAT_R8G8_SNORM;
    case Core::Graphics::FVertexFormat::Byte4N:
        return DXGI_FORMAT_R8G8B8A8_SNORM;
    case Core::Graphics::FVertexFormat::UByte2N:
        return DXGI_FORMAT_R8G8_UNORM;
    case Core::Graphics::FVertexFormat::UByte4N:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Core::Graphics::FVertexFormat::Short2N:
        return DXGI_FORMAT_R16G16_SNORM;
    case Core::Graphics::FVertexFormat::Short4N:
        return DXGI_FORMAT_R16G16B16A16_SNORM;
    case Core::Graphics::FVertexFormat::UShort2N:
        return DXGI_FORMAT_R16G16_UNORM;
    case Core::Graphics::FVertexFormat::UShort4N:
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    case Core::Graphics::FVertexFormat::UX10Y10Z10W2N:
        return DXGI_FORMAT_R10G10B10A2_UNORM;
    default:
        AssertNotImplemented();
        break;
    }
    return static_cast<DXGI_FORMAT>(-1);
}
//----------------------------------------------------------------------------
FVertexFormat DXGIFormatToVertexFormat(DXGI_FORMAT value) {
    switch (value)
    {
    case DXGI_FORMAT_R32_FLOAT:
        return Core::Graphics::FVertexFormat::Float;
    case DXGI_FORMAT_R32G32_FLOAT:
        return Core::Graphics::FVertexFormat::Float2;
    case DXGI_FORMAT_R32G32B32_FLOAT:
        return Core::Graphics::FVertexFormat::Float3;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return Core::Graphics::FVertexFormat::Float4;
    case DXGI_FORMAT_R8_SINT:
        return Core::Graphics::FVertexFormat::Byte;
    case DXGI_FORMAT_R8G8_SINT:
        return Core::Graphics::FVertexFormat::Byte2;
    case DXGI_FORMAT_R8G8B8A8_SINT:
        return Core::Graphics::FVertexFormat::Byte4;
    case DXGI_FORMAT_R8_UINT:
        return Core::Graphics::FVertexFormat::UByte;
    case DXGI_FORMAT_R8G8_UINT:
        return Core::Graphics::FVertexFormat::UByte2;
    case DXGI_FORMAT_R8G8B8A8_UINT:
        return Core::Graphics::FVertexFormat::UByte4;
    case DXGI_FORMAT_R16_SINT:
        return Core::Graphics::FVertexFormat::Short;
    case DXGI_FORMAT_R16G16_SINT:
        return Core::Graphics::FVertexFormat::Short2;
    case DXGI_FORMAT_R16G16B16A16_SINT:
        return Core::Graphics::FVertexFormat::Short4;
    case DXGI_FORMAT_R16_UINT:
        return Core::Graphics::FVertexFormat::UShort;
    case DXGI_FORMAT_R16G16_UINT:
        return Core::Graphics::FVertexFormat::UShort2;
    case DXGI_FORMAT_R16G16B16A16_UINT:
        return Core::Graphics::FVertexFormat::UShort4;
    case DXGI_FORMAT_R32_SINT:
        return Core::Graphics::FVertexFormat::Word;
    case DXGI_FORMAT_R32G32_SINT:
        return Core::Graphics::FVertexFormat::Word2;
    case DXGI_FORMAT_R32G32B32_SINT:
        return Core::Graphics::FVertexFormat::Word3;
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return Core::Graphics::FVertexFormat::Word4;
    case DXGI_FORMAT_R32_UINT:
        return Core::Graphics::FVertexFormat::UWord;
    case DXGI_FORMAT_R32G32_UINT:
        return Core::Graphics::FVertexFormat::UWord2;
    case DXGI_FORMAT_R32G32B32_UINT:
        return Core::Graphics::FVertexFormat::UWord3;
    case DXGI_FORMAT_R32G32B32A32_UINT:
        return Core::Graphics::FVertexFormat::UWord4;
    case DXGI_FORMAT_R16_FLOAT:
        return Core::Graphics::FVertexFormat::Half;
    case DXGI_FORMAT_R16G16_FLOAT:
        return Core::Graphics::FVertexFormat::Half2;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return Core::Graphics::FVertexFormat::Half4;
    case DXGI_FORMAT_R8G8_SNORM:
        return Core::Graphics::FVertexFormat::Byte2N;
    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return Core::Graphics::FVertexFormat::Byte4N;
    case DXGI_FORMAT_R8G8_UNORM:
        return Core::Graphics::FVertexFormat::UByte2N;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return Core::Graphics::FVertexFormat::UByte4N;
    case DXGI_FORMAT_R16G16_SNORM:
        return Core::Graphics::FVertexFormat::Short2N;
    case DXGI_FORMAT_R16G16B16A16_SNORM:
        return Core::Graphics::FVertexFormat::Short4N;
    case DXGI_FORMAT_R16G16_UNORM:
        return Core::Graphics::FVertexFormat::UShort2N;
    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return Core::Graphics::FVertexFormat::UShort4N;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return Core::Graphics::FVertexFormat::UX10Y10Z10W2N;
    default:
        AssertNotImplemented();
    }
    return static_cast<FVertexFormat>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define DEF_DX11SEMANTIC_NAME(_SEMANTIC) \
    static const char *CONCAT(GDX11SemanticName_, _SEMANTIC) = STRINGIZE(_SEMANTIC)
//----------------------------------------------------------------------------
DEF_DX11SEMANTIC_NAME(POSITION);
DEF_DX11SEMANTIC_NAME(TEXCOORD);
DEF_DX11SEMANTIC_NAME(COLOR);
DEF_DX11SEMANTIC_NAME(NORMAL);
DEF_DX11SEMANTIC_NAME(TANGENT);
DEF_DX11SEMANTIC_NAME(BINORMAL);
//----------------------------------------------------------------------------
#undef DEF_DX11SEMANTIC_NAME
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
LPCSTR VertexSubPartSemanticToDX11SemanticName(const Graphics::FName& value) {
    if      (value == FVertexSemantic::Position)
        return GDX11SemanticName_POSITION;
    else if (value == FVertexSemantic::TexCoord)
        return GDX11SemanticName_TEXCOORD;
    else if (value == FVertexSemantic::Normal)
        return GDX11SemanticName_NORMAL;
    else if (value == FVertexSemantic::Color)
        return GDX11SemanticName_COLOR;
    else if (value == FVertexSemantic::Tangent)
        return GDX11SemanticName_TANGENT;
    else if (value == FVertexSemantic::Binormal)
        return GDX11SemanticName_BINORMAL;
    else {
        AssertNotImplemented();
        return NULL;
    }
}
//----------------------------------------------------------------------------
const FVertexSemantic& DX11SemanticNameVertexSubPartSemantic(LPCSTR value) {
    if      (value == GDX11SemanticName_POSITION)
        return FVertexSemantic::Position;
    else if (value == GDX11SemanticName_TEXCOORD)
        return FVertexSemantic::TexCoord;
    else if (value == GDX11SemanticName_COLOR)
        return FVertexSemantic::Color;
    else if (value == GDX11SemanticName_NORMAL)
        return FVertexSemantic::Normal;
    else if (value == GDX11SemanticName_TANGENT)
        return FVertexSemantic::Tangent;
    else if (value == GDX11SemanticName_BINORMAL)
        return FVertexSemantic::Binormal;
    else {
        AssertNotImplemented();
        return FVertexSemantic::Position;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
