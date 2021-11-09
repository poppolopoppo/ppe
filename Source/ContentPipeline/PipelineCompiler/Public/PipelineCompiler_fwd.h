#pragma once

#include "Core_fwd.h"

#ifdef EXPORT_PPE_CONTENTPIPELINE_PIPELINECOMPILER
#   define PPE_PIPELINECOMPILER_API DLL_EXPORT
#else
#   define PPE_PIPELINECOMPILER_API DLL_IMPORT
#endif

#include "RHI_fwd.h"

#include "Diagnostic/Logger_fwd.h"

namespace PPE {
namespace RHI {
EXTERN_LOG_CATEGORY(PPE_PIPELINECOMPILER_API, PipelineCompiler);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EVulkanShaderCompilationFlags : u32;
//----------------------------------------------------------------------------
class FVulkanPipelineCompiler;
//----------------------------------------------------------------------------
template <typename T>
class TVulkanDebuggableShaderData;
using FVulkanDebuggableShaderModule = TVulkanDebuggableShaderData<FShaderModule>;
using FVulkanDebuggableShaderSPIRV = TVulkanDebuggableShaderData<FRawData>;
//----------------------------------------------------------------------------
using PVulkanDebuggableShaderModule = TRefPtr<TVulkanDebuggableShaderData<FShaderModule>>;
using PVulkanDebuggableShaderSPIRV = TRefPtr<TVulkanDebuggableShaderData<FRawData>>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
