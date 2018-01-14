#pragma once

#include "Core/Core.h"

#ifndef EXPORT_CORE_EXTERNAL_DOUBLECONVERSION
#   pragma warning(push)
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

#ifdef _MSC_VER
#   pragma warning(disable: 4244) // 'argument': conversion from 'const uc16' to 'char', possible loss of data
#endif

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#ifndef EXPORT_CORE_EXTERNAL_DOUBLECONVERSION

#   pragma include_alias(<double-conversion/utils.h>, <Core.External/double-conversion/double-conversion/utils.h>)
#   include "Core.External/double-conversion/double-conversion/double-conversion.h"

// clean the mess done by xxHash-config.h :

#   pragma pop_macro("ASSERT")
#   pragma pop_macro("UNIMPLEMENTED")
#   pragma pop_macro("UNREACHABLE")

#   pragma warning(pop)

#endif //!EXPORT_CORE_EXTERNAL_DOUBLECONVERSION
