#pragma once

#include "BuildGraph_fwd.h"

#include "MetaObject.h"
#include "RTTI/Macros.h"
#include "RTTI/Module.h"
#include "RTTI/OpaqueData.h"
#include "RTTI/Typedefs.h"

#include "Container/Vector.h"
#include "Thread/AtomicSpinLock.h"

#include <atomic>

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_MODULE_DECL(PPE_BUILDGRAPH_API, BuildGraph);
//----------------------------------------------------------------------------
struct FBuildState {
    FAtomicReadWriteLock RWLock;
    std::atomic<void*> UserData;
    std::atomic<FBuildRevision> Revision;
    EBuildResult Result{};
};
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildNode : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_BUILDGRAPH_API, FBuildNode, RTTI::FMetaObject);
public:
    FBuildNode() NOEXCEPT;
    virtual ~FBuildNode();

    FBuildState& State() { return _state; }
    const FBuildState& State() const { return _state; }

    const RTTI::FOpaqueData& OpaqueData() const { return _opaqueData; }

    using FDependenciesView = TMemoryView<const PBuildNode>;

    FDependenciesView StaticDeps() const { return _staticDeps; }
    FDependenciesView DynamicDeps() const { return _dynamicDeps; }
    FDependenciesView RuntimeDeps() const { return _runtimeDeps; }

public:
    virtual EBuildResult Scan(FScanContext& ctx) = 0;
    virtual EBuildResult Import(FBuildContext& ctx) = 0;
    virtual EBuildResult Process(FBuildContext& ctx) = 0;
    virtual EBuildResult Clean(FCleanContext& ctx) = 0;

protected:
    RTTI::FOpaqueData& OpaqueData() { return _opaqueData; }

    void AddStaticDep(FBuildNode* node);
    void AddDynamicDep(FBuildNode* node);
    void AddRuntimeDep(FBuildNode* node);

private:
    using FDependencies = VECTORINSITU(BuildGraph, PBuildNode, 3);

    FBuildState _state;

    RTTI::FOpaqueData _opaqueData;

    FDependencies _staticDeps;
    FDependencies _dynamicDeps;
    FDependencies _runtimeDeps;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
