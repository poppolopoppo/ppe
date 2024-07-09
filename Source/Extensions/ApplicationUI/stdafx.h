// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "HAL/PlatformIncludes.h"

#include "ApplicationUI_fwd.h"

#include "UI/ImGui.h"

#include "RHIApi.h"

#include "Diagnostic/Logger.h"

#include "Runtime/Core/stdafx.h"
#include "Runtime/Application/stdafx.h"
#include "Runtime/RHI/stdafx.h"

#ifdef BUILD_PCH // deprecated
#   include "stdafx.generated.h"
#endif
