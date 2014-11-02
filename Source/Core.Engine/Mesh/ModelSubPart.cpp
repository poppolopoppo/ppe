#include "stdafx.h"

#include "ModelSubPart.h"

#include "ModelMeshSubPart.h"

#include "Core/Allocator/PoolAllocator-impl.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_DEF(ModelSubPart, );
//----------------------------------------------------------------------------
ModelSubPart::ModelSubPart(
    const MeshName& name,
    const MeshName& group,
    const AABB3f& boundinbBox,
    VECTOR(Mesh, PModelMeshSubPart)&& meshSubParts )
:   _name(name)
,   _group(group)
,   _boundingBox(boundinbBox)
,   _meshSubParts(std::move(meshSubParts)) {
    Assert(!name.empty());
    Assert(!group.empty());
    Assert(boundinbBox.HasPositiveExtents());
    Assert(_meshSubParts.size());
#ifdef WITH_CORE_ASSERT
    for (const PModelMeshSubPart& p : _meshSubParts)
        Assert(_boundingBox.Contains(p->BoundingBox()));
#endif
}
//----------------------------------------------------------------------------
ModelSubPart::~ModelSubPart() {}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
