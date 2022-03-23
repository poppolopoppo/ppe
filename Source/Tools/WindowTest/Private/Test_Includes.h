#pragma once

#include "WindowTestApp.h"

#include "RHI/FrameGraph.h"

#include "RHI/BufferDesc.h"
#include "RHI/DrawContext.h"
#include "RHI/ImageView.h"
#include "RHI/MemoryDesc.h"
#include "RHI/PipelineDesc.h"
#include "RHI/SamplerDesc.h"

#include "Diagnostic/Logger.h"
#include "HAL/PlatformNotification.h"
#include "HAL/RHIService.h"
#include "Memory/RefPtr.h"
#include "Meta/Utility.h"

namespace PPE {
EXTERN_LOG_CATEGORY(, WindowTest)
} //!namespace PPE
