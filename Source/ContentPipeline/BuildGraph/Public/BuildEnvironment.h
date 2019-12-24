#pragma once

#include "BuildGraph_fwd.h"

#include "HAL/TargetPlatform_fwd.h"
#include "IO/Dirpath.h"
#include "IO/FileSystem_fwd.h"
#include "RTTI/OpaqueData.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildEnvironment : Meta::FNonCopyableNorMovable {
public:
    FBuildEnvironment(
        const ITargetPlaftorm& platform,
        const FDirpath& outputDir,
        IBuildCache& cache,
        IBuildExecutor& executor,
        IBuildLog& log ) NOEXCEPT;
    ~FBuildEnvironment();

    const ITargetPlaftorm& Platform() const { return _platform; }
    const FDirpath& OutputDir() const { return _outputDir; }

    IBuildCache& Cache() const { return _cache; }
    IBuildExecutor& Executor() const { return _executor; }
    IBuildLog& Log() const { return _log; }

    RTTI::FOpaqueData& OpaqueData() { return _opaqueData; }
    const RTTI::FOpaqueData& OpaqueData() const { return _opaqueData; }

    FDirpath MakeOutputDir(const FDirpath& relative) const;
    FFilename MakeOutputFile(const FFilename& relative) const;

private:
    const ITargetPlaftorm& _platform;
    const FDirpath _outputDir;

    IBuildCache& _cache;
    IBuildExecutor& _executor;
    IBuildLog& _log;

    RTTI::FOpaqueData _opaqueData;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
