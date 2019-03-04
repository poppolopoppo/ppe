#pragma once

#include "Core_fwd.h"

#if !defined(FINAL_RELEASE) || USE_PPE_FORCE_LOGGING
#   define USE_DEBUG_LOGGER
#endif

#ifdef USE_DEBUG_LOGGER

#define LOG_CATEGORY_GET(_NAME) CONCAT(GLogCategory_, _NAME)
#define EXTERN_LOG_CATEGORY(_API, _NAME) \
    _API extern ::PPE::FLoggerCategory LOG_CATEGORY_GET(_NAME);

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
