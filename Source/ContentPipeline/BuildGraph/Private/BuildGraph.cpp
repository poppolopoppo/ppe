// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "BuildGraph.h"

#include "BuildCache.h"
#include "BuildContext.h"
#include "BuildEnvironment.h"
#include "BuildExecutor.h"
#include "BuildLog.h"
#include "BuildNode.h"

#include "MetaTransaction.h"

#include "Diagnostic/Logger.h"
#include "IO/Filename.h"

namespace PPE {
namespace ContentPipeline {
EXTERN_LOG_CATEGORY(PPE_BUILDGRAPH_API, BuildGraph)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static EBuildResult ReportBuildStatus_(const FPipelineContext& ctx, const FWStringView& action) {
    const FTimespan elapsed = FTimepoint::ElapsedSince(ctx.StartedAt());

    const size_t numBuilt = ctx.NumBuilt();
    const size_t numFailed = ctx.NumFailed();
    const size_t numUpToDate = ctx.NumUpToDate();

    const size_t totalNodes = (numBuilt + numFailed + numUpToDate);

    switch (ctx.Result()) {
    case EBuildResult::Unbuilt:
        Assert_NoAssume(0 == totalNodes);
        ctx.Log().TraceWarning(L"[{0}] nothing done ({1}).",
            action, elapsed);
        return EBuildResult::Unbuilt;

    case EBuildResult::UpToDate:
        Assert_NoAssume(totalNodes == numUpToDate);
        ctx.Log().TraceInfo(L"[{0}] {2} nodes are up-to-date ({1}).",
            action, elapsed, totalNodes);
        return EBuildResult::UpToDate;

    case EBuildResult::Built:
        Assert_NoAssume(totalNodes == numBuilt + numUpToDate);
        ctx.Log().TraceInfo(L"[{0}] {2} nodes were built, {3} up-to-date ({1}).",
            action, elapsed, numBuilt, numUpToDate);
        return EBuildResult::Built;

    case EBuildResult::Failed:
        Assert_NoAssume(numFailed > 0);
        ctx.Log().TraceError(L"[{0}] {2} nodes failed, {3} built, {4} up-to-date ({1}).",
            action, elapsed, numFailed, numBuilt, numUpToDate);
        return EBuildResult::Failed;

    default:
        AssertNotImplemented();
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBuildGraph::FBuildGraph() NOEXCEPT
:   _revision(INDEX_NONE) {
}
//----------------------------------------------------------------------------
FBuildGraph::~FBuildGraph()
{} // for fwd declaration
//----------------------------------------------------------------------------
void FBuildGraph::AddNode(FBuildNode* pnode) {
    Assert(pnode);
    Add_AssertUnique(_roots, SBuildNode{ pnode });
}
//----------------------------------------------------------------------------
void FBuildGraph::AppendNodes(const TMemoryView<const PBuildNode>& nodes) {
    _roots.reserve_Additional(nodes.size());
    for (const PBuildNode& node : nodes)
        AddNode(node.get());
}
//----------------------------------------------------------------------------
void FBuildGraph::AppendNodes(const RTTI::FMetaTransaction& transaction) {
    const RTTI::FMetaClass* const buildNodeClass = FBuildNode::RTTI_FMetaClass::Get();
    Assert(buildNodeClass);

    for (const RTTI::PMetaObject& topObj : transaction.TopObjects()) {
        Assert(topObj);
        if (topObj->RTTI_IsA(*buildNodeClass))
            AddNode(RTTI::CastChecked<FBuildNode>(topObj.get()));
    }
}
//----------------------------------------------------------------------------
void FBuildGraph::Clear() {
    _roots.clear();
    _files.clear();
}
//----------------------------------------------------------------------------
FBuildNode* FBuildGraph::GetFile(const FFilename& fname) const NOEXCEPT {
    Assert_NoAssume(not fname.empty());

    const auto it = _files.find(fname);
    return (it == _files.end() ? nullptr : it->second.get());
}
//----------------------------------------------------------------------------
void FBuildGraph::AddFile(const FFilename& fname, FBuildNode* node) {
    Assert(node);
    Assert_NoAssume(not fname.empty());

    Insert_AssertUnique(_files, fname, SBuildNode{ node });
}
//----------------------------------------------------------------------------
void FBuildGraph::RemoveFile(const FFilename& fname, FBuildNode* node) {
    Assert(node);
    Assert_NoAssume(not fname.empty());

    Remove_AssertExistsAndSameValue(_files, fname, SBuildNode{ node });
}
//----------------------------------------------------------------------------
EBuildResult FBuildGraph::ScanAll(const FBuildEnvironment& env) {
    return BuildNodes_(env, _roots);
}
//----------------------------------------------------------------------------
EBuildResult FBuildGraph::BuildAll(const FBuildEnvironment& env) {
    return BuildNodes_(env, _roots);
}
//----------------------------------------------------------------------------
EBuildResult FBuildGraph::Build(const FBuildEnvironment& env, const FFilename& fname) {
    if (const SBuildNode target{ GetFile(fname) })
        return BuildNodes_(env, TMemoryView<const SBuildNode>(&target, 1));
    else
        return EBuildResult::Failed;
}
//----------------------------------------------------------------------------
EBuildResult FBuildGraph::Build(const FBuildEnvironment& env, const TMemoryView<const FFilename>& fnames) {
    VECTOR(BuildGraph, SBuildNode) nodes;
    nodes.reserve_AssumeEmpty(fnames.size());

    for (const FFilename& fname : fnames) {
        if (SBuildNode target{ GetFile(fname) })
            nodes.emplace_back(std::move(target));
        else
            return EBuildResult::Failed;
    }

    return BuildNodes_(env, nodes);
}
//----------------------------------------------------------------------------
EBuildResult FBuildGraph::CleanAll(const FBuildEnvironment& env) {
    return CleanNodes_(env, _roots);
}
//----------------------------------------------------------------------------
EBuildResult FBuildGraph::Clean(const FBuildEnvironment& env, const FFilename& fname) {
    if (const SBuildNode target{ GetFile(fname) })
        return CleanNodes_(env, TMemoryView<const SBuildNode>(&target, 1));
    else
        return EBuildResult::Failed;
}
//----------------------------------------------------------------------------
EBuildResult FBuildGraph::Clean(const FBuildEnvironment& env, const TMemoryView<const FFilename>& fnames) {
    VECTOR(BuildGraph, SBuildNode) nodes;
    nodes.reserve_AssumeEmpty(fnames.size());

    for (const FFilename& fname : fnames) {
        if (SBuildNode target{ GetFile(fname) })
            nodes.emplace_back(std::move(target));
        else
            return EBuildResult::Failed;
    }

    return CleanNodes_(env, nodes);
}
//----------------------------------------------------------------------------
FBuildRevision FBuildGraph::NextRevision_() {
    return (++_revision);
}
//----------------------------------------------------------------------------
EBuildResult FBuildGraph::ScanNodes_(const FBuildEnvironment& env, const TMemoryView<const SBuildNode>& nodes) {
    FScanContext ctx(*this, env, NextRevision_(), FTimepoint::Now());

    IBuildExecutor& exe = env.Executor();
    exe.QueueAndWaitFor(ctx, nodes);
    exe.WaitForAll();

    if (ctx.Result() != EBuildResult::Failed) {
        _files.clear();
        AddFiles(std::begin(ctx.OutputFiles()), std::end(ctx.OutputFiles()));
    }

    return ReportBuildStatus_(ctx, L"SCAN");
}
//----------------------------------------------------------------------------
EBuildResult FBuildGraph::BuildNodes_(const FBuildEnvironment& env, const TMemoryView<const SBuildNode>& nodes) {
    FBuildContext ctx(*this, env, NextRevision_(), FTimepoint::Now());

    IBuildExecutor& exe = env.Executor();
    exe.QueueAndWaitFor(ctx, nodes);
    exe.WaitForAll();

    return ReportBuildStatus_(ctx, L"BUILD");
}
//----------------------------------------------------------------------------
EBuildResult FBuildGraph::CleanNodes_(const FBuildEnvironment& env, const TMemoryView<const SBuildNode>& nodes) {
    FCleanContext ctx(*this, env, NextRevision_(), FTimepoint::Now());

    IBuildExecutor& exe = env.Executor();
    exe.QueueAndWaitFor(ctx, nodes);
    exe.WaitForAll();

    return ReportBuildStatus_(ctx, L"CLEAN");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
