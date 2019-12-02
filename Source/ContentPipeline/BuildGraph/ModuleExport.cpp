#include "stdafx.h"

#include "ModuleExport.h"

#include "BuildGraph.h"

#include "Allocator/New.h"
#include "Memory/MemoryDomain.h"
#include "RTTI/Module-impl.h"

#include "Module-impl.h"

namespace PPE {
namespace ContentPipeline {
RTTI_MODULE_DEF(PPE_BUILDGRAPH_API, BuildGraph, BuildGraph);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBuildGraphModule::FBuildGraphModule()
:   FModule("ContentPipeline/BuildGraph")
{}
//----------------------------------------------------------------------------
FBuildGraphModule::~FBuildGraphModule() = default;
//----------------------------------------------------------------------------
void FBuildGraphModule::Start(FModuleManager& manager) {
    FModule::Start(manager);

    RTTI_MODULE(BuildGraph).Start();
}
//----------------------------------------------------------------------------
void FBuildGraphModule::Shutdown() {
    FModule::Shutdown();

    RTTI_MODULE(BuildGraph).Shutdown();
}
//----------------------------------------------------------------------------
void FBuildGraphModule::ReleaseMemory() {
    FModule::ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
