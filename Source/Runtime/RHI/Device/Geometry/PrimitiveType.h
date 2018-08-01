#pragma once

#include "Core.Graphics/Graphics.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EPrimitiveType {
    LineList = 0,
    LineStrip,

    TriangleList,
    TriangleStrip,
};
//----------------------------------------------------------------------------
FStringView PrimitiveTypeToCStr(EPrimitiveType primitiveType);
//----------------------------------------------------------------------------
size_t PrimitiveCount(EPrimitiveType primitiveType, size_t indexCount);
//----------------------------------------------------------------------------
size_t IndexCount(EPrimitiveType primitiveType, size_t primitiveCount);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
