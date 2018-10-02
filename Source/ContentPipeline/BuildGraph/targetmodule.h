#pragma once

#include "BuildGraph.h"

#ifndef PPE_STATICMODULES_STARTUP
#   error "ContentPipeline/BuildGraph/targetmodule.h can't be included first !"
#endif

using FBuildGraphModule = PPE::ContentPipeline::FBuildGraphModule;
PPE_STATICMODULE_STARTUP_DEF(BuildGraph);

#undef PPE_STATICMODULES_STARTUP
#define PPE_STATICMODULES_STARTUP PPE_STATICMODULE_STARTUP_NAME(BuildGraph)
