#include "stdafx.h"

#include "DX11PrimitiveType.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeToDX11PrimitiveTopology(PrimitiveType value) {
    switch (value)
    {
    case Core::Graphics::PrimitiveType::LineList:
        return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case Core::Graphics::PrimitiveType::LineStrip:
        return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case Core::Graphics::PrimitiveType::TriangleList:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case Core::Graphics::PrimitiveType::TriangleStrip:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    }
    AssertNotImplemented();
    return static_cast<D3D11_PRIMITIVE_TOPOLOGY>(-1);
}
//----------------------------------------------------------------------------
PrimitiveType DX11PrimitiveTopologyToPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY value) {
    switch (value)
    {
    case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
        return Core::Graphics::PrimitiveType::LineList;
    case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
        return Core::Graphics::PrimitiveType::LineStrip;
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
        return Core::Graphics::PrimitiveType::TriangleList;
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
        return Core::Graphics::PrimitiveType::TriangleStrip;
    default:
        AssertNotImplemented();
    }
    return static_cast<Graphics::PrimitiveType>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
