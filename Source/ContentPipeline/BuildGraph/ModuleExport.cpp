#include "stdafx.h"

#include "ModuleExport.h"

#include "BuildNode.h"
#include "Memory/MemoryDomain.h"
#include "RTTI_Namespace-impl.h"

PRAGMA_INITSEG_LIB

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
