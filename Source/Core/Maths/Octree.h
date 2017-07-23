#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/NodeBasedContainerAllocator.h"

#include "Core/Maths/ScalarBoundingBox_fwd.h"
#include "Core/Maths/ScalarVector.h"

#include "Core/Meta/PointerWFlags.h"

#include <functional>

namespace Core {
class FFrustum;
class FRay;
class FSphere;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FOctreeNode {
    typedef Meta::TPointerWFlags<FOctreeNode> pointer;
    pointer Children[8];
};
CORE_ASSERT_TYPE_IS_POD(FOctreeNode)
//----------------------------------------------------------------------------
class FBasicOctree {
public:
    FBasicOctree(const float3& center, float voxelSize, int dimension);
    ~FBasicOctree();

    FBasicOctree(const FBasicOctree&) = delete;
    FBasicOctree& operator =(const FBasicOctree&) = delete;

    const float3& Center() const { return _center; }
    float VoxelSize() const { return _voxelSize; }
    int Dimension() const { return _dimension; }

    typedef u32 FLeafIndex;

    struct FHitResult {
        float Distance;
        FLeafIndex LeafIndex;
    };

    bool Intersects(const FRay& ray) const;
    bool Intersects(const FRay& ray, FHitResult* firstHit) const;

    size_t Intersects(const TMemoryView<const FRay>& rays) const;
    size_t Intersects(const TMemoryView<const FRay>& rays, const TMemoryView<FHitResult>& firstHits) const;

protected:
    FOctreeNode* _root;
private:
    float3 _center;
    float _voxelSize;
    int _dimension;
};
//----------------------------------------------------------------------------
template <typename _Allocator = typename _Allocator = NODEBASED_CONTAINER_ALLOCATOR(Maths, FOctreeNode) >
class TBareOctree : public FBasicOctree, _Allocator {
public:
    typedef _Allocator allocator_type;

    using typename FBasicOctree::FLeafIndex;
    using typename FBasicOctree::FHitResult;

    using FBasicOctree::Intersects;
    using FBasicOctree::Cull;

private:

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
