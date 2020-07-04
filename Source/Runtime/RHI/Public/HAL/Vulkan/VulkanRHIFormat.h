#pragma once

#include "HAL/Vulkan/VulkanRHI_fwd.h"

#ifdef RHI_VULKAN

#include "HAL/Generic/GenericRHIFormat.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
using EVulkanColorSpace = EGenericColorSpace;
using EVulkanPixelFormat = EGenericPixelFormat;
using EVulkanVertexFormat = EGenericVertexFormat;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!RHI_VULKAN
