#pragma once

#include <stdio.h>

#include <algorithm>
#include <cstdint>
#include <atomic>
#include <memory>
#include <type_traits>

#if defined(_MSC_VER) && !defined(__clang__) /* clang-cl works just fine */
// #TODO: remove this workaround when MSVC is fixed...
// include <optional> early to avoid this compiler/stl issue:
// https://developercommunity.visualstudio.com/t/unexpected-error-c2131-expression-did-not-evaluate/1343697
#	include <optional>
#endif

#ifdef EXPORT_PPE_RUNTIME_CORE
#   define PPE_CORE_API DLL_EXPORT
#else
#   define PPE_CORE_API DLL_IMPORT
#endif

#include "Meta/Config.h"

#include "Meta/Aliases.h"
#include "Meta/Alignment.h"
#include "Meta/Arithmetic.h"
#include "Meta/Assert.h"
#include "Meta/Cast.h"
#include "Meta/Delete.h"
#include "Meta/Enum.h"
#include "Meta/ForRange.h"
#include "Meta/Hash_fwd.h"
#include "Meta/Iterator.h"
#include "Meta/NumericLimits.h"
#include "Meta/OneTimeInitialize.h"
#include "Meta/StronglyTyped.h"
#include "Meta/ThreadResource.h"
#include "Meta/TypeTraits.h"

#include "HAL/PlatformMacros.h"
