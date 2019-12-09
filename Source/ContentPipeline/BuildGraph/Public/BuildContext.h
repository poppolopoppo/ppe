#pragma once

#include "BuildGraph_fwd.h"

#include "BuildEnums.h"

#include "Container/Pair.h"
#include "Container/HashMap.h"
#include "Container/SparseArray.h"
#include "IO/Filename.h"
#include "Time/Timepoint.h"

#include <mutex>

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FPipelineContext : Meta::FNonCopyableNorMovable {
public:
    FPipelineContext* Parent() const { return _parent; }
    FPipelineContext& Root() NOEXCEPT;

    const FBuildGraph& Graph() const { return _graph; }

    EBuildFlags Flags() const { return _flags; }
    FBuildRevision Revision() const { return _revision; }
    FTimepoint StartedAt() const { return _startedAt; }

    const ITargetPlaftorm& Platform() const NOEXCEPT;
    const FDirpath& OutputDir() const NOEXCEPT;

    IBuildCache& Cache() const NOEXCEPT;
    IBuildLog& Log() const NOEXCEPT;

    bool HasRebuild() const { return (_flags & EBuildFlags::Rebuild); }
    bool HasCacheRead() const { return (_flags & EBuildFlags::CacheRead); }
    bool HasCacheWrite() const { return (_flags & EBuildFlags::CacheWrite); }
    bool HasDryRun() const { return (_flags & EBuildFlags::DryRun); }
    bool HasStopOnError() const { return (_flags & EBuildFlags::StopOnError); }
    bool HasVerbose() const { return (_flags & EBuildFlags::Verbose); }

    EBuildResult Result() const { return _result; }

    size_t NumBuilt() const { return _numBuilt; }
    size_t NumFailed() const { return _numFailed; }
    size_t NumUpToDate() const { return _numUpToDate; }

    FDirpath MakeOutputDir(const FDirpath& relative) const NOEXCEPT;
    FFilename MakeOutputFile(const FFilename& relative) const NOEXCEPT;

protected:
    FPipelineContext(
        const FBuildGraph& graph,
        const FBuildEnvironment& environment,
        FBuildRevision revision,
        FTimepoint startedAt,
        EBuildFlags flags ) NOEXCEPT;

    FPipelineContext(FPipelineContext& parent) NOEXCEPT;

    ~FPipelineContext();

    void SetResult_(EBuildResult result) NOEXCEPT;

    FPipelineContext* const _parent;

    const FBuildGraph& _graph;
    const FBuildEnvironment& _environment;
    const EBuildFlags _flags;
    const FBuildRevision _revision;
    const FTimepoint _startedAt;

    std::atomic<EBuildResult> _result;
    std::atomic<size_t> _numBuilt;
    std::atomic<size_t> _numFailed;
    std::atomic<size_t> _numUpToDate;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FScanContext : public FPipelineContext {
public:
    using FTarget = TPair<FFilename, SBuildNode>;

    FScanContext(
        const FBuildGraph& graph,
        const FBuildEnvironment& environment,
        FBuildRevision revision,
        FTimepoint startedAt,
        EBuildFlags flags = EBuildFlags::Default) NOEXCEPT;

    FScanContext(FScanContext& parent) NOEXCEPT;

    ~FScanContext();

    FScanContext* Parent() const { return static_cast<FScanContext*>(FPipelineContext::Parent()); }
    FScanContext& Root() NOEXCEPT { return static_cast<FScanContext&>(FPipelineContext::Root()); }

    const auto& OutputFiles() const { return _outputFiles; }

    EBuildResult Scan(FBuildNode& node);

    void Flush(FScanContext&& rvalue);

    FFileNode* GetOrCreateFileNode(const FFilename& input);
    void AddOutputFile(FBuildNode* target, const FFilename& output);

private:
    std::mutex _barrier;
    HASHMAP(BuildGraph, FFilename, PFileNode) _fileNodes;
    SPARSEARRAY_INSITU(BuildGraph, FTarget) _outputFiles;
};
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildContext : public FPipelineContext {
public:
    FBuildContext(
        const FBuildGraph& graph,
        const FBuildEnvironment& environment,
        FBuildRevision revision,
        FTimepoint startedAt,
        EBuildFlags flags = EBuildFlags::Default) NOEXCEPT;

    FBuildContext(FBuildContext& parent) NOEXCEPT;

    FBuildContext* Parent() const { return static_cast<FBuildContext*>(FPipelineContext::Parent()); }
    FBuildContext& Root() NOEXCEPT { return static_cast<FBuildContext&>(FPipelineContext::Root()); }

    EBuildResult Build(FBuildNode& node);
};
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FCleanContext : public FPipelineContext {
public:
    FCleanContext(
        const FBuildGraph& graph,
        const FBuildEnvironment& environment,
        FBuildRevision revision,
        FTimepoint startedAt,
        EBuildFlags flags = EBuildFlags::Default) NOEXCEPT;

    FCleanContext(FCleanContext& parent) NOEXCEPT;

    FCleanContext* Parent() const { return static_cast<FCleanContext*>(FPipelineContext::Parent()); }
    FCleanContext& Root() NOEXCEPT { return static_cast<FCleanContext&>(FPipelineContext::Root()); }

    EBuildResult Clean(FBuildNode& node);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
