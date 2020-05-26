#include "stdafx.h"

#include "PrimitiveType.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FStringView PrimitiveTypeToCStr(EPrimitiveType primitiveType) {
    switch (primitiveType)
    {
    case PPE::Graphics::EPrimitiveType::LineList:
        return MakeStringView("LineList");
    case PPE::Graphics::EPrimitiveType::LineStrip:
        return MakeStringView("LineStrip");
    case PPE::Graphics::EPrimitiveType::TriangleList:
        return MakeStringView("TriangleList");
    case PPE::Graphics::EPrimitiveType::TriangleStrip:
        return MakeStringView("TriangleStrip");
    }

    AssertNotImplemented();
    return FStringView();
}
//----------------------------------------------------------------------------
size_t PrimitiveCount(EPrimitiveType primitiveType, size_t indexCount) {
    if (0 == indexCount)
        return 0;

    switch (primitiveType)
    {
    case PPE::Graphics::EPrimitiveType::LineList:
        Assert(0 == (indexCount & 1));
        return indexCount / 2;
    case PPE::Graphics::EPrimitiveType::LineStrip:
        Assert(1 <= indexCount);
        return indexCount - 1;
    case PPE::Graphics::EPrimitiveType::TriangleList:
        Assert(0 == (indexCount % 3));
        return indexCount / 3;
    case PPE::Graphics::EPrimitiveType::TriangleStrip:
        Assert(2 >= (indexCount % 3));
        return indexCount - 2;
    }

    AssertNotImplemented();
    return 0;
}
//----------------------------------------------------------------------------
size_t IndexCount(EPrimitiveType primitiveType, size_t primitiveCount) {
    if (0 == primitiveCount)
        return 0;

    switch (primitiveType)
    {
    case PPE::Graphics::EPrimitiveType::LineList:
        return primitiveCount >> 1;
    case PPE::Graphics::EPrimitiveType::LineStrip:
        return primitiveCount + 1;
    case PPE::Graphics::EPrimitiveType::TriangleList:
        return primitiveCount * 3;
    case PPE::Graphics::EPrimitiveType::TriangleStrip:
        return primitiveCount + 2;
    }

    AssertNotImplemented();
    return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
