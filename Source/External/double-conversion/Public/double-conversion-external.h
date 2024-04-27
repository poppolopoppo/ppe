#pragma once

#include "Core.h"

#ifndef EXPORT_PPE_EXTERNAL_DOUBLE_CONVERSION
    PRAGMA_MSVC_WARNING_PUSH()
#   ifdef __clang__
#       pragma clang diagnostic push
#   endif
#   pragma push_macro("ASSERT")
#   pragma push_macro("UNIMPLEMENTED")
#   pragma push_macro("UNREACHABLE")
#endif

#ifdef ASSERT
#   undef ASSERT
#endif
#ifdef UNIMPLEMENTED
#   undef UNIMPLEMENTED
#endif
#ifdef UNREACHABLE
#   undef UNREACHABLE
#endif

#define ASSERT(_CONDITION) Assert(_CONDITION)
#define UNIMPLEMENTED AssertNotImplemented
#define UNREACHABLE AssertNotReached

PRAGMA_MSVC_WARNING_DISABLE(4244) // 'argument': conversion from 'const uc16' to 'char', possible loss of data
PRAGMA_MSVC_WARNING_DISABLE(4505) // 'double_conversion::AssertTrimmedDigits': unreferenced local function has been removed

#ifdef __gcc__
#    pragma GCC system_header
#endif

#ifdef __clang__
#    pragma clang system_header
#    pragma clang diagnostic ignored "-Wunused-function"
#endif

#ifndef EXPORT_PPE_EXTERNAL_DOUBLE_CONVERSION

#   pragma include_alias(<double-conversion/utils.h>, <External/double-conversion/git/double-conversion/utils.h>)
#   include "External/double-conversion/double-conversion.git/double-conversion/double-conversion.h"
#   include "External/double-conversion/double-conversion.git/double-conversion/ieee.h"

// clean the mess done by xxHash-config.h :

#   pragma pop_macro("ASSERT")
#   pragma pop_macro("UNIMPLEMENTED")
#   pragma pop_macro("UNREACHABLE")

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_DOUBLE_CONVERSION
