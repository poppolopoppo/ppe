#pragma once

#ifdef PLATFORM_WINDOWS

#   ifdef CPP_CLANG
#       pragma GCC system_header
#       pragma clang system_header
#   endif

#   include <comdef.h>
#   include <comutil.h>
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
