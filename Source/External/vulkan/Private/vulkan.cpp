
#include "vulkan-exports.h"
#include "vulkan-exports.generated.cpp"

STATIC_ASSERT(sizeof(Vulkan::FExportedAPI) > 0);
STATIC_ASSERT(sizeof(Vulkan::FInstanceAPI) > 0);
STATIC_ASSERT(sizeof(Vulkan::FDeviceAPI) > 0);

STATIC_ASSERT(sizeof(Vulkan::FInstanceFunctions) == sizeof(void*));
STATIC_ASSERT(sizeof(Vulkan::FDeviceFunctions) == sizeof(void*));
