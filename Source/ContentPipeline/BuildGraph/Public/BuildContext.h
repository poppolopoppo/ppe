#pragma once

#include "BuildGraph_fwd.h"

#include "IO/Dirpath.h"
#include "IO/String_fwd.h"
#include "Misc/Guid.h"
#include "RTTI/Typedefs.h"

namespace PPE {
class PPE_CORE_API ITargetPlaftorm;
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EBuildFlags {
    None        = 0,
    Rebuild     = 1<<0,
    CacheRead   = 1<<1,
    CacheWrite  = 1<<2,
    DryRun      = 1<<3,
    Verbose     = 1<<4,
};
ENUM_FLAGS(EBuildFlags);
//----------------------------------------------------------------------------
enum class EBuildResult {
    Unbuilt,
    UpToDate,
    Failed,
    Built
};
//----------------------------------------------------------------------------
struct FBuildInfos {
    FGuid Revision;
    FDirpath Output;
    FDirpath Intermediate;
    //IBuildCache* Cache; #TODO
    IBuildExecutor* Executor;
    //IBuildLog* Log; #TODO
    const ITargetPlaftorm* Platform;
};
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildContext : Meta::FNonCopyable {
public:
    FBuildContext(const FBuildInfos& infos, FBuildNode& root);
    FBuildContext(FBuildContext& parent, FBuildNode& root);

    const FBuildContext* Parent() const { return _parent; }
    const FBuildInfos& Infos() const { return _infos; }
    const FBuildNode& Root() const { return (*_root); }

    EBuildResult State() const { return _state; }

    bool NeedToBuild(FBuildNode& node);
    void Build(FBuildNode& node);
    void Flush();

    void LogInfo(const FWStringView& fmt, const FWFormatArgList& args);
    void LogWarning(const FWStringView& fmt, const FWFormatArgList& args);
    void LogError(const FWStringView& fmt, const FWFormatArgList& args);

private:
    FBuildContext* const _parent;
    const FBuildInfos& _infos;
    const SBuildNode _root;

    EBuildResult _state;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
