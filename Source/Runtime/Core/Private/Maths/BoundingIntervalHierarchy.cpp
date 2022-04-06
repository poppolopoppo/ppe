#include "stdafx.h"

#include "Maths/BoundingIntervalHierarchy.h"

#include "Container/MinMaxHeap.h"
#include "HAL/PlatformMaths.h"

#include "Maths/Frustum.h"
#include "Maths/PackingHelpers.h"
#include "Maths/Ray.h"
#include "Maths/ScalarVectorHelpers.h"
#include "Maths/Sphere.h"

// http://ainc.de/Research/BIH.pdf
// http://scholarworks.uark.edu/cgi/viewcontent.cgi?article=2054&context=etd

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
FORCE_INLINE static size_t ApproxBIHTreeDepth_(size_t count) {
    return (FPlatformMaths::CeilLog2(count) + 1);
}
//----------------------------------------------------------------------------
FORCE_INLINE static u32 QuantizeClip0_(float value, float vmin, float vmax) {
    return u32(QuantizeCeil<size_t, FBIHNode::Clip0Bits>(value, vmin, vmax));
}
//----------------------------------------------------------------------------
FORCE_INLINE static u32 QuantizeClip1_(float value, float vmin, float vmax) {
    return u32(QuantizeFloor<size_t, FBIHNode::Clip1Bits>(value, vmin, vmax));
}
//----------------------------------------------------------------------------
FORCE_INLINE static float UnquantizeClip0_(u32 quantized, float vmin, float vmax) {
    return Unquantize<size_t, FBIHNode::Clip0Bits>(size_t(quantized), vmin, vmax);
}
//----------------------------------------------------------------------------
FORCE_INLINE static float UnquantizeClip1_(u32 quantized, float vmin, float vmax) {
    return Unquantize<size_t, FBIHNode::Clip1Bits>(size_t(quantized), vmin, vmax);
}
//----------------------------------------------------------------------------
template <typename _Volume>
static size_t CullBIHTree_(
    const _Volume& volume,
    const FBasicBIHTree::hitrange_delegate& hitrange,
    size_t count, FBIHNode* root, const FBoundingBox& bounds ) {

    if (0 == count)
        return 0;

    struct FBIHCandidate_ {
        const FBIHNode* Node;
        FBoundingBox Bounds;
        u32 Begin, End;
    };

    FBIHCandidate_ it{ root, bounds, 0, checked_cast<u32>(count) };
    STACKLOCAL_ASSUMEPOD_STACK(FBIHCandidate_, candidates, ApproxBIHTreeDepth_(count));

    size_t intersections = 0;

    do {
        Assert(it.Node);
        Assert(it.Bounds.HasPositiveExtents());
        Assert(it.Begin < it.End);

        const size_t axis = it.Node->Axis;
        const u32 split = it.Node->Split;
        Assert(it.Begin <= split);
        Assert(it.End >= split);

        const float node0 = it.Bounds.Min()[axis];
        const float node1 = it.Bounds.Max()[axis];

        FBoundingBox bounds0 = it.Bounds, bounds1 = it.Bounds;
        bounds0.Max()[axis] = UnquantizeClip0_(it.Node->Clip0, node0, node1);
        bounds1.Min()[axis] = UnquantizeClip1_(it.Node->Clip1, node0, node1);

        const EContainmentType collision0 = volume(bounds0);
        const EContainmentType collision1 = volume(bounds1);

        const bool isLeaf = it.Node->IsLeaf();

        if ((EContainmentType::Contains == collision0) ||
            (EContainmentType::Intersects == collision0 && isLeaf) ) {
            hitrange(it.Begin, split, collision0);
            intersections += (split - it.Begin);
        }
        else if (EContainmentType::Intersects == collision0) {
            candidates.Push(it.Node + it.Node->Child0, bounds0, it.Begin, split);
        }
        else {
            Assert(EContainmentType::Disjoint == collision0);
        }

        if ((EContainmentType::Contains == collision1) ||
            (EContainmentType::Intersects == collision1 && isLeaf)) {
            hitrange(split, it.End, collision1);
            intersections += (it.End - split);
        }
        else if (EContainmentType::Intersects == collision0) {
            candidates.Push(it.Node + it.Node->Child0 + 1, bounds1, split, it.End);
        }
        else {
            Assert(EContainmentType::Disjoint == collision1);
        }

    } while (candidates.Pop(&it));

    return intersections;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
STATIC_ASSERT(sizeof(FBIHNode) == sizeof(u64));
STATIC_ASSERT(64 == (FBIHNode::AxisBits + FBIHNode::Child0Bits + FBIHNode::Clip0Bits + FBIHNode::Clip1Bits + FBIHNode::SplitBits));
PPE_ASSERT_TYPE_IS_POD(FBIHNode);
//----------------------------------------------------------------------------
FBasicBIHTree::FBasicBIHTree()
    : _root(nullptr)
{}
//----------------------------------------------------------------------------
FBasicBIHTree::FBasicBIHTree(FBasicBIHTree&& rvalue) NOEXCEPT
    : _root(nullptr) {
    std::swap(_root, rvalue._root);
    swap(_bounds, rvalue._bounds);
}
//----------------------------------------------------------------------------
FBasicBIHTree& FBasicBIHTree::operator =(FBasicBIHTree&& rvalue) NOEXCEPT {
    Clear();
    std::swap(_root, rvalue._root);
    swap(_bounds, rvalue._bounds);
    return (*this);
}
//----------------------------------------------------------------------------
FBasicBIHTree::~FBasicBIHTree() {
    Clear();
}
//----------------------------------------------------------------------------
void FBasicBIHTree::Build(
    size_t maxItemsPerLeaf,
    const FBoundingBox& bounds,
    const TMemoryView<u32>& indices,
    const TMemoryView<const FBoundingBox>& aabbs ) {
    Assert(nullptr == _root);
    Assert(maxItemsPerLeaf);
    Assert(indices.size() == aabbs.size());
    Assert(indices.size() <= FBIHNode::MaxItems);
    Assert(indices.size() / maxItemsPerLeaf <= FBIHNode::MaxNodes);

    _root = allocator_traits::AllocateOneT<FBIHNode>(*this);
    Assert(_root);

    _bounds = bounds;
    Assert(_bounds.HasPositiveExtents());

    struct FBIHCandidate_ {
        u32 Begin, End;
        FBIHNode* Node;
        FBoundingBox Grid;
        FBoundingBox Bounds;
    };

    FBIHCandidate_ it{ 0, checked_cast<u32>(indices.size()), _root, _bounds, _bounds };
    STACKLOCAL_ASSUMEPOD_STACK(FBIHCandidate_, candidates, ApproxBIHTreeDepth_(indices.size()));

    do {
        Assert(it.Node);
        Assert(it.Begin < it.End);
        Assert(it.End <= indices.size());
        Assert(it.Grid.HasPositiveExtents());
        Assert(it.Bounds.HasPositiveExtents());
        Assert(it.Grid.Intersects(it.Bounds));

        size_t iteration = 0;

        const size_t count = (it.End - it.Begin);
        Assert(count > 0);

    SPLIT_LONGEST_AXIS:
        iteration++;
        Assert(iteration <= 3);

        const size_t axis = it.Grid.Extents().MaxComponentIndex();
        const float splitD = (it.Grid.Min()[axis] + it.Grid.Max()[axis]) * 0.5f;

        float clip0 = -FLT_MAX;
        float clip1 = FLT_MAX;

        u32 pivot = it.End;
        for (size_t i = it.Begin; i < pivot; ) {
            const size_t p = indices[i];
            const FBoundingBox& aabb = aabbs[p];

            const float minD = aabb.Min()[axis];
            const float maxD = aabb.Max()[axis];
            const float clipD = (minD + maxD) * 0.5f;

            if (clipD <= splitD) {
                clip0 = Max(clip0, maxD);
                i++;
            }
            else {
                clip1 = Min(clip1, minD);
                std::swap(indices[i], indices[--pivot]);
            }
        }

        if (count <= maxItemsPerLeaf) {
            // ignore empty child on leaf nodes
        }
        else if (pivot == it.End) { // empty child 1
            it.Grid.Max()[axis] = splitD;
            goto SPLIT_LONGEST_AXIS;
        }
        else if (it.Begin == pivot) { // empty child 0
            it.Grid.Min()[axis] = splitD;
            goto SPLIT_LONGEST_AXIS;
        }

        it.Node->Axis = axis;
        Assert(it.Node->Axis == axis);

        const float node0 = it.Bounds.Min()[axis];
        const float node1 = it.Bounds.Max()[axis];

        it.Node->Clip0 = QuantizeClip0_(clip0, node0, node1);
        it.Node->Clip1 = QuantizeClip1_(clip1, node0, node1);
        Assert(UnquantizeClip0_(it.Node->Clip0, node0, node1) >= clip0);
        Assert(UnquantizeClip1_(it.Node->Clip1, node0, node1) <= clip1);

        it.Node->Split = pivot;
        Assert(it.Node->Split == pivot);

        if (count <= maxItemsPerLeaf * 2) { // make leaf
            it.Node->SetLeaf();
            Assert(it.Node->IsLeaf());

#if USE_PPE_ASSERT
            {
                FBoundingBox bounds0 = it.Bounds;
                bounds0.Max()[axis] = UnquantizeClip0_(it.Node->Clip0, node0, node1);
                forrange(i, it.Begin, it.Node->Split)
                    Assert(bounds0.Contains(aabbs[i]));
            }
            {
                FBoundingBox bounds1 = it.Bounds;
                bounds1.Min()[axis] = UnquantizeClip1_(it.Node->Clip1, node0, node1);
                forrange(i, it.Node->Split, it.End)
                    Assert(bounds1.Contains(aabbs[i]));
            }
#endif
        }
        else { // continue to split
            Assert(it.Node->IsLeaf());

            FBIHCandidate_& child0 = *candidates.Push_Uninitialized();
            FBIHCandidate_& child1 = *candidates.Push_Uninitialized();

            child0.Begin = it.Begin;
            child0.End = pivot;

            child1.Begin = pivot;
            child1.End = it.End;

            child0.Node = allocator_traits::AllocateOneT<FBIHNode>(*this);
            child1.Node = allocator_traits::AllocateOneT<FBIHNode>(*this);
            Assert(child1.Node == 1 + child0.Node); // necessary for packing in FBIHNode

            const ptrdiff_t offsetToChild0 = (child0.Node - it.Node);
            Assert(offsetToChild0 > 0 && size_t(offsetToChild0) < FBIHNode::MaxNodes);
            it.Node->Child0 = u32(offsetToChild0);
            Assert(it.Node + it.Node->Child0 == child0.Node);
            Assert(it.Node + it.Node->Child0 + 1 == child1.Node);

            child0.Grid = child1.Grid = it.Grid;
            child0.Grid.Max()[axis] = splitD;
            child1.Grid.Min()[axis] = splitD;

            child0.Bounds = child1.Bounds = it.Bounds;
            child0.Bounds.Max()[axis] = UnquantizeClip0_(it.Node->Clip0, node0, node1);
            child1.Bounds.Min()[axis] = UnquantizeClip1_(it.Node->Clip1, node0, node1);
        }

        Assert(count > maxItemsPerLeaf || it.Node->IsLeaf());

    } while (candidates.Pop(&it));
}
//----------------------------------------------------------------------------
void FBasicBIHTree::Clear() {
    if (nullptr == _root)
        return;

    STACKLOCAL_POD_STACK(FBIHNode*, nodes, 32);

    do {
        Assert(_root);

        if (_root->IsInternal()) {
            // reverse allocation order :
            nodes.Push(_root + _root->Child0);
            nodes.Push(_root + _root->Child0 + 1);
        }

        allocator_traits::DeallocateOneT<FBIHNode>(*this, _root);

    } while (nodes.Pop(&_root));
}
//----------------------------------------------------------------------------
bool FBasicBIHTree::Intersects(const FRay& ray, size_t count, const raycast_item_delegate& raycast) const {
    return Intersects(ray, nullptr, count, raycast);
}
//----------------------------------------------------------------------------
bool FBasicBIHTree::Intersects(const FRay& ray, FHitResult* firstHit, size_t count, const raycast_item_delegate& raycast) const {
    Assert(_root);
    Assert(raycast);

    if (0 == count)
        return false;

    struct FBIHCandidate_ {
        float Distance;
        const FBIHNode* Node;
        FBoundingBox Bounds;
        u32 Begin, End;
    };

    struct FClosestBIHCandidate_ {
        bool operator ()(const FBIHCandidate_& lhs, const FBIHCandidate_& rhs) const {
            return (rhs.Distance < lhs.Distance);
        }
    };

    float distance;
    FHitResult hit{ FLT_MAX, (decltype(firstHit->Item))-1 };
    FBIHCandidate_ it{ FLT_MAX, _root, _bounds, 0, checked_cast<u32>(count) };
    STACKLOCAL_ASSUMEPOD_HEAP(FBIHCandidate_, FClosestBIHCandidate_{}, candidates, ApproxBIHTreeDepth_(count));

    do {
        Assert(it.Node);
        Assert(it.Bounds.HasPositiveExtents());
        Assert(it.Begin < it.End);

        bool intersectsLeaf = false;

        const size_t axis = it.Node->Axis;
        const u32 split = it.Node->Split;
        Assert(it.Begin <= split);
        Assert(it.End >= split);

        const float node0 = it.Bounds.Min()[axis];
        const float node1 = it.Bounds.Max()[axis];

        FBoundingBox bounds0 = it.Bounds, bounds1 = it.Bounds;
        bounds0.Max()[axis] = UnquantizeClip0_(it.Node->Clip0, node0, node1);
        bounds1.Min()[axis] = UnquantizeClip1_(it.Node->Clip1, node0, node1);

        if (ray.Intersects(bounds1, &distance)) {
            if (it.Node->IsLeaf())
                intersectsLeaf |= raycast(ray, it.Node->Split, it.End, &hit);
            else
                candidates.Push(distance, it.Node + it.Node->Child0 + 1, bounds1, split, it.End);
        }

        if (ray.Intersects(bounds0, &distance)) {
            if (it.Node->IsLeaf())
                intersectsLeaf |= raycast(ray, it.Begin, it.Node->Split, &hit);
            else
                candidates.Push(distance, it.Node + it.Node->Child0, bounds0, it.Begin, split);
        }

        if (intersectsLeaf) {
            Assert(hit.Distance < FLT_MAX);
            Assert(hit.Item < count);

            if (nullptr == firstHit)
                return true; // don't need to find closest hit

            const FBIHCandidate_* nextBest = candidates.PeekMin();
            if (nextBest && nextBest->Distance > hit.Distance)
                break;
        }

    } while (candidates.PopMin(&it));

    if (hit.Item < count) {
        if (firstHit) *firstHit = hit;
        return true;
    }
    else {
        return false;
    }
}
//----------------------------------------------------------------------------
bool FBasicBIHTree::Intersects(const FRay& ray, const onhit_delegate& onHit, size_t count, const raycast_onhit_delegate& raycast) const {
    Assert(_root);
    Assert(onHit);
    Assert(raycast);

    if (0 == count)
        return false;

    struct FBIHCandidate_ {
        const FBIHNode* Node;
        FBoundingBox Bounds;
        u32 Begin, End;
    };

    FBIHCandidate_ it{ _root, _bounds, 0, checked_cast<u32>(count) };
    STACKLOCAL_ASSUMEPOD_STACK(FBIHCandidate_, candidates, ApproxBIHTreeDepth_(count));

    bool intersected = false;

    do {
        Assert(it.Node);
        Assert(it.Bounds.HasPositiveExtents());
        Assert(it.Begin < it.End);

        const size_t axis = it.Node->Axis;
        const u32 split = it.Node->Split;
        Assert(it.Begin <= split);
        Assert(it.End >= split);

        const float node0 = it.Bounds.Min()[axis];
        const float node1 = it.Bounds.Max()[axis];

        FBoundingBox bounds0 = it.Bounds, bounds1 = it.Bounds;
        bounds0.Max()[axis] = UnquantizeClip0_(it.Node->Clip0, node0, node1);
        bounds1.Min()[axis] = UnquantizeClip1_(it.Node->Clip1, node0, node1);

        if (ray.Intersects(bounds1)) {
            if (it.Node->IsLeaf())
                intersected |= raycast(ray, it.Node->Split, it.End, onHit);
            else
                candidates.Push(it.Node + it.Node->Child0 + 1, bounds1, split, it.End);
        }

        if (ray.Intersects(bounds0)) {
            if (it.Node->IsLeaf())
                intersected |= raycast(ray, it.Begin, it.Node->Split, onHit);
            else
                candidates.Push(it.Node + it.Node->Child0, bounds0, it.Begin, split);
        }

    } while (candidates.Pop(&it));

    return intersected;
}
//----------------------------------------------------------------------------
size_t FBasicBIHTree::BatchIntersects(const TMemoryView<const FRay>& rays, const TMemoryView<FHitResult>& firstHits, size_t count, const raycast_item_delegate& raycast) const {
    Assert(_root);
    Assert(raycast.Valid());
    Assert(rays.size() == firstHits.size() || firstHits.empty());

    if (0 == count)
        return 0;

    if (rays.size() == 1)
        return (Intersects(rays.front(), (firstHits.empty() ? &firstHits.front() : nullptr), count, raycast) ? 1 : 0);

    struct FBIHCandidate_ {
        const FBIHNode* Node;
        FBoundingBox Bounds;
        u16 RaysBegin, RaysEnd;
        u32 ItemsBegin, ItemsEnd;
    };

    FBIHCandidate_ it{ _root, _bounds, 0, checked_cast<u16>(rays.size()), 0, checked_cast<u32>(count) };
    STACKLOCAL_ASSUMEPOD_STACK(FBIHCandidate_, candidates, 32);

    STACKLOCAL_POD_ARRAY(u16, rayIndices, rays.size());
    forrange(i, it.RaysBegin, it.RaysEnd)
        rayIndices[i] = i;

    for (FHitResult& hit : firstHits) {
        hit.Distance = FLT_MAX;
        hit.Item = (decltype(hit.Item))-1;
    }

    size_t intersections = 0;

    do {
        Assert(it.Node);
        Assert(it.Bounds.HasPositiveExtents());
        Assert(it.RaysBegin < it.RaysEnd);
        Assert(it.ItemsBegin < it.ItemsEnd);

        const size_t axis = it.Node->Axis;
        const u32 split = it.Node->Split;
        Assert(it.ItemsBegin <= split);
        Assert(it.ItemsEnd >= split);

        const float node0 = it.Bounds.Min()[axis];
        const float node1 = it.Bounds.Max()[axis];

        FBoundingBox bounds0 = it.Bounds, bounds1 = it.Bounds;
        bounds0.Max()[axis] = UnquantizeClip0_(it.Node->Clip0, node0, node1);
        bounds1.Min()[axis] = UnquantizeClip1_(it.Node->Clip1, node0, node1);

        if (it.Node->IsLeaf()) {
            forrange(i, it.RaysBegin, it.RaysEnd) {
                const size_t rayIndex = rayIndices[i];
                const FRay& ray = rays[rayIndex];
                FHitResult& hit = firstHits[rayIndex];

                const bool notHitted = (hit.Item < count);

                if ((ray.Intersects(bounds0) && raycast(ray, it.ItemsBegin, it.Node->Split, &hit)) ||
                    (ray.Intersects(bounds1) && raycast(ray, it.Node->Split, it.ItemsEnd, &hit)) ) {

                    if (notHitted)
                        ++intersections;
                }
            }
        }
        else {
            u32 raysSplit0 = it.RaysEnd;
            for (u32 i = it.RaysBegin; i < raysSplit0; ) {
                const FRay& ray = rays[rayIndices[i]];

                if (ray.Intersects(bounds0))
                    i++;
                else
                    std::swap(rayIndices[i], rayIndices[--raysSplit0]);
            }

            u32 raysSplit1 = raysSplit0;
            for (u32 i = it.RaysBegin; i < raysSplit1; ) {
                const FRay& ray = rays[rayIndices[i]];

                if (ray.Intersects(bounds1))
                    i++;
                else
                    std::swap(rayIndices[i], rayIndices[--raysSplit1]);
            }

            u32 raysSplit2 = it.RaysEnd;
            for (u32 i = raysSplit0; i < raysSplit2; ) {
                const FRay& ray = rays[rayIndices[i]];

                if (ray.Intersects(bounds1))
                    i++;
                else
                    std::swap(rayIndices[i], rayIndices[--raysSplit2]);
            }

            Assert(raysSplit1 <= raysSplit0);
            Assert(raysSplit0 <= raysSplit2);

            if (it.RaysBegin < raysSplit1) { // touch both children
                candidates.Push(it.Node + it.Node->Child0 + 1, bounds1, it.RaysBegin, (u16)raysSplit1, it.Node->Split, it.ItemsEnd);
                candidates.Push(it.Node + it.Node->Child0, bounds0, it.RaysBegin, (u16)raysSplit1, it.ItemsBegin, it.Node->Split);
            }

            if (raysSplit0 < raysSplit2) { // touch only child 1
                candidates.Push(it.Node + it.Node->Child0 + 1, bounds1, (u16)raysSplit0, (u16)raysSplit2, it.Node->Split, it.ItemsEnd);
            }

            if (raysSplit1 < raysSplit0) { // touch only child 0
                candidates.Push(it.Node + it.Node->Child0, bounds0, (u16)raysSplit1, (u16)raysSplit0, it.ItemsBegin, it.Node->Split);
            }

            // skips [raySplit2, it.ItemsEnd] which don't touch anything
        }

    } while (candidates.Pop(&it));

    return intersections;
}
//----------------------------------------------------------------------------
size_t FBasicBIHTree::Intersects(const FBoundingBox& box, const hitrange_delegate& onhit, size_t count) const {
    Assert(_root);
    Assert(onhit.Valid());
    Assert(box.HasPositiveExtentsStrict());

    struct FBIHBox_ {
        FBoundingBox Box;
        EContainmentType operator ()(const FBoundingBox& box) const {
            bool inside = false;
            return (Box.Intersects(box, &inside)
                ? (inside ? EContainmentType::Contains : EContainmentType::Intersects)
                : EContainmentType::Disjoint );
        }
    };

    return CullBIHTree_(FBIHBox_{ box }, onhit, count, _root, _bounds);
}
//----------------------------------------------------------------------------
size_t FBasicBIHTree::Intersects(const FFrustum& frustum, const hitrange_delegate& onhit, size_t count) const {
    Assert(_root);
    Assert(onhit.Valid());

    struct FBIHFrustum_ {
        FFrustum Frustum;
        EContainmentType operator ()(const FBoundingBox& box) const {
            return Frustum.Contains(box);
        }
    };

    return CullBIHTree_(FBIHFrustum_{ frustum }, onhit, count, _root, _bounds);
}
//----------------------------------------------------------------------------
size_t FBasicBIHTree::Intersects(const FSphere& sphere, const hitrange_delegate& onhit, size_t count) const {
    Assert(_root);
    Assert(onhit.Valid());
    Assert(sphere.Radius() > 0);

    struct FBIHSphere_ {
        FSphere Sphere;
        EContainmentType operator ()(const FBoundingBox& box) const {
            return Sphere.Contains(box);
        }
    };

    return CullBIHTree_(FBIHSphere_{ sphere }, onhit, count, _root, _bounds);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <>
struct PPE_CORE_API TBIHTraits<FBoundingBox> {
    static FBoundingBox MakeBounds(const FBoundingBox& volume) { return volume; }
    static bool Intersects(const FRay& ray, const FBoundingBox& volume, float* distance) {
        return ray.Intersects(volume, distance);
    }
};
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBIHTree<FBoundingBox, TBIHTraits<FBoundingBox> >;
//----------------------------------------------------------------------------
template <>
struct PPE_CORE_API TBIHTraits<FSphere> {
    static FBoundingBox MakeBounds(const FSphere& volume) { return volume.ToBox(); }
    static bool Intersects(const FRay& ray, const FSphere& volume, float* distance) {
        return ray.Intersects(volume, distance);
    }
};
EXTERN_TEMPLATE_CLASS_DEF(PPE_CORE_API) TBIHTree<FSphere, TBIHTraits<FSphere> >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
