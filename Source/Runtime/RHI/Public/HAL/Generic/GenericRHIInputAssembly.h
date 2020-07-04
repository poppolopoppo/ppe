#pragma once

#include "HAL/Generic/GenericRHI_fwd.h"

#include "Container/Vector.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EGenericPrimitiveTopology : u32 {
    PointList = 0,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
    TriangleFan,
    LineListWithAdjacency,
    LineStripWithAdjacency,
    TriangleListWithAdjacency,
    TriangleStripWithAdjacency,
    PatchList,
};
//----------------------------------------------------------------------------
enum class EGenericVertexInputRate : u32 {
    Vertex = 0,
    Instance,
};
//----------------------------------------------------------------------------
struct FGenericVertexBinding {
    u32 Binding{ 0 };
    u32 Stride{ 0 };
    EGenericVertexInputRate InputRate{ EGenericVertexInputRate::Vertex };
};
//----------------------------------------------------------------------------
struct FGenericVertexAttribute {
    u32 Location;
    u32 Binding;
    u32 Offset;
    EGenericFormat Format;
};
//----------------------------------------------------------------------------
struct FGenericInputAssembly {
    bool EnablePrimitiveRestart{ false };

    EGenericPrimitiveTopology PrimitiveType{ EGenericPrimitiveTopology::TriangleList };

    VECTORINSITU(RHIState, FGenericVertexBinding, 2) Bindings;
    VECTORINSITU(RHIState, FGenericVertexAttribute, 2) Attributes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
