#pragma once

#include "Graphics.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class PrimitiveType {
    LineList = 0,
    LineStrip,

    TriangleList,
    TriangleStrip,
};
//----------------------------------------------------------------------------
const char *PrimitiveTypeToCStr(PrimitiveType primitiveType);
//----------------------------------------------------------------------------
size_t PrimitiveCount(PrimitiveType primitiveType, size_t indexCount);
//----------------------------------------------------------------------------
size_t IndexCount(PrimitiveType primitiveType, size_t primitiveCount);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
