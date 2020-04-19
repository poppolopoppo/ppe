#pragma once

#include "Core_fwd.h"

#define USE_PPE_LOGGER (!USE_PPE_FINAL_RELEASE || USE_PPE_FORCE_LOGGING)

#if USE_PPE_LOGGER

#define LOG_CATEGORY_GET(_NAME) CONCAT(LogCategory_, _NAME)()
#define EXTERN_LOG_CATEGORY(_API, _NAME) \
    _API extern ::PPE::FLoggerCategory& LOG_CATEGORY_GET(_NAME);

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

#endif //!#if USE_PPE_LOGGER
