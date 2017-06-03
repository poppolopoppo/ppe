#pragma once

#ifdef PLATFORM_WINDOWS

#   include <windows.h>

//  Stupid M$ macros removal ...
#   pragma push_macro("CreateDirectory")
#   ifdef CreateDirectory
#       undef CreateDirectory
#   endif
#   pragma push_macro("RemoveDirectory")
#   ifdef RemoveDirectory
#       undef RemoveDirectory
#   endif

#endif
