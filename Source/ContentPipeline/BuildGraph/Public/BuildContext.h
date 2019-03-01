#pragma once

#include "BuildGraph_fwd.h"

#include "IO/FileSystem_fwd.h"
#include "IO/String_fwd.h"
#include "RTTI/Typedefs.h"
#include "Task/TaskManager.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildContext : Meta::FNonCopyable {
public:
    FBuildContext(IBuildExecutor& executor, FBuildNode* root);
    FBuildContext(FBuildContext& parent, FBuildNode* root);

    const IBuildExecutor& Executor() const;
    const FBuildContext* Parent() const;

    void Build(FBuildNode* node);
    bool UpToDate(FBuildNode* node) const;
    void Flush();

    void AddDynamicDependency(FBuildNode* dep);
    void AddRuntimeDependency(FBuildNode* dep);

    void LogInfo(const FWStringView& fmt, const FWFormatArgList& args);
    void LogWarning(const FWStringView& fmt, const FWFormatArgList& args);
    void LogError(const FWStringView& fmt, const FWFormatArgList& args);

private:
    IBuildExecutor& _executor;
    FBuildContext* _parent;
    SBuildNode _root;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
