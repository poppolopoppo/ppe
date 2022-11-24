// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Maths/Octree.h"

#include "Container/MinMaxHeap.h"
#include "HAL/PlatformMaths.h"

#include "Maths/Ray.h"
#include "Maths/ScalarBoundingBox.h"
#include "Maths/ScalarVector.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static constexpr u8 GChildOffset_[8][3] = {
    { 0, 0, 0 },
    { 0, 0, 1 },
    { 0, 1, 0 },
    { 0, 1, 1 },
    { 1, 0, 0 },
    { 1, 0, 1 },
    { 1, 1, 0 },
    { 1, 1, 1 },
};
//----------------------------------------------------------------------------
struct FOctreeCandidate_ {
    i16 X, Y, Z;
    u16 Extent;
    Meta::TPointerWFlags<const FOctreeNode> Node;

    bool IsContained() const { return (Node.Flag0()); }
    void SetContained() { Node.SetFlag0(true); }
};
PPE_ASSERT_TYPE_IS_POD(FOctreeCandidate_);
//----------------------------------------------------------------------------
struct FOctreeCandidateWDist_ {
    float Distance;
    i16 X, Y, Z;
    u16 Extent;
    const FOctreeNode* Node;
};
PPE_ASSERT_TYPE_IS_POD(FOctreeCandidateWDist_);
//----------------------------------------------------------------------------
struct FOctreeClosestCandidate_ {
    bool operator ()(const FOctreeCandidateWDist_& lhs, const FOctreeCandidateWDist_& rhs) const {
        return (rhs.Distance < lhs.Distance);
    }
};
//----------------------------------------------------------------------------
static bool OctreeIsLeaf_(const FOctreeNode::pointer& p) {
    return (p.Flag0());
}
//----------------------------------------------------------------------------
static u32 OctreeLeafIndex_(const FOctreeNode::pointer& p) {
    Assert(OctreeIsLeaf_(p));
    return checked_cast<u32>(uintptr_t(p.Get()) - 1/* offseted to keep p.Get() != nullptr */);
}
//----------------------------------------------------------------------------
static void OctreeNodeBounds_(int x, int y, int z, int extent, float voxelSize, FBoundingBox* bounds) {
    bounds->SetMinMax(
        float3(x * voxelSize, y * voxelSize, z * voxelSize),
        float3((x + extent) * voxelSize, (y + extent) * voxelSize, (z + extent) * voxelSize) );
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static bool RayIntersectsOctree_(
    const FRay& ray, FBasicOctree::FHitResult* firstHit,
    const FOctreeNode* root, float voxelSize, int dimension,
    TStackHeapAdapter<FOctreeCandidateWDist_, FOctreeClosestCandidate_>& candidates) {
    Assert(candidates.empty());

    const i16 halfDim = checked_cast<i16>(-dimension / 2);
    FOctreeCandidateWDist_ it{ FLT_MAX, halfDim, halfDim, halfDim, checked_cast<u16>(dimension), root };

    FBoundingBox bounds;
    FBasicOctree::FHitResult hit{ 0.0f, UMax };
    do {
        Assert(it.Extent > 1);
        const u16 halfExtent = (it.Extent / 2);

        hit.Distance = FLT_MAX;

        forrange(i, 0, 8) {
            const FOctreeNode::pointer& p = it.Node->Children[i];
            if (not p._raw)
                continue;

            const i16 x = it.X + GChildOffset_[i][0] * halfExtent;
            const i16 y = it.Y + GChildOffset_[i][1] * halfExtent;
            const i16 z = it.Z + GChildOffset_[i][2] * halfExtent;

            OctreeNodeBounds_(x, y, z, int(halfExtent), voxelSize, &bounds);

#if USE_PPE_ASSERT
            float d = NAN;
#else
            float d;
#endif
            if (ray.Intersects(bounds, &d)) {
                Assert(not IsNANorINF(d));
                Assert(d >= 0.f);

                if (halfExtent > 1) {
                    candidates.Push(d, x, y, z, halfExtent, p.Get());
                }
                else if (d < hit.Distance){
                    hit.Distance = d;
                    hit.LeafIndex = OctreeLeafIndex_(p);
                }
            }
        }

        if (hit.Distance < FLT_MAX) {
            if (firstHit)
                *firstHit = hit;
            return true;
        }

    } while (candidates.PopMin(&it));

    return false;
}
//----------------------------------------------------------------------------
static bool RayIntersectsOctree_(
    const FRay& ray, FBasicOctree::FHitResult* firstHit,
    const FOctreeNode* root, float voxelSize, int dimension ) {
    Assert(root);

    STACKLOCAL_POD_HEAP(FOctreeCandidateWDist_, FOctreeClosestCandidate_{}, candidates, FPlatformMaths::CeilLog2((size_t)dimension) * 2);

    return RayIntersectsOctree_(ray, firstHit, root, voxelSize, dimension, candidates);
}
//----------------------------------------------------------------------------
static bool BatchRaysIntersectsOctree_(
    const float3& center,
    const TMemoryView<const FRay>& rays,
    const TMemoryView<FBasicOctree::FHitResult>& firstHits,
    const FOctreeNode* root, float voxelSize, int dimension) {
    Assert(root);
    Assert(rays.size() == firstHits.size() || firstHits.empty());

    STACKLOCAL_POD_HEAP(FOctreeCandidateWDist_, FOctreeClosestCandidate_{}, candidates, FPlatformMaths::CeilLog2((size_t)dimension) * 2);

    size_t intersections = 0;

    forrange(i, 0, rays.size()) {
        const FRay& globalRay(rays[i]);
        const FRay localRay(globalRay.Origin() - center, globalRay.Direction());

        FBasicOctree::FHitResult* firstHit = (firstHits.empty() ? nullptr : &firstHits[i]);
        if (RayIntersectsOctree_(localRay, firstHit, root, voxelSize, dimension, candidates))
            intersections++;

        candidates.Clear();
    }

    return intersections;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FBasicOctree::FBasicOctree(const float3& center, float voxelSize, int dimension)
    : _root(nullptr)
    , _center(center)
    , _voxelSize(voxelSize)
    , _dimension(dimension) {
    Assert(_voxelSize > 0);
    Assert(_dimension > 1);
    Assert(Meta::IsPow2(_dimension));
}
//----------------------------------------------------------------------------
FBasicOctree::~FBasicOctree() = default;
//----------------------------------------------------------------------------
bool FBasicOctree::Intersects(const FRay& ray) const {
    const FRay localRay(ray.Origin() - _center, ray.Direction());
    return RayIntersectsOctree_(localRay, nullptr, _root, _voxelSize, _dimension);
}
//----------------------------------------------------------------------------
bool FBasicOctree::Intersects(const FRay& ray, FHitResult* firstHit) const {
    const FRay localRay(ray.Origin() - _center, ray.Direction());
    return RayIntersectsOctree_(localRay, firstHit, _root, _voxelSize, _dimension);
}
//----------------------------------------------------------------------------
size_t FBasicOctree::Intersects(const TMemoryView<const FRay>& rays) const {
    return BatchRaysIntersectsOctree_(_center, rays, TMemoryView<FHitResult>(), _root, _voxelSize, _dimension);
}
//----------------------------------------------------------------------------
size_t FBasicOctree::Intersects(const TMemoryView<const FRay>& rays, const TMemoryView<FHitResult>& firstHits) const {
    return BatchRaysIntersectsOctree_(_center, rays, firstHits, _root, _voxelSize, _dimension);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
