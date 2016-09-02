#include "stdafx.h"

#include "PrimitiveType.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
StringView PrimitiveTypeToCStr(PrimitiveType primitiveType) {
    switch (primitiveType)
    {
    case Core::Graphics::PrimitiveType::LineList:
        return MakeStringView("LineList");
    case Core::Graphics::PrimitiveType::LineStrip:
        return MakeStringView("LineStrip");
    case Core::Graphics::PrimitiveType::TriangleList:
        return MakeStringView("TriangleList");
    case Core::Graphics::PrimitiveType::TriangleStrip:
        return MakeStringView("TriangleStrip");
    }

    AssertNotImplemented();
    return StringView();
}
//----------------------------------------------------------------------------
size_t PrimitiveCount(PrimitiveType primitiveType, size_t indexCount) {
    if (0 == indexCount)
        return 0;

    switch (primitiveType)
    {
    case Core::Graphics::PrimitiveType::LineList:
        Assert(0 == (indexCount & 1));
        return indexCount / 2;
    case Core::Graphics::PrimitiveType::LineStrip:
        Assert(1 <= indexCount);
        return indexCount - 1;
    case Core::Graphics::PrimitiveType::TriangleList:
        Assert(0 == (indexCount % 3));
        return indexCount / 3;
    case Core::Graphics::PrimitiveType::TriangleStrip:
        Assert(2 >= (indexCount % 3));
        return indexCount - 2;
    }

    AssertNotImplemented();
    return 0;
}
//----------------------------------------------------------------------------
size_t IndexCount(PrimitiveType primitiveType, size_t primitiveCount) {
    if (0 == primitiveCount)
        return 0;

    switch (primitiveType)
    {
    case Core::Graphics::PrimitiveType::LineList:
        return primitiveCount >> 1;
    case Core::Graphics::PrimitiveType::LineStrip:
        return primitiveCount + 1;
    case Core::Graphics::PrimitiveType::TriangleList:
        return primitiveCount * 3;
    case Core::Graphics::PrimitiveType::TriangleStrip:
        return primitiveCount + 2;
    }

    AssertNotImplemented();
    return 0;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
