#pragma once

#include "Core.h"

#ifndef EXPORT_PPE_EXTERNAL_DOUBLECONVERSION
    PRAGMA_MSVC_WARNING_PUSH()
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

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_DOUBLECONVERSION

#   pragma include_alias(<double-conversion/utils.h>, <External/double-conversion/git/double-conversion/utils.h>)
#   include "External/double-conversion/git/double-conversion/double-conversion.h"

// clean the mess done by xxHash-config.h :

#   pragma pop_macro("ASSERT")
#   pragma pop_macro("UNIMPLEMENTED")
#   pragma pop_macro("UNREACHABLE")

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_DOUBLECONVERSION
