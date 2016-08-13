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
DX11VertexDeclaration::DX11VertexDeclaration(IDeviceAPIEncapsulator *device, VertexDeclaration* owner)
:   DeviceAPIDependantVertexDeclaration(device, owner) {
    const size_t count = owner->size();
    Assert(count < _layout.capacity());

    ::D3D11_INPUT_ELEMENT_DESC *const subPartDescs = _layout.Allocate(count);
    ::SecureZeroMemory(subPartDescs, sizeof(::D3D11_INPUT_ELEMENT_DESC) * count);

    for (size_t i = 0; i < count; ++i) {
        const ValueBlock::Field& subPart = owner->SubPartByIndex(i);

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
DX11VertexDeclaration::~DX11VertexDeclaration() {}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, DX11VertexDeclaration, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DXGI_FORMAT VertexFormatToDXGIFormat(VertexFormat value) {
    switch (value)
    {
    case Core::Graphics::VertexFormat::Float:
        return DXGI_FORMAT_R32_FLOAT;
    case Core::Graphics::VertexFormat::Float2:
        return DXGI_FORMAT_R32G32_FLOAT;
    case Core::Graphics::VertexFormat::Float3:
        return DXGI_FORMAT_R32G32B32_FLOAT;
    case Core::Graphics::VertexFormat::Float4:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case Core::Graphics::VertexFormat::Byte:
        return DXGI_FORMAT_R8_SINT;
    case Core::Graphics::VertexFormat::Byte2:
        return DXGI_FORMAT_R8G8_SINT;
    case Core::Graphics::VertexFormat::Byte4:
        return DXGI_FORMAT_R8G8B8A8_SINT;
    case Core::Graphics::VertexFormat::UByte:
        return DXGI_FORMAT_R8_UINT;
    case Core::Graphics::VertexFormat::UByte2:
        return DXGI_FORMAT_R8G8_UINT;
    case Core::Graphics::VertexFormat::UByte4:
        return DXGI_FORMAT_R8G8B8A8_UINT;
    case Core::Graphics::VertexFormat::Short:
        return DXGI_FORMAT_R16_SINT;
    case Core::Graphics::VertexFormat::Short2:
        return DXGI_FORMAT_R16G16_SINT;
    case Core::Graphics::VertexFormat::Short4:
        return DXGI_FORMAT_R16G16B16A16_SINT;
    case Core::Graphics::VertexFormat::UShort:
        return DXGI_FORMAT_R16_UINT;
    case Core::Graphics::VertexFormat::UShort2:
        return DXGI_FORMAT_R16G16_UINT;
    case Core::Graphics::VertexFormat::UShort4:
        return DXGI_FORMAT_R16G16B16A16_UINT;
    case Core::Graphics::VertexFormat::Word:
        return DXGI_FORMAT_R32_SINT;
    case Core::Graphics::VertexFormat::Word2:
        return DXGI_FORMAT_R32G32_SINT;
    case Core::Graphics::VertexFormat::Word3:
        return DXGI_FORMAT_R32G32B32_SINT;
    case Core::Graphics::VertexFormat::Word4:
        return DXGI_FORMAT_R32G32B32A32_SINT;
    case Core::Graphics::VertexFormat::UWord:
        return DXGI_FORMAT_R32_UINT;
    case Core::Graphics::VertexFormat::UWord2:
        return DXGI_FORMAT_R32G32_UINT;
    case Core::Graphics::VertexFormat::UWord3:
        return DXGI_FORMAT_R32G32B32_UINT;
    case Core::Graphics::VertexFormat::UWord4:
        return DXGI_FORMAT_R32G32B32A32_UINT;
    case Core::Graphics::VertexFormat::Half:
        return DXGI_FORMAT_R16_FLOAT;
    case Core::Graphics::VertexFormat::Half2:
        return DXGI_FORMAT_R16G16_FLOAT;
    case Core::Graphics::VertexFormat::Half4:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case Core::Graphics::VertexFormat::Byte2N:
        return DXGI_FORMAT_R8G8_SNORM;
    case Core::Graphics::VertexFormat::Byte4N:
        return DXGI_FORMAT_R8G8B8A8_SNORM;
    case Core::Graphics::VertexFormat::UByte2N:
        return DXGI_FORMAT_R8G8_UNORM;
    case Core::Graphics::VertexFormat::UByte4N:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Core::Graphics::VertexFormat::Short2N:
        return DXGI_FORMAT_R16G16_SNORM;
    case Core::Graphics::VertexFormat::Short4N:
        return DXGI_FORMAT_R16G16B16A16_SNORM;
    case Core::Graphics::VertexFormat::UShort2N:
        return DXGI_FORMAT_R16G16_UNORM;
    case Core::Graphics::VertexFormat::UShort4N:
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    case Core::Graphics::VertexFormat::UX10Y10Z10W2N:
        return DXGI_FORMAT_R10G10B10A2_UNORM;
    default:
        AssertNotImplemented();
        break;
    }
    return static_cast<DXGI_FORMAT>(-1);
}
//----------------------------------------------------------------------------
VertexFormat DXGIFormatToVertexFormat(DXGI_FORMAT value) {
    switch (value)
    {
    case DXGI_FORMAT_R32_FLOAT:
        return Core::Graphics::VertexFormat::Float;
    case DXGI_FORMAT_R32G32_FLOAT:
        return Core::Graphics::VertexFormat::Float2;
    case DXGI_FORMAT_R32G32B32_FLOAT:
        return Core::Graphics::VertexFormat::Float3;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return Core::Graphics::VertexFormat::Float4;
    case DXGI_FORMAT_R8_SINT:
        return Core::Graphics::VertexFormat::Byte;
    case DXGI_FORMAT_R8G8_SINT:
        return Core::Graphics::VertexFormat::Byte2;
    case DXGI_FORMAT_R8G8B8A8_SINT:
        return Core::Graphics::VertexFormat::Byte4;
    case DXGI_FORMAT_R8_UINT:
        return Core::Graphics::VertexFormat::UByte;
    case DXGI_FORMAT_R8G8_UINT:
        return Core::Graphics::VertexFormat::UByte2;
    case DXGI_FORMAT_R8G8B8A8_UINT:
        return Core::Graphics::VertexFormat::UByte4;
    case DXGI_FORMAT_R16_SINT:
        return Core::Graphics::VertexFormat::Short;
    case DXGI_FORMAT_R16G16_SINT:
        return Core::Graphics::VertexFormat::Short2;
    case DXGI_FORMAT_R16G16B16A16_SINT:
        return Core::Graphics::VertexFormat::Short4;
    case DXGI_FORMAT_R16_UINT:
        return Core::Graphics::VertexFormat::UShort;
    case DXGI_FORMAT_R16G16_UINT:
        return Core::Graphics::VertexFormat::UShort2;
    case DXGI_FORMAT_R16G16B16A16_UINT:
        return Core::Graphics::VertexFormat::UShort4;
    case DXGI_FORMAT_R32_SINT:
        return Core::Graphics::VertexFormat::Word;
    case DXGI_FORMAT_R32G32_SINT:
        return Core::Graphics::VertexFormat::Word2;
    case DXGI_FORMAT_R32G32B32_SINT:
        return Core::Graphics::VertexFormat::Word3;
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return Core::Graphics::VertexFormat::Word4;
    case DXGI_FORMAT_R32_UINT:
        return Core::Graphics::VertexFormat::UWord;
    case DXGI_FORMAT_R32G32_UINT:
        return Core::Graphics::VertexFormat::UWord2;
    case DXGI_FORMAT_R32G32B32_UINT:
        return Core::Graphics::VertexFormat::UWord3;
    case DXGI_FORMAT_R32G32B32A32_UINT:
        return Core::Graphics::VertexFormat::UWord4;
    case DXGI_FORMAT_R16_FLOAT:
        return Core::Graphics::VertexFormat::Half;
    case DXGI_FORMAT_R16G16_FLOAT:
        return Core::Graphics::VertexFormat::Half2;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return Core::Graphics::VertexFormat::Half4;
    case DXGI_FORMAT_R8G8_SNORM:
        return Core::Graphics::VertexFormat::Byte2N;
    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return Core::Graphics::VertexFormat::Byte4N;
    case DXGI_FORMAT_R8G8_UNORM:
        return Core::Graphics::VertexFormat::UByte2N;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return Core::Graphics::VertexFormat::UByte4N;
    case DXGI_FORMAT_R16G16_SNORM:
        return Core::Graphics::VertexFormat::Short2N;
    case DXGI_FORMAT_R16G16B16A16_SNORM:
        return Core::Graphics::VertexFormat::Short4N;
    case DXGI_FORMAT_R16G16_UNORM:
        return Core::Graphics::VertexFormat::UShort2N;
    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return Core::Graphics::VertexFormat::UShort4N;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return Core::Graphics::VertexFormat::UX10Y10Z10W2N;
    default:
        AssertNotImplemented();
    }
    return static_cast<VertexFormat>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
#define DEF_DX11SEMANTIC_NAME(_SEMANTIC) \
    static const char *CONCAT(gDX11SemanticName_, _SEMANTIC) = STRINGIZE(_SEMANTIC)
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
LPCSTR VertexSubPartSemanticToDX11SemanticName(const Graphics::Name& value) {
    if      (value == VertexSemantic::Position)
        return gDX11SemanticName_POSITION;
    else if (value == VertexSemantic::TexCoord)
        return gDX11SemanticName_TEXCOORD;
    else if (value == VertexSemantic::Normal)
        return gDX11SemanticName_NORMAL;
    else if (value == VertexSemantic::Color)
        return gDX11SemanticName_COLOR;
    else if (value == VertexSemantic::Tangent)
        return gDX11SemanticName_TANGENT;
    else if (value == VertexSemantic::Binormal)
        return gDX11SemanticName_BINORMAL;
    else {
        AssertNotImplemented();
        return NULL;
    }
}
//----------------------------------------------------------------------------
const VertexSemantic& DX11SemanticNameVertexSubPartSemantic(LPCSTR value) {
    if      (value == gDX11SemanticName_POSITION)
        return VertexSemantic::Position;
    else if (value == gDX11SemanticName_TEXCOORD)
        return VertexSemantic::TexCoord;
    else if (value == gDX11SemanticName_COLOR)
        return VertexSemantic::Color;
    else if (value == gDX11SemanticName_NORMAL)
        return VertexSemantic::Normal;
    else if (value == gDX11SemanticName_TANGENT)
        return VertexSemantic::Tangent;
    else if (value == gDX11SemanticName_BINORMAL)
        return VertexSemantic::Binormal;
    else {
        AssertNotImplemented();
        return VertexSemantic::Position;
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
