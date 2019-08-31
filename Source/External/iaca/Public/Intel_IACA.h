#pragma once

#include "External/iaca/Private/iacaMarks.h"

#define USE_INTEL_IACA (0) // %_NOCOMMIT%

#if !USE_INTEL_IACA || defined(FINAL_RELEASE)
#   define INTEL_IACA_START() (void)0
#   define INTEL_IACA_END() (void)0
// seriously intel ? :|
#elif defined(_WIN64)
#   define INTEL_IACA_START() IACA_VC64_START
#   define INTEL_IACA_END() IACA_VC64_END
#else
#   define INTEL_IACA_START() IACA_START
#   define INTEL_IACA_END() IACA_END
#endif
