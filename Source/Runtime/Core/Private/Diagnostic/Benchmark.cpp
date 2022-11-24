// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Diagnostic/Benchmark.h"

#if USE_PPE_BENCHMARK

namespace PPE {
LOG_CATEGORY_VERBOSITY(PPE_CORE_API, Benchmark, NoDebug)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PRAGMA_DISABLE_OPTIMIZATION
void FBenchmark::UseCharPointer_(char const volatile* p) NOEXCEPT {
    Unused(p);
}
PRAGMA_ENABLE_OPTIMIZATION
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE

#endif //!!(USE_PPE_FINAL_RELEASE)
