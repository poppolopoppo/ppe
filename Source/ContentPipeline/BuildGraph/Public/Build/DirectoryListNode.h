#pragma once

#include "BuildGraph_fwd.h"

#include "BuildNode.h"

#include "IO/Dirpath.h"
#include "IO/String.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(DirectoryListNode);
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FDirectoryListNode : public FBuildNode {
    RTTI_CLASS_HEADER(PPE_BUILDGRAPH_API, FDirectoryListNode, FBuildNode);
public:
    FDirectoryListNode() NOEXCEPT;

    explicit FDirectoryListNode(const FDirpath& path) NOEXCEPT;
    FDirectoryListNode(const FDirpath& path, FWString&& pattern) NOEXCEPT;

    const FDirpath& Path() const { return _path; }
    const FWString& Pattern() const { return _pattern; }

#ifdef WITH_RTTI_VERIFY_PREDICATES
    virtual void RTTI_VerifyPredicates() const PPE_THROW() override;
#endif

public: // FBuildNode:
    virtual EBuildResult Scan(FScanContext& ctx) override;
    virtual EBuildResult Import(FBuildContext& ctx) override;
    virtual EBuildResult Process(FBuildContext& ctx) override;
    virtual EBuildResult Clean(FCleanContext& ctx) override;

protected:
    void SetPath(const FDirpath& path) NOEXCEPT;
    void SetPattern(FWString&& pattern) NOEXCEPT;

private:
    FDirpath _path;
    FWString _pattern;
    bool _recursive;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
