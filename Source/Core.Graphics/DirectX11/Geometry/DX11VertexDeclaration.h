#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Stack.h"

namespace Core {
namespace Graphics {
class DeviceEncapsulator;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DX11VertexDeclaration : public DeviceAPIDependantVertexDeclaration {
public:
    DX11VertexDeclaration(IDeviceAPIEncapsulator *device, VertexDeclaration *owner);
    virtual ~DX11VertexDeclaration();

    MemoryView<const ::D3D11_INPUT_ELEMENT_DESC> Layout() const { return _layout.MakeView().Cast<const ::D3D11_INPUT_ELEMENT_DESC>(); }

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FixedSizeStack<::D3D11_INPUT_ELEMENT_DESC, VertexDeclaration::MaxSubPartCount> _layout;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DXGI_FORMAT VertexSubPartFormatToDXGIFormat(VertexSubPartFormat value);
//----------------------------------------------------------------------------
VertexSubPartFormat DXGIFormatToVertexSubPartFormat(DXGI_FORMAT value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LPCSTR VertexSubPartSemanticToDX11SemanticName(VertexSubPartSemantic value);
//----------------------------------------------------------------------------
VertexSubPartSemantic DX11SemanticNameVertexSubPartSemantic(LPCSTR value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
