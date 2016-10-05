#include "stdafx.h"

#include "DX11PrimitiveType.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeToDX11PrimitiveTopology(EPrimitiveType value) {
    switch (value)
    {
    case Core::Graphics::EPrimitiveType::LineList:
        return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case Core::Graphics::EPrimitiveType::LineStrip:
        return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case Core::Graphics::EPrimitiveType::TriangleList:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case Core::Graphics::EPrimitiveType::TriangleStrip:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    }
    AssertNotImplemented();
    return static_cast<D3D11_PRIMITIVE_TOPOLOGY>(-1);
}
//----------------------------------------------------------------------------
EPrimitiveType DX11PrimitiveTopologyToPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY value) {
    switch (value)
    {
    case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
        return Core::Graphics::EPrimitiveType::LineList;
    case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
        return Core::Graphics::EPrimitiveType::LineStrip;
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
        return Core::Graphics::EPrimitiveType::TriangleList;
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
        return Core::Graphics::EPrimitiveType::TriangleStrip;
    default:
        AssertNotImplemented();
    }
    return static_cast<Graphics::EPrimitiveType>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
