#pragma once

#include "DirectX11/DX11Includes.h"

#include "Device/Geometry/VertexDeclaration.h"

#include "Allocator/PoolAllocator.h"
#include "Container/Stack.h"

namespace PPE {
namespace Graphics {
class FDeviceEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDX11VertexDeclaration : public FDeviceAPIDependantVertexDeclaration {
public:
    FDX11VertexDeclaration(IDeviceAPIEncapsulator *device, FVertexDeclaration *owner);
    virtual ~FDX11VertexDeclaration();

    TMemoryView<const ::D3D11_INPUT_ELEMENT_DESC> Layout() const { return _layout.MakeView().Cast<const ::D3D11_INPUT_ELEMENT_DESC>(); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    TFixedSizeStack<::D3D11_INPUT_ELEMENT_DESC, FVertexDeclaration::MaxSubPartCount> _layout;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DXGI_FORMAT VertexFormatToDXGIFormat(EValueType value);
//----------------------------------------------------------------------------
EValueType DXGIFormatToVertexFormat(DXGI_FORMAT value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LPCSTR VertexSubPartSemanticToDX11SemanticName(const Graphics::FName& value);
//----------------------------------------------------------------------------
const FVertexSemantic& DX11SemanticNameVertexSubPartSemantic(LPCSTR value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
