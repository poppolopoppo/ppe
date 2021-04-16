
#include "vulkan-exports.h"
#include "vulkan-exports.generated.cpp"

STATIC_ASSERT(sizeof(vk::exported_api) > 0);
STATIC_ASSERT(sizeof(vk::instance_api) > 0);
STATIC_ASSERT(sizeof(vk::device_api) > 0);

STATIC_ASSERT(sizeof(vk::instance_fn) == sizeof(void*));
STATIC_ASSERT(sizeof(vk::device_fn) == sizeof(void*));
