// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "DX11PrimitiveType.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeToDX11PrimitiveTopology(EPrimitiveType value) {
    switch (value)
    {
    case PPE::Graphics::EPrimitiveType::LineList:
        return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
    case PPE::Graphics::EPrimitiveType::LineStrip:
        return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case PPE::Graphics::EPrimitiveType::TriangleList:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case PPE::Graphics::EPrimitiveType::TriangleStrip:
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
        return PPE::Graphics::EPrimitiveType::LineList;
    case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
        return PPE::Graphics::EPrimitiveType::LineStrip;
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
        return PPE::Graphics::EPrimitiveType::TriangleList;
    case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
        return PPE::Graphics::EPrimitiveType::TriangleStrip;
    default:
        AssertNotImplemented();
    }
    return static_cast<Graphics::EPrimitiveType>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
