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
DX11VertexDeclaration::DX11VertexDeclaration(IDeviceAPIEncapsulator *device, VertexDeclaration *owner)
:   DeviceAPIDependantVertexDeclaration(device, owner) {
    const size_t count = owner->size();
    Assert(count < _layout.capacity());

    ::D3D11_INPUT_ELEMENT_DESC *const subPartDescs = _layout.Allocate(count);
    ::SecureZeroMemory(subPartDescs, sizeof(::D3D11_INPUT_ELEMENT_DESC) * count);

    for (size_t i = 0; i < count; ++i) {
        const Pair<const VertexSubPartKey *, const AbstractVertexSubPart *>& subPart = owner->SubPartByIndex(i);

        ::D3D11_INPUT_ELEMENT_DESC& subPartDesc = subPartDescs[i];

        subPartDesc.SemanticName = VertexSubPartSemanticToDX11SemanticName(subPart.first->Semantic());
        subPartDesc.SemanticIndex = checked_cast<UINT>(subPart.first->Index());
        subPartDesc.Format = VertexSubPartFormatToDXGIFormat(subPart.first->Format());
        subPartDesc.AlignedByteOffset = checked_cast<UINT>(subPart.second->Offset());

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
DXGI_FORMAT VertexSubPartFormatToDXGIFormat(VertexSubPartFormat value) {
    switch (value)
    {
    case Core::Graphics::VertexSubPartFormat::Float:
        return DXGI_FORMAT_R32_FLOAT;
    case Core::Graphics::VertexSubPartFormat::Float2:
        return DXGI_FORMAT_R32G32_FLOAT;
    case Core::Graphics::VertexSubPartFormat::Float3:
        return DXGI_FORMAT_R32G32B32_FLOAT;
    case Core::Graphics::VertexSubPartFormat::Float4:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case Core::Graphics::VertexSubPartFormat::Byte:
        return DXGI_FORMAT_R8_SINT;
    case Core::Graphics::VertexSubPartFormat::Byte2:
        return DXGI_FORMAT_R8G8_SINT;
    case Core::Graphics::VertexSubPartFormat::Byte4:
        return DXGI_FORMAT_R8G8B8A8_SINT;
    case Core::Graphics::VertexSubPartFormat::UByte:
        return DXGI_FORMAT_R8_UINT;
    case Core::Graphics::VertexSubPartFormat::UByte2:
        return DXGI_FORMAT_R8G8_UINT;
    case Core::Graphics::VertexSubPartFormat::UByte4:
        return DXGI_FORMAT_R8G8B8A8_UINT;
    case Core::Graphics::VertexSubPartFormat::Short:
        return DXGI_FORMAT_R16_SINT;
    case Core::Graphics::VertexSubPartFormat::Short2:
        return DXGI_FORMAT_R16G16_SINT;
    case Core::Graphics::VertexSubPartFormat::Short4:
        return DXGI_FORMAT_R16G16B16A16_SINT;
    case Core::Graphics::VertexSubPartFormat::UShort:
        return DXGI_FORMAT_R16_UINT;
    case Core::Graphics::VertexSubPartFormat::UShort2:
        return DXGI_FORMAT_R16G16_UINT;
    case Core::Graphics::VertexSubPartFormat::UShort4:
        return DXGI_FORMAT_R16G16B16A16_UINT;
    case Core::Graphics::VertexSubPartFormat::Word:
        return DXGI_FORMAT_R32_SINT;
    case Core::Graphics::VertexSubPartFormat::Word2:
        return DXGI_FORMAT_R32G32_SINT;
    case Core::Graphics::VertexSubPartFormat::Word3:
        return DXGI_FORMAT_R32G32B32_SINT;
    case Core::Graphics::VertexSubPartFormat::Word4:
        return DXGI_FORMAT_R32G32B32A32_SINT;
    case Core::Graphics::VertexSubPartFormat::UWord:
        return DXGI_FORMAT_R32_UINT;
    case Core::Graphics::VertexSubPartFormat::UWord2:
        return DXGI_FORMAT_R32G32_UINT;
    case Core::Graphics::VertexSubPartFormat::UWord3:
        return DXGI_FORMAT_R32G32B32_UINT;
    case Core::Graphics::VertexSubPartFormat::UWord4:
        return DXGI_FORMAT_R32G32B32A32_UINT;
    case Core::Graphics::VertexSubPartFormat::Half:
        return DXGI_FORMAT_R16_FLOAT;
    case Core::Graphics::VertexSubPartFormat::Half2:
        return DXGI_FORMAT_R16G16_FLOAT;
    case Core::Graphics::VertexSubPartFormat::Half4:
        return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case Core::Graphics::VertexSubPartFormat::Byte2N:
        return DXGI_FORMAT_R8G8_SNORM;
    case Core::Graphics::VertexSubPartFormat::Byte4N:
        return DXGI_FORMAT_R8G8B8A8_SNORM;
    case Core::Graphics::VertexSubPartFormat::UByte2N:
        return DXGI_FORMAT_R8G8_UNORM;
    case Core::Graphics::VertexSubPartFormat::UByte4N:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Core::Graphics::VertexSubPartFormat::Short2N:
        return DXGI_FORMAT_R16G16_SNORM;
    case Core::Graphics::VertexSubPartFormat::Short4N:
        return DXGI_FORMAT_R16G16B16A16_SNORM;
    case Core::Graphics::VertexSubPartFormat::UShort2N:
        return DXGI_FORMAT_R16G16_UNORM;
    case Core::Graphics::VertexSubPartFormat::UShort4N:
        return DXGI_FORMAT_R16G16B16A16_UNORM;
    case Core::Graphics::VertexSubPartFormat::UX10Y10Z10W2N:
        return DXGI_FORMAT_R10G10B10A2_UNORM;
    }
    AssertNotImplemented();
    return static_cast<DXGI_FORMAT>(-1);
}
//----------------------------------------------------------------------------
VertexSubPartFormat DXGIFormatToVertexSubPartFormat(DXGI_FORMAT value) {
    switch (value)
    {
    case DXGI_FORMAT_R32_FLOAT:
        return Core::Graphics::VertexSubPartFormat::Float;
    case DXGI_FORMAT_R32G32_FLOAT:
        return Core::Graphics::VertexSubPartFormat::Float2;
    case DXGI_FORMAT_R32G32B32_FLOAT:
        return Core::Graphics::VertexSubPartFormat::Float3;
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
        return Core::Graphics::VertexSubPartFormat::Float4;
    case DXGI_FORMAT_R8_SINT:
        return Core::Graphics::VertexSubPartFormat::Byte;
    case DXGI_FORMAT_R8G8_SINT:
        return Core::Graphics::VertexSubPartFormat::Byte2;
    case DXGI_FORMAT_R8G8B8A8_SINT:
        return Core::Graphics::VertexSubPartFormat::Byte4;
    case DXGI_FORMAT_R8_UINT:
        return Core::Graphics::VertexSubPartFormat::UByte;
    case DXGI_FORMAT_R8G8_UINT:
        return Core::Graphics::VertexSubPartFormat::UByte2;
    case DXGI_FORMAT_R8G8B8A8_UINT:
        return Core::Graphics::VertexSubPartFormat::UByte4;
    case DXGI_FORMAT_R16_SINT:
        return Core::Graphics::VertexSubPartFormat::Short;
    case DXGI_FORMAT_R16G16_SINT:
        return Core::Graphics::VertexSubPartFormat::Short2;
    case DXGI_FORMAT_R16G16B16A16_SINT:
        return Core::Graphics::VertexSubPartFormat::Short4;
    case DXGI_FORMAT_R16_UINT:
        return Core::Graphics::VertexSubPartFormat::UShort;
    case DXGI_FORMAT_R16G16_UINT:
        return Core::Graphics::VertexSubPartFormat::UShort2;
    case DXGI_FORMAT_R16G16B16A16_UINT:
        return Core::Graphics::VertexSubPartFormat::UShort4;
    case DXGI_FORMAT_R32_SINT:
        return Core::Graphics::VertexSubPartFormat::Word;
    case DXGI_FORMAT_R32G32_SINT:
        return Core::Graphics::VertexSubPartFormat::Word2;
    case DXGI_FORMAT_R32G32B32_SINT:
        return Core::Graphics::VertexSubPartFormat::Word3;
    case DXGI_FORMAT_R32G32B32A32_SINT:
        return Core::Graphics::VertexSubPartFormat::Word4;
    case DXGI_FORMAT_R32_UINT:
        return Core::Graphics::VertexSubPartFormat::UWord;
    case DXGI_FORMAT_R32G32_UINT:
        return Core::Graphics::VertexSubPartFormat::UWord2;
    case DXGI_FORMAT_R32G32B32_UINT:
        return Core::Graphics::VertexSubPartFormat::UWord3;
    case DXGI_FORMAT_R32G32B32A32_UINT:
        return Core::Graphics::VertexSubPartFormat::UWord4;
    case DXGI_FORMAT_R16_FLOAT:
        return Core::Graphics::VertexSubPartFormat::Half;
    case DXGI_FORMAT_R16G16_FLOAT:
        return Core::Graphics::VertexSubPartFormat::Half2;
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
        return Core::Graphics::VertexSubPartFormat::Half4;
    case DXGI_FORMAT_R8G8_SNORM:
        return Core::Graphics::VertexSubPartFormat::Byte2N;
    case DXGI_FORMAT_R8G8B8A8_SNORM:
        return Core::Graphics::VertexSubPartFormat::Byte4N;
    case DXGI_FORMAT_R8G8_UNORM:
        return Core::Graphics::VertexSubPartFormat::UByte2N;
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        return Core::Graphics::VertexSubPartFormat::UByte4N;
    case DXGI_FORMAT_R16G16_SNORM:
        return Core::Graphics::VertexSubPartFormat::Short2N;
    case DXGI_FORMAT_R16G16B16A16_SNORM:
        return Core::Graphics::VertexSubPartFormat::Short4N;
    case DXGI_FORMAT_R16G16_UNORM:
        return Core::Graphics::VertexSubPartFormat::UShort2N;
    case DXGI_FORMAT_R16G16B16A16_UNORM:
        return Core::Graphics::VertexSubPartFormat::UShort4N;
    case DXGI_FORMAT_R10G10B10A2_UNORM:
        return Core::Graphics::VertexSubPartFormat::UX10Y10Z10W2N;
    default:
        AssertNotImplemented();
    }
    return static_cast<VertexSubPartFormat>(-1);
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
LPCSTR VertexSubPartSemanticToDX11SemanticName(VertexSubPartSemantic value) {
    switch (value)
    {
    case Core::Graphics::VertexSubPartSemantic::Position:
        return gDX11SemanticName_POSITION;
    case Core::Graphics::VertexSubPartSemantic::TexCoord:
        return gDX11SemanticName_TEXCOORD;
    case Core::Graphics::VertexSubPartSemantic::Color:
        return gDX11SemanticName_COLOR;
    case Core::Graphics::VertexSubPartSemantic::Normal:
        return gDX11SemanticName_NORMAL;
    case Core::Graphics::VertexSubPartSemantic::Tangent:
        return gDX11SemanticName_TANGENT;
    case Core::Graphics::VertexSubPartSemantic::Binormal:
        return gDX11SemanticName_BINORMAL;
    default:
        AssertNotImplemented();
    }
    return nullptr;
}
//----------------------------------------------------------------------------
VertexSubPartSemantic DX11SemanticNameVertexSubPartSemantic(LPCSTR value) {
    if (value == gDX11SemanticName_POSITION)
        return Core::Graphics::VertexSubPartSemantic::Position;
    if (value == gDX11SemanticName_TEXCOORD)
        return Core::Graphics::VertexSubPartSemantic::TexCoord;
    if (value == gDX11SemanticName_COLOR)
        return Core::Graphics::VertexSubPartSemantic::Color;
    if (value == gDX11SemanticName_NORMAL)
        return Core::Graphics::VertexSubPartSemantic::Normal;
    if (value == gDX11SemanticName_TANGENT)
        return Core::Graphics::VertexSubPartSemantic::Tangent;
    if (value == gDX11SemanticName_BINORMAL)
        return Core::Graphics::VertexSubPartSemantic::Binormal;
    AssertNotImplemented();
    return static_cast<Graphics::VertexSubPartSemantic>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
