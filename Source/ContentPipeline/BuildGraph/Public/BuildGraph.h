#pragma once

#include "BuildGraph_fwd.h"

#include "Container/HashMap.h"
#include "Container/Vector.h"
#include "Diagnostic/Logger_fwd.h"
#include "IO/FileSystem_fwd.h"

#include "RTTI_fwd.h"

namespace PPE {
namespace ContentPipeline {
EXTERN_LOG_CATEGORY(PPE_BUILDGRAPH_API, BuildGraph)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildGraph : Meta::FNonCopyableNorMovable {
public:
    FBuildGraph() NOEXCEPT;
    ~FBuildGraph();

    FBuildRevision Revision() const { return _revision; }

public: // Construction
    void AddNode(FBuildNode* pnode);
    void AppendNodes(const TMemoryView<const PBuildNode>& nodes);
    void AppendNodes(const RTTI::FMetaTransaction& transaction);
    void Clear();

public: // Link
    FBuildNode* GetFile(const FFilename& fname) const NOEXCEPT;

    void AddFile(const FFilename& fname, FBuildNode* node);
    void RemoveFile(const FFilename& fname, FBuildNode* node);

    template <typename _It>
    void AddFiles(_It first, _It last) { _files.insert(first, last); }

public: // Build
    EBuildResult ScanAll(const FBuildEnvironment& env);

    EBuildResult BuildAll(const FBuildEnvironment& env);
    EBuildResult Build(const FBuildEnvironment& env, const FFilename& fname);
    EBuildResult Build(const FBuildEnvironment& env, const TMemoryView<const FFilename>& fnames);

    EBuildResult CleanAll(const FBuildEnvironment& env);
    EBuildResult Clean(const FBuildEnvironment& env, const FFilename& fname);
    EBuildResult Clean(const FBuildEnvironment& env, const TMemoryView<const FFilename>& fnames);

private:
    FBuildRevision _revision;

    VECTOR(BuildGraph, SBuildNode) _roots;
    HASHMAP(BuildGraph, FFilename, SBuildNode) _files;

    FBuildRevision NextRevision_();

    EBuildResult ScanNodes_(const FBuildEnvironment& env, const TMemoryView<const SBuildNode>& nodes);
    EBuildResult BuildNodes_(const FBuildEnvironment& env, const TMemoryView<const SBuildNode>& nodes);
    EBuildResult CleanNodes_(const FBuildEnvironment& env, const TMemoryView<const SBuildNode>& nodes);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
