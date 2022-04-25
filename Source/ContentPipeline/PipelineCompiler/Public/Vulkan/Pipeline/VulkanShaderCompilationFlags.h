#pragma once

#include "PipelineCompiler_fwd.h"

#include "Meta/Enum.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EVulkanShaderCompilationFlags : u32
{
    Quiet                       = 1 << 8,
    //KeepSrcShaderData         = 1 << 9,   // compiler will keep incoming GLSL source and adds SPIRV or VkShaderModule

    UseCurrentDeviceLimits      = 1 << 10,  // get current device properties and use it to setup spirv compiler

    // SPIRV compilation flags:
    GenerateDebug               = 1 << 15,
    Optimize                    = 1 << 16,
    OptimizeSize                = 1 << 17,
    StrongOptimization          = 1 << 18, // very slow, may be usable for offline compilation
    Validate                    = 1 << 19,

    ParseAnnotations            = 1 << 20,

    _Last,
    Unknown = 0,
};
ENUM_FLAGS(EVulkanShaderCompilationFlags);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
