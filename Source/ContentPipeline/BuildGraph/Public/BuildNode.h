#pragma once

#include "BuildGraph_fwd.h"

#include "Container/Vector.h"
#include "Meta/StronglyTyped.h"
#include "MetaObject.h"
#include "Memory/RefPtr.h"
#include "Time/Timestamp.h"

#include "RTTI/Macros.h"
#include "RTTI/Namespace.h"
#include "RTTI/Typedefs.h"

namespace PPE {
namespace ContentPipeline {
RTTI_NAMESPACE_DECL(PPE_BUILDGRAPH_API, BuildGraph);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EBuildState : u32 {
    Unbuilt             = 0,
    WaitStaticDeps      ,
    WaitDynamicDeps     ,
    WaitRuntimeDeps     ,
    Building            ,
    UpToDate            ,
    Failed              ,
};
//----------------------------------------------------------------------------
struct FBuildDependency {
    PBuildNode Node;
    bool IsWeak;
};
//----------------------------------------------------------------------------
using FBuildDependencies = VECTORINSITU(BuildGraph, FBuildDependency, 3);
using FBuildFingerprint = u128;
PPE_STRONGLYTYPED_NUMERIC_DEF(u32, FBuildPass);
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildNode : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(FBuildNode, RTTI::FMetaObject);
public:
    virtual ~FBuildNode();

    const RTTI::FName& Name() const { return _name; }
    EBuildState State() const { return _state; }
    FTimestamp LastBuilt() const { return _lastBuilt; }
    const FBuildFingerprint& Fingerpint() const { return _fingerprint; }

    const FBuildDependencies& StaticDeps() const { return _staticDeps; }
    const FBuildDependencies& DynamicDeps() const { return _dynamicDeps; }
    const FBuildDependencies& RuntimeDeps() const { return _runtimeDeps; }

    bool NeedToBuild(FBuildContext& ctx);
    void BuildStep(FBuildContext& ctx, EBuildState state); // intentionally non virtual

protected:
    explicit FBuildNode(RTTI::FConstructorTag);
    explicit FBuildNode(RTTI::FName&& name);

    void AddStaticDeps(FBuildNode& node, bool unique);
    void AddDynamicDeps(FBuildNode& node, bool unique);
    void AddRuntimeDeps(FBuildNode& node, bool unique);

    virtual bool BuildStaticDeps(FBuildContext& ctx);
    virtual bool BuildDynamicDeps(FBuildContext& ctx);
    virtual bool BuildRuntimeDeps(FBuildContext& ctx);
    virtual bool BuildArtefact(FBuildContext& ctx);

private:
    RTTI::FName _name;

    EBuildState _state;
    FBuildPass _pass;
    FTimestamp _lastBuilt;
    FBuildFingerprint _fingerprint;

    FBuildDependencies _staticDeps;
    FBuildDependencies _dynamicDeps;
    FBuildDependencies _runtimeDeps;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
