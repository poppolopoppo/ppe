#pragma once

#include "Core_fwd.h"

#ifdef EXPORT_PPE_CONTENTPIPELINE_PIPELINEREFLECTION
#   define PPE_PIPELINEREFLECTION_API DLL_EXPORT
#else
#   define PPE_PIPELINEREFLECTION_API DLL_IMPORT
#endif

#include "RHI_fwd.h"
