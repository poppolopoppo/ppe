/*
// Generated by PCHRefactor.rb v1.0 for target <Core>
//------------------------------------------------------------------------------
// Date : 2017-08-22 12:50:16 UTC
*/

// Processed dependencies for Core-Win32-Debug : 50 std, 43 sdk, 22 prj, 106 src
// Processed dependencies for Core-Win32-FastDebug : 50 std, 43 sdk, 22 prj, 106 src
// Processed dependencies for Core-Win32-Release : 50 std, 43 sdk, 21 prj, 106 src
// Processed dependencies for Core-Win32-Profiling : 50 std, 43 sdk, 21 prj, 106 src
// Processed dependencies for Core-Win32-Final : 50 std, 43 sdk, 21 prj, 106 src
// Processed dependencies for Core-Win64-Debug : 50 std, 42 sdk, 22 prj, 106 src
// Processed dependencies for Core-Win64-FastDebug : 50 std, 42 sdk, 22 prj, 106 src
// Processed dependencies for Core-Win64-Release : 50 std, 42 sdk, 21 prj, 106 src
// Processed dependencies for Core-Win64-Profiling : 50 std, 42 sdk, 21 prj, 106 src
// Processed dependencies for Core-Win64-Final : 50 std, 42 sdk, 21 prj, 106 src

/*
// Standard headers
//------------------------------------------------------------------------------
*/
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cwchar>
#include <cstddef>
#include <initializer_list>
#include <new>
#include <exception>
#include <type_traits>
#include <utility>
#include <iosfwd>
#include <cstdio>
#include <cstring>
#include <atomic>
#include <memory>
#include <typeinfo>
#include <functional>
#include <tuple>
#include <stdexcept>
#include <iomanip>
#include <istream>
#include <ostream>
#include <ios>
#include <streambuf>
#include <system_error>
#include <cerrno>
#include <ctime>
#include <iterator>
#include <mutex>
#include <chrono>
#include <ratio>
#include <thread>
#include <cctype>
#include <cwctype>
#include <locale>
#include <string>
#include <iostream>
#include <sstream>
#include <shared_mutex>
#include <condition_variable>
#include <unordered_map>
#include <list>
#include <vector>
#include <regex>
#include <queue>
#include <deque>

/*
// SDK headers
//------------------------------------------------------------------------------
*/
#if 0 // dummy
#elif defined(BUILDCONFIG_Win32_Debug) || defined(BUILDCONFIG_Win32_FastDebug) || defined(BUILDCONFIG_Win32_Release) || defined(BUILDCONFIG_Win32_Profiling) || defined(BUILDCONFIG_Win32_Final)
#    include "excpt.h"
#    include "vcruntime.h"
#    include "sal.h"
#    include "ConcurrencySal.h"
#    include "vadefs.h"
#    include "stdarg.h"
#    include "vcruntime_string.h"
#    include "yvals.h"
#    include "xkeycheck.h"
#    include "crtdefs.h"
#    include "use_ansi.h"
#    include "stdint.h"
#    include "limits.h"
#    include "ymath.h"
#    include "xtgmath.h"
#    include "vcruntime_new.h"
#    include "vcruntime_new_debug.h"
#    include "xatomic0.h"
#    include "intrin0.h"
#    include "xatomic.h"
#    include "vcruntime_typeinfo.h"
#    include "vcruntime_exception.h"
#    include "eh.h"
#    include "intrin.h"
#    include "setjmp.h"
#    include "immintrin.h"
#    include "wmmintrin.h"
#    include "nmmintrin.h"
#    include "smmintrin.h"
#    include "tmmintrin.h"
#    include "pmmintrin.h"
#    include "emmintrin.h"
#    include "xmmintrin.h"
#    include "mmintrin.h"
#    include "zmmintrin.h"
#    include "ammintrin.h"
#    include "mm3dnow.h"
#    include "xlocinfo.h"
#    include "xstring_insert.h"
#    include "comdef.h"
#    include "comutil.h"
#    include "comip.h"
#    include "comdefsp.h"
#elif defined(BUILDCONFIG_Win64_Debug) || defined(BUILDCONFIG_Win64_FastDebug) || defined(BUILDCONFIG_Win64_Release) || defined(BUILDCONFIG_Win64_Profiling) || defined(BUILDCONFIG_Win64_Final)
#    include "excpt.h"
#    include "vcruntime.h"
#    include "sal.h"
#    include "ConcurrencySal.h"
#    include "vadefs.h"
#    include "stdarg.h"
#    include "vcruntime_string.h"
#    include "yvals.h"
#    include "xkeycheck.h"
#    include "crtdefs.h"
#    include "use_ansi.h"
#    include "stdint.h"
#    include "limits.h"
#    include "ymath.h"
#    include "xtgmath.h"
#    include "vcruntime_new.h"
#    include "vcruntime_new_debug.h"
#    include "xatomic0.h"
#    include "intrin0.h"
#    include "xatomic.h"
#    include "vcruntime_typeinfo.h"
#    include "vcruntime_exception.h"
#    include "eh.h"
#    include "intrin.h"
#    include "setjmp.h"
#    include "immintrin.h"
#    include "wmmintrin.h"
#    include "nmmintrin.h"
#    include "smmintrin.h"
#    include "tmmintrin.h"
#    include "pmmintrin.h"
#    include "emmintrin.h"
#    include "xmmintrin.h"
#    include "mmintrin.h"
#    include "zmmintrin.h"
#    include "ammintrin.h"
#    include "xlocinfo.h"
#    include "xstring_insert.h"
#    include "comdef.h"
#    include "comutil.h"
#    include "comip.h"
#    include "comdefsp.h"
#else
#     error "unknown build config"
#endif

/*
// Project headers
//------------------------------------------------------------------------------
*/
#if 0 // dummy
#elif defined(BUILDCONFIG_Win32_Debug) || defined(BUILDCONFIG_Win32_FastDebug) || defined(BUILDCONFIG_Win64_Debug) || defined(BUILDCONFIG_Win64_FastDebug)
#    include "Core/Misc/Platform_Windows.h" // 98% 104/106
#    include "Core/Core.h" // 98% 104/106
#    include "Core/Meta/Aliases.h" // 98% 104/106
#    include "Core/Meta/Alignment.h" // 98% 104/106
#    include "Core/Meta/TypeTraits.h" // 98% 104/106
#    include "Core/Meta/Assert.h" // 98% 104/106
#    include "Core/Diagnostic/Exception.h" // 98% 104/106
#    include "Core/Meta/BitCount.h" // 98% 104/106
#    include "Core/Meta/Cast.h" // 98% 104/106
#    include "Core/Meta/TypeHash.h" // 98% 104/106
#    include "Core/Meta/Hash_fwd.h" // 98% 104/106
#    include "Core/Meta/Delete.h" // 98% 104/106
#    include "Core/Meta/Enum.h" // 98% 104/106
#    include "Core/Meta/ForRange.h" // 98% 104/106
#    include "Core/Meta/Iterator.h" // 98% 104/106
#    include "Core/Meta/Warnings.h" // 98% 104/106
#    include "Core/Meta/NumericLimits.h" // 98% 104/106
#    include "Core/Meta/OneTimeInitialize.h" // 98% 104/106
#    include "Core/Meta/StronglyTyped.h" // 98% 104/106
#    include "Core/Meta/ThreadResource.h" // 98% 104/106
#    include "Core/Memory/MemoryView.h" // 76% 81/106
#    include "Core/Memory/HashFunctions.h" // 78% 83/106
#elif defined(BUILDCONFIG_Win32_Release) || defined(BUILDCONFIG_Win64_Release)
#    include "Core/Misc/Platform_Windows.h" // 98% 104/106
#    include "Core/Core.h" // 98% 104/106
#    include "Core/Meta/Aliases.h" // 98% 104/106
#    include "Core/Meta/Alignment.h" // 98% 104/106
#    include "Core/Meta/TypeTraits.h" // 98% 104/106
#    include "Core/Meta/Assert.h" // 98% 104/106
#    include "Core/Diagnostic/Exception.h" // 98% 104/106
#    include "Core/Meta/BitCount.h" // 98% 104/106
#    include "Core/Meta/Cast.h" // 98% 104/106
#    include "Core/Meta/TypeHash.h" // 98% 104/106
#    include "Core/Meta/Hash_fwd.h" // 98% 104/106
#    include "Core/Meta/Delete.h" // 98% 104/106
#    include "Core/Meta/Enum.h" // 98% 104/106
#    include "Core/Meta/ForRange.h" // 98% 104/106
#    include "Core/Meta/Iterator.h" // 98% 104/106
#    include "Core/Meta/Warnings.h" // 98% 104/106
#    include "Core/Meta/NumericLimits.h" // 98% 104/106
#    include "Core/Meta/OneTimeInitialize.h" // 98% 104/106
#    include "Core/Meta/StronglyTyped.h" // 98% 104/106
#    include "Core/Meta/ThreadResource.h" // 98% 104/106
#    include "Core/Memory/HashFunctions.h" // 78% 83/106
#elif defined(BUILDCONFIG_Win32_Profiling) || defined(BUILDCONFIG_Win64_Profiling)
#    include "Core/Misc/Platform_Windows.h" // 98% 104/106
#    include "Core/Core.h" // 98% 104/106
#    include "Core/Meta/Aliases.h" // 98% 104/106
#    include "Core/Meta/Alignment.h" // 98% 104/106
#    include "Core/Meta/TypeTraits.h" // 98% 104/106
#    include "Core/Meta/Assert.h" // 98% 104/106
#    include "Core/Diagnostic/Exception.h" // 98% 104/106
#    include "Core/Meta/BitCount.h" // 98% 104/106
#    include "Core/Meta/Cast.h" // 98% 104/106
#    include "Core/Meta/TypeHash.h" // 98% 104/106
#    include "Core/Meta/Hash_fwd.h" // 98% 104/106
#    include "Core/Meta/Delete.h" // 98% 104/106
#    include "Core/Meta/Enum.h" // 98% 104/106
#    include "Core/Meta/ForRange.h" // 98% 104/106
#    include "Core/Meta/Iterator.h" // 98% 104/106
#    include "Core/Meta/Warnings.h" // 98% 104/106
#    include "Core/Meta/NumericLimits.h" // 98% 104/106
#    include "Core/Meta/OneTimeInitialize.h" // 98% 104/106
#    include "Core/Meta/StronglyTyped.h" // 98% 104/106
#    include "Core/Meta/ThreadResource.h" // 98% 104/106
#    include "Core/Memory/HashFunctions.h" // 77% 82/106
#elif defined(BUILDCONFIG_Win32_Final) || defined(BUILDCONFIG_Win64_Final)
#    include "Core/Misc/Platform_Windows.h" // 98% 104/106
#    include "Core/Core.h" // 98% 104/106
#    include "Core/Meta/Aliases.h" // 98% 104/106
#    include "Core/Meta/Alignment.h" // 98% 104/106
#    include "Core/Meta/TypeTraits.h" // 98% 104/106
#    include "Core/Meta/Assert.h" // 98% 104/106
#    include "Core/Diagnostic/Exception.h" // 98% 104/106
#    include "Core/Meta/BitCount.h" // 98% 104/106
#    include "Core/Meta/Cast.h" // 98% 104/106
#    include "Core/Meta/TypeHash.h" // 98% 104/106
#    include "Core/Meta/Hash_fwd.h" // 98% 104/106
#    include "Core/Meta/Delete.h" // 98% 104/106
#    include "Core/Meta/Enum.h" // 98% 104/106
#    include "Core/Meta/ForRange.h" // 98% 104/106
#    include "Core/Meta/Iterator.h" // 98% 104/106
#    include "Core/Meta/Warnings.h" // 98% 104/106
#    include "Core/Meta/NumericLimits.h" // 98% 104/106
#    include "Core/Meta/OneTimeInitialize.h" // 98% 104/106
#    include "Core/Meta/StronglyTyped.h" // 98% 104/106
#    include "Core/Meta/ThreadResource.h" // 98% 104/106
#    include "Core/Memory/HashFunctions.h" // 76% 81/106
#else
#     error "unknown build config"
#endif
