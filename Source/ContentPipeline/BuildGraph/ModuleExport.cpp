#include "stdafx.h"

#include "ModuleExport.h"

#include "BuildNode.h"

#include "Allocator/New.h"
#include "Memory/MemoryDomain.h"
#include "RTTI/Namespace-impl.h"

#include "Module-impl.h"

namespace PPE {
namespace ContentPipeline {
RTTI_NAMESPACE_DEF(PPE_BUILDGRAPH_API, BuildGraph, BuildGraph);
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
