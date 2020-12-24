#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIInputAssembly.h"


namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EVulkanPrimitiveTopology : u32 {
    PointList = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
    LineList = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
    LineStrip = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
    TriangleList = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    TriangleStrip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
    TriangleFan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
    LineListWithAdjacency = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
    LineStripWithAdjacency = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
    TriangleListWithAdjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
    TriangleStripWithAdjacency = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
    PatchList = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
};
//----------------------------------------------------------------------------
enum class EVulkanVertexInputRate : u32 {
    Vertex = VK_VERTEX_INPUT_RATE_VERTEX,
    Instance = VK_VERTEX_INPUT_RATE_INSTANCE,
};
//----------------------------------------------------------------------------
struct FVulkanVertexBinding {
    u32 Binding{ 0 };
    u32 Stride{ 0 };
    EVulkanVertexInputRate InputRate{ EVulkanVertexInputRate::Vertex };
};
//----------------------------------------------------------------------------
struct FVulkanVertexAttribute {
    u32 Location;
    u32 Binding;
    u32 Offset;
    EVulkanFormat Format;
};
//----------------------------------------------------------------------------
struct FVulkanInputAssembly {
    bool EnablePrimitiveRestart{ false };

    EVulkanPrimitiveTopology PrimitiveType{ EVulkanPrimitiveTopology::TriangleList };

    VECTORINSITU(RHIState, FVulkanVertexBinding, 2) Bindings;
    VECTORINSITU(RHIState, FVulkanVertexAttribute, 2) Attributes;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
