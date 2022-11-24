// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "BuildEnvironment.h"

#include "RTTI/Any.h"
#include "MetaObject.h"

#include "HAL/TargetPlatform.h"
#include "IO/Filename.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBuildEnvironment::FBuildEnvironment(
    const ITargetPlaftorm& platform,
    const FDirpath& outputDir,
    IBuildCache& cache,
    IBuildExecutor& executor,
    IBuildLog& log ) NOEXCEPT
:   _platform(platform)
,   _outputDir(outputDir)
,   _cache(cache)
,   _executor(executor)
,   _log(log) {
    Assert(not outputDir.empty());
    Assert_NoAssume(outputDir.IsAbsolute());
}
//----------------------------------------------------------------------------
FBuildEnvironment::~FBuildEnvironment() 
{} // for forward declaration
//----------------------------------------------------------------------------
FDirpath FBuildEnvironment::MakeOutputDir(const FDirpath& relative) const {
    Assert(not relative.empty());
    Assert_NoAssume(relative.IsRelative());

    FDirpath absolute;
    VerifyRelease(FDirpath::Absolute(&absolute, _outputDir, relative));

    return absolute;
}
//----------------------------------------------------------------------------
FFilename FBuildEnvironment::MakeOutputFile(const FFilename& relative) const {
    Assert(not relative.empty());
    Assert_NoAssume(relative.IsRelative());

    return relative.Absolute(_outputDir);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
