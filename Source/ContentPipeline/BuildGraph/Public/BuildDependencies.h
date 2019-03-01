#pragma once

#include "BuildGraph_fwd.h"

#include "Container/Vector.h"
#include "RTTI/Macros.h"
#include "MetaObject.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EDependencyFlags {
    Default = 0,
    Weak    = 1<<0, // won't fail the build if the dependency failed
};
ENUM_FLAGS(EDependencyFlags);
RTTI_ENUM_HEADER(PPE_BUILDGRAPH_API, EDependencyFlags);
//----------------------------------------------------------------------------
struct FBuildDependency {
    PBuildNode Node;
    EDependencyFlags Flags;
};
//----------------------------------------------------------------------------
class PPE_BUILDGRAPH_API FBuildDependencies : public RTTI::FMetaObject {
public:
    RTTI_CLASS_HEADER(FBuildDependencies, RTTI::FMetaObject);

    FBuildDependencies();

    const VECTOR(BuildGraph, FBuildDependency)& Links() const { return _links; }

    void Add(FBuildNode* node, EDependencyFlags flags = EDependencyFlags::Default);
    bool DependsOn(const FBuildNode& node) const;
    bool Remove(const FBuildNode& node);
    void Reset();

    EBuildResult Build(IBuildContext& ctx) const;

private:
    VECTOR(BuildGraph, FBuildDependency) _links;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE