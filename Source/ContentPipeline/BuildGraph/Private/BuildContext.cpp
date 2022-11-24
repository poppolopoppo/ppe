// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "BuildContext.h"

#include "BuildCache.h"
#include "BuildEnvironment.h"
#include "BuildExecutor.h"
#include "BuildGraph.h"
#include "BuildLog.h"
#include "BuildNode.h"
#include "Build/FileNode.h"

#include "Diagnostic/Logger.h"
#include "IO/Filename.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPipelineContext::FPipelineContext(
    const FBuildGraph& graph,
    const FBuildEnvironment& environment,
    FBuildRevision revision,
    FTimepoint startedAt,
    EBuildFlags flags ) NOEXCEPT
:   _parent(nullptr)
,   _graph(graph)
,   _environment(environment)
,   _flags(flags)
,   _revision(revision)
,   _startedAt(startedAt)
,   _result(EBuildResult::Unbuilt)
,   _numBuilt(0)
,   _numFailed(0)
,   _numUpToDate(0)
{}
//----------------------------------------------------------------------------
FPipelineContext::FPipelineContext(FPipelineContext& parent) NOEXCEPT
:   _parent(&parent)
,   _graph(parent._graph)
,   _environment(parent._environment)
,   _flags(parent._flags)
,   _revision(parent._revision)
,   _startedAt(parent._startedAt)
//  build state is synchronized in the destructor bellow
,   _result(EBuildResult::Unbuilt)
,   _numBuilt(0)
,   _numFailed(0)
,   _numUpToDate(0)
{}
//----------------------------------------------------------------------------
FPipelineContext::~FPipelineContext() {
    if (_parent) {
        _parent->_numBuilt += _numBuilt;
        _parent->_numFailed += _numFailed;
        _parent->_numUpToDate += _numUpToDate;

        for (;;) {
            EBuildResult old = _parent->_result;
            EBuildResult upd = Combine(old, _result);
            if (_parent->_result.compare_exchange_weak(old, upd))
                break;
        }
    }
}
//----------------------------------------------------------------------------
FPipelineContext& FPipelineContext::Root() NOEXCEPT {
    FPipelineContext* root = this;
    for (; root->_parent; root = root->_parent);
    return (*root);
}
//----------------------------------------------------------------------------
const ITargetPlaftorm& FPipelineContext::Platform() const NOEXCEPT {
    return _environment.Platform();
}
//----------------------------------------------------------------------------
const FDirpath& FPipelineContext::OutputDir() const NOEXCEPT {
    return _environment.OutputDir();
}
//----------------------------------------------------------------------------
IBuildCache& FPipelineContext::Cache() const NOEXCEPT {
    return _environment.Cache();
}
//----------------------------------------------------------------------------
IBuildLog& FPipelineContext::Log() const NOEXCEPT {
    return _environment.Log();
}
//----------------------------------------------------------------------------
void FPipelineContext::SetResult_(EBuildResult result) NOEXCEPT {
    switch (result) {
    case PPE::ContentPipeline::EBuildResult::Built: ++_numBuilt; break;
    case PPE::ContentPipeline::EBuildResult::Failed: ++_numFailed; break;
    case PPE::ContentPipeline::EBuildResult::UpToDate: ++_numUpToDate; break;
    case PPE::ContentPipeline::EBuildResult::Unbuilt: AssertNotReached();
    }

    EBuildResult old = _result;
    for (;;) {
        const EBuildResult desired = Combine(result, old);
        if (old == desired || _result.compare_exchange_weak(old, desired))
            return;
    }
}
//----------------------------------------------------------------------------
FDirpath FPipelineContext::MakeOutputDir(const FDirpath& relative) const NOEXCEPT {
    Assert(not relative.empty());
    Assert_NoAssume(relative.IsRelative());

    return _environment.MakeOutputDir(relative);
}
//----------------------------------------------------------------------------
FFilename FPipelineContext::MakeOutputFile(const FFilename& relative) const NOEXCEPT {
    Assert(not relative.empty());
    Assert_NoAssume(relative.IsRelative());

    return _environment.MakeOutputFile(relative);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FScanContext::FScanContext(
    const FBuildGraph& graph,
    const FBuildEnvironment& environment,
    FBuildRevision revision,
    FTimepoint startedAt,
    EBuildFlags flags /* = EBuildFlags::Default */) NOEXCEPT
:   FPipelineContext(graph, environment, revision, startedAt, flags)
{}
//----------------------------------------------------------------------------
FScanContext::FScanContext(FScanContext& parent) NOEXCEPT
:   FPipelineContext(parent)
{}
//----------------------------------------------------------------------------
FScanContext::~FScanContext() {
    if (!!_parent & !_outputFiles.empty()) {
        FScanContext& pr = static_cast<FScanContext&>(*_parent);
        const Meta::FLockGuard scopeLock(pr._barrier);
        pr._outputFiles.AppendMove(_outputFiles);
    }
}
//----------------------------------------------------------------------------
EBuildResult FScanContext::Scan(FBuildNode& node) {
    IBuildExecutor& exe = _environment.Executor();
    IBuildLog& log = _environment.Log();

    log.NodeBegin(node);

    EBuildResult result = exe.QueueAndWaitFor(*this, node.StaticDeps());

    if (EBuildResult::Failed != result)
        result = Combine(result, node.Scan(*this));

    if ((EBuildResult::Failed != result) | not HasStopOnError())
        exe.Queue(*this, node.DynamicDeps());

    if ((EBuildResult::Failed != result) | not HasStopOnError())
        exe.Queue(*this, node.RuntimeDeps());

    SetResult_(result);

    log.NodeEnd(node, result);

    return result;
}
//----------------------------------------------------------------------------
void FScanContext::Flush(FScanContext&& rvalue) {
    Assert(this != &rvalue);
    Assert_NoAssume(rvalue._fileNodes.empty());

    if (rvalue._outputFiles.empty())
        return;

    const Meta::FLockGuard scopeLock(_barrier);
    _outputFiles.AppendMove(rvalue._outputFiles);
}
//----------------------------------------------------------------------------
FFileNode* FScanContext::GetOrCreateFileNode(const FFilename& input) {
    Assert(not input.empty());

    if (_parent) // always use global state, to ensure uniqueness
        return Root().GetOrCreateFileNode(input);

    const Meta::FLockGuard scopeLock(_barrier);

    PFileNode& n = _fileNodes.FindOrAdd(input);
    if (not n)
        n = NEW_RTTI(FFileNode, input);

    return n.get();
}
//----------------------------------------------------------------------------
void FScanContext::AddOutputFile(FBuildNode* target, const FFilename& output) {
    Assert(target);
    Assert(not output.empty());

    const Meta::FLockGuard scopeLock(_barrier);

    _outputFiles.Emplace(output, SBuildNode{ target });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBuildContext::FBuildContext(
    const FBuildGraph& graph,
    const FBuildEnvironment& environment,
    FBuildRevision revision,
    FTimepoint startedAt,
    EBuildFlags flags /* = EBuildFlags::Default */) NOEXCEPT
:   FPipelineContext(graph, environment, revision, startedAt, flags)
{}
//----------------------------------------------------------------------------
FBuildContext::FBuildContext(FBuildContext& parent) NOEXCEPT
:   FPipelineContext(parent)
{}
//----------------------------------------------------------------------------
EBuildResult FBuildContext::Build(FBuildNode& node) {
    IBuildExecutor& exe = _environment.Executor();
    IBuildLog& log = _environment.Log();

    log.NodeBegin(node);

    EBuildResult result = exe.QueueAndWaitFor(*this, node.StaticDeps());

    if (EBuildResult::Failed != result) {
        result = Combine(result, node.Import(*this));

        if ((EBuildResult::Built == result) | ((EBuildResult::UpToDate == result) & HasRebuild())) {
            result = exe.QueueAndWaitFor(*this, node.DynamicDeps());

            if (EBuildResult::Failed != result) {
                result = Combine(result, node.Process(*this));

                if ((EBuildResult::Failed != result) | not HasStopOnError())
                    exe.Queue(*this, node.RuntimeDeps());
            }
        }
    }

    SetResult_(result);

    log.NodeEnd(node, result);

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FCleanContext::FCleanContext(
    const FBuildGraph& graph,
    const FBuildEnvironment& environment,
    FBuildRevision revision,
    FTimepoint startedAt,
    EBuildFlags flags /* = EBuildFlags::Default */) NOEXCEPT
:   FPipelineContext(graph, environment, revision, startedAt, flags)
{}
//----------------------------------------------------------------------------
FCleanContext::FCleanContext(FCleanContext& parent) NOEXCEPT
:   FPipelineContext(parent)
{}
//----------------------------------------------------------------------------
EBuildResult FCleanContext::Clean(FBuildNode& node) {
    IBuildExecutor& exe = _environment.Executor();
    IBuildLog& log = _environment.Log();

    log.NodeBegin(node);

    EBuildResult result = exe.QueueAndWaitFor(*this, node.StaticDeps());

    if (EBuildResult::Failed != result)
        result = Combine(result, node.Clean(*this));

    if ((EBuildResult::Failed != result) | not HasStopOnError())
        exe.Queue(*this, node.DynamicDeps());

    if ((EBuildResult::Failed != result) | not HasStopOnError())
        exe.Queue(*this, node.RuntimeDeps());

    SetResult_(result);

    log.NodeEnd(node, result);

    return result;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
