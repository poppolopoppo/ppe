#include "stdafx.h"

#include "BuildGraph.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBuildGraphModule::FBuildGraphModule()
:   FModule("ContentPipeline/BuildGraph")
{}
//----------------------------------------------------------------------------
FBuildGraphModule::~FBuildGraphModule()
{}
//----------------------------------------------------------------------------
void FBuildGraphModule::Start(FModuleManager& manager) {
    FModule::Start(manager);
}
//----------------------------------------------------------------------------
void FBuildGraphModule::Shutdown() {
    FModule::Shutdown();
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
