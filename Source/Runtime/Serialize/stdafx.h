// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "HAL/PlatformIncludes.h"

#include "Serialize_fwd.h"

#include "IO/Filename.h"
#include "IO/String.h"

#include "RTTI/NativeTypes.h"
#include "RTTI/Macros.h"
#include "RTTI/Macros-impl.h"

#include "Runtime/Core/stdafx.h"
#include "Runtime/RTTI/stdafx.h"
#include "Runtime/VFS/stdafx.h"

#ifdef BUILD_PCH // deprecated
#   include "stdafx.generated.h"
#endif
