#pragma once

#include "HAL/Generic/GenericPlatformIncludes.h"

#include <comdef.h>
#include <comutil.h>
#include <windows.h>
#include <winnt.h>

//  Stupid M$ macros removal ...
#pragma push_macro("CreateDirectory")
#ifdef CreateDirectory
#   undef CreateDirectory
#endif
#pragma push_macro("CreateProcess")
#ifdef CreateProcess
#   undef CreateProcess
#endif
#pragma push_macro("CreateSemaphore")
#ifdef CreateSemaphore
#   undef CreateSemaphore
#endif
#pragma push_macro("MoveFile")
#ifdef MoveFile
#   undef MoveFile
#endif
#pragma push_macro("RemoveDirectory")
#ifdef RemoveDirectory
#   undef RemoveDirectory
#endif
