#include "stdafx.h"

#include "BuildDependencies.h"

#include "BuildGraph.h"

#include "RTTI/Macros-impl.h"
#include "MetaEnum.h"

namespace PPE {
namespace ContentPipeline {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RTTI_ENUM_BEGIN(BuildGraph, EDependencyFlags)
    RTTI_ENUM_VALUE(Default)
    RTTI_ENUM_VALUE(Weak)
RTTI_ENUM_END()
//----------------------------------------------------------------------------
RTTI_CLASS_BEGIN(BuildGraph, FBuildDependencies, Public, Concrete, Mergeable)
    RTTI_PROPERTY_PRIVATE_FIELD(_links)
RTTI_CLASS_END()
//----------------------------------------------------------------------------
void FBuildDependencies::Add(FBuildNode* node, EDependencyFlags flags/* = EDependencyFlags::Default */) {
    Assert(node);
    Assert_NoAssume(not DependsOn(*node));

    _links.emplace_back(node, flags);
}
//----------------------------------------------------------------------------
bool FBuildDependencies::DependsOn(const FBuildNode& node) const {
    auto const it = std::find_if(_links.begin(), _links.end(),
        [pNode{&node}](const FBuildDependency& dep) {
            return (dep.Node.get() == pNode);
        });
    return (it != _links.end());
}
//----------------------------------------------------------------------------
bool FBuildDependencies::Remove(const FBuildNode& node) {
    auto const it = std::find_if(_links.begin(), _links.end(),
        [pNode{ &node }](const FBuildDependency& dep) {
        return (dep.Node.get() == pNode);
    });

    if (it != _links.end()) {
        _links.erase(it);
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
void FBuildDependencies::Reset() {
    _links.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
EBuildResult FBuildDependencies::Build(IBuildContext& ctx) const {

}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentPipeline
} //!namespace PPE
