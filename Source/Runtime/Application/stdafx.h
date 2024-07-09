// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "HAL/PlatformIncludes.h"

#include "Application_fwd.h"

#include "RHI_fwd.h"
#include "IO/String.h"
#include "IO/StringBuilder.h"
#include "Thread/ThreadPool.h"
#include "VirtualFileSystem_fwd.h"

#ifdef BUILD_PCH // deprecated
#   include "stdafx.generated.h"
#endif
