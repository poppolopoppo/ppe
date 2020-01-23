#pragma once

#include "BuildGraph_fwd.h"

#include "BuildNode.h"

#include "IO/Filename.h"
#include "Time/Timestamp.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(FileNode);
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FFileNode : public FBuildNode {
    RTTI_CLASS_HEADER(PPE_BUILDGRAPH_API, FFileNode, FBuildNode);

public:
    FFileNode() NOEXCEPT;
    explicit FFileNode(const FFilename& filename) NOEXCEPT;

    const FFilename& Filename() const { return _filename; }
    FTimestamp Timestamp() const { return _timestamp; }

#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const PPE_THROW() override;
#endif

public: // FBuildNode:
    virtual EBuildResult Scan(FScanContext& ctx) override;
    virtual EBuildResult Import(FBuildContext& ctx) override;
    virtual EBuildResult Process(FBuildContext& ctx) override;
    virtual EBuildResult Clean(FCleanContext& ctx) override;

protected:
    void SetFilename(const FFilename& filename) NOEXCEPT;

private:
    FFilename _filename;
    FTimestamp _timestamp;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
