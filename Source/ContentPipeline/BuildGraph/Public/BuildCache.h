#pragma once

#include "BuildGraph_fwd.h"

#include "IO/FileSystem_fwd.h"
#include "IO/StreamProvider.h"
#include "Memory/RefPtr.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_INTEFARCE_UNIQUEPTR(BuildCache);
//----------------------------------------------------------------------------
class IBuildCache {
public:
    virtual ~IBuildCache() = default;

    virtual void Initialize(const FTimestamp& buildTime) = 0;

    virtual UStreamReader Read(FBuildFingerpint fingerprint) = 0;
    virtual bool Write(FBuildFingerpint fingerprint, const FRawMemoryConst& rawdata) = 0;

    virtual void Cleanup() = 0;
};
//----------------------------------------------------------------------------
PPE_BUILDGRAPH_API UBuildCache MakeLocalBuildCache(const FDirpath& path, bool writable);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
