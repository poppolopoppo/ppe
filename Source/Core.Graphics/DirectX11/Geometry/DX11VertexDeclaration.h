#pragma once

#include "Core.Graphics/DirectX11/DX11Includes.h"

#include "Core.Graphics/Device/Geometry/VertexDeclaration.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/MemoryStack.h"

namespace Core {
namespace Graphics {
class DeviceEncapsulator;
} //!namespace Graphics
} //!namespace Core

namespace Core {
namespace Graphics {
namespace DX11 {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class VertexDeclaration : public DeviceAPIDependantVertexDeclaration {
public:
    VertexDeclaration(IDeviceAPIEncapsulator *device, Graphics::VertexDeclaration *owner);
    virtual ~VertexDeclaration();

    MemoryView<const ::D3D11_INPUT_ELEMENT_DESC> Layout() const { return _layout.Cast<const ::D3D11_INPUT_ELEMENT_DESC>(); }

    SINGLETON_POOL_ALLOCATED_DECL(VertexDeclaration);

private:
    StaticStack<::D3D11_INPUT_ELEMENT_DESC, Graphics::VertexDeclaration::MaxSubPartCount> _layout;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DXGI_FORMAT VertexSubPartFormatToDXGIFormat(Graphics::VertexSubPartFormat value);
//----------------------------------------------------------------------------
Graphics::VertexSubPartFormat DXGIFormatToVertexSubPartFormat(DXGI_FORMAT value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
LPCSTR VertexSubPartSemanticToDX11SemanticName(Graphics::VertexSubPartSemantic value);
//----------------------------------------------------------------------------
Graphics::VertexSubPartSemantic DX11SemanticNameVertexSubPartSemantic(LPCSTR value);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace DX11
} //!namespace Graphics
} //!namespace Core
