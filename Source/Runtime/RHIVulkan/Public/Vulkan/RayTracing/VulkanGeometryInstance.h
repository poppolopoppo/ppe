﻿#pragma once

#include "Vulkan/VulkanCommon.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FVulkanGeometryInstance {
    float4x3 Transform;

    u32 CustomIndex     : 24;
    u32 Mask            :  8;

    u32 InstanceOffset  : 24;
    u32 Flags           :  8;

    FVulkanBLASHandle BlasHandle;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE