#pragma once

#include "BuildGraph_fwd.h"

#include "BuildDependencies.h"

#include "Container/Vector.h"
#include "Memory/RefPtr.h"
#include "Misc/Guid.h"

#include "MetaObject.h"
#include "RTTI/Macros.h"
#include "RTTI/Typedefs.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildNode : public RTTI::FMetaObject {
    RTTI_CLASS_HEADER(PPE_BUILDGRAPH_API, FBuildNode, RTTI::FMetaObject);
public:
    virtual ~FBuildNode();

    const RTTI::FName& Name() const { return _name; }

    const FGuid& Revision() const { return _revision; }
    const FBuildFingerprint& Fingerprint() const { return _fingerprint; }

    const FBuildDependencies& StaticDeps() const { return _staticDeps; }
    const FBuildDependencies& DynamicDeps() const { return _dynamicDeps; }
    const FBuildDependencies& RuntimeDeps() const { return _runtimeDeps; }

    bool DependsOn(const FBuildNode& node) const;

    virtual bool ImportData(FBuildContext& ctx) = 0;
    virtual bool BuildArtefact(FBuildContext& ctx) = 0;

protected:
    explicit FBuildNode(RTTI::FConstructorTag);
    explicit FBuildNode(RTTI::FName&& name);

    void AddStaticDep(FBuildNode* node);
    void AddDynamicDep(FBuildNode* node);
    void AddRuntimeDep(FBuildNode* node);

private:
    RTTI::FName _name;

    FGuid _revision;
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
