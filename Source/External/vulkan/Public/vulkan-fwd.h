#pragma once

#include "vulkan-platform.h"

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#if USE_PPE_VULKAN_MINIMALAPI
#   include "vulkan-minimal.h"
#else
#   include "vulkan-external.h"
#endif
