#include "stdafx.h"

#include "BuildGraph.h"

#include "BuildNode.h"

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

    RTTI_NAMESPACE(BuildGraph).Start();
}
//----------------------------------------------------------------------------
void FBuildGraphModule::Shutdown() {
    FModule::Shutdown();

    RTTI_NAMESPACE(BuildGraph).Shutdown();
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