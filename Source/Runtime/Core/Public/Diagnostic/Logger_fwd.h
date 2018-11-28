#pragma once

#include "Core_fwd.h"

#if !defined(FINAL_RELEASE) || USE_PPE_FORCE_LOGGING
#   define USE_DEBUG_LOGGER
#endif

#ifdef USE_DEBUG_LOGGER

#define EXTERN_LOG_CATEGORY(_API, _NAME) \
    extern _API ::PPE::FLoggerCategory CONCAT(GLogCategory_, _NAME);

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FLoggerCategory;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#else

#define EXTERN_LOG_CATEGORY(...)

#endif //!#ifdef USE_DEBUG_LOGGER
