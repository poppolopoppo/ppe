#pragma once

#include "Graphics.h"

namespace PPE {
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
} //!namespace PPE
