#pragma once

#include "Core/Core.h"

#include "Core/Allocator/Allocation.h"
#include "Core/Allocator/NodeBasedContainerAllocator.h"
#include "Core/Maths/ScalarBoundingBox.h"
#include "Core/Memory/MemoryView.h"
#include "Core/Misc/Function.h"

namespace Core {
enum class EContainmentType;
class FFrustum;
class FRay;
class FSphere;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBIHNode {
    STATIC_CONST_INTEGRAL(size_t, AxisBits,     2 ); // max value = (1<< 2) - 1 = 3
    STATIC_CONST_INTEGRAL(size_t, Child0Bits,   20); // max value = (1<<20) - 1 = 1048575
    STATIC_CONST_INTEGRAL(size_t, Clip0Bits,    10); // max value = (1<<10) - 1 = 1023
    STATIC_CONST_INTEGRAL(size_t, Clip1Bits,    10); // max value = (1<<10) - 1 = 1023
    STATIC_CONST_INTEGRAL(size_t, SplitBits,    22); // max value = (1<<22) - 1 = 4194303


    STATIC_CONST_INTEGRAL(size_t, AxisMask, (1 << AxisBits) - 1);
    STATIC_CONST_INTEGRAL(size_t, Child0Mask, (1 << Child0Bits) - 1);
    STATIC_CONST_INTEGRAL(size_t, Clip0Mask, (1 << Clip0Bits) - 1);
    STATIC_CONST_INTEGRAL(size_t, Clip1Mask, (1 << Clip1Bits) - 1);
    STATIC_CONST_INTEGRAL(size_t, SplitMask, (1 << SplitBits) - 1);

    u32 Axis    : AxisBits;   //   0 +  2
    u32 Child0  : Child0Bits; //   2 + 20
    u32 Clip0   : Clip0Bits;  //  22 + 10
    u32 Clip1   : Clip1Bits;  //  32 + 10
    u32 Split   : SplitBits;  //  42 + 22
                              //     = 64
    bool IsInternal() const { return (Child0 != Child0Mask); }
    bool IsLeaf() const { return (Child0 == Child0Mask); }
    void SetLeaf() { Child0 = Child0Mask; }

    STATIC_CONST_INTEGRAL(size_t, MaxItems, (1 << SplitBits));
    STATIC_CONST_INTEGRAL(size_t, MaxNodes, (1 << Child0Bits));
};
//----------------------------------------------------------------------------
class CORE_API FBasicBIHTree : NODEBASED_CONTAINER_ALLOCATOR(Maths, FBIHNode) {
    typedef NODEBASED_CONTAINER_ALLOCATOR(Maths, FBIHNode) allocator_type;
public:
    FBasicBIHTree();
    ~FBasicBIHTree();

    FBasicBIHTree(const FBasicBIHTree&) = delete;
    FBasicBIHTree& operator =(const FBasicBIHTree&) = delete;

    FBasicBIHTree(FBasicBIHTree&& rvalue);
    FBasicBIHTree& operator =(FBasicBIHTree&& rvalue);

    const FBoundingBox& Bounds() const { return _bounds; }

    bool empty() const { return (nullptr == _root); }

    struct FHitResult {
        float Distance;
        u32 Item;
    };

    typedef TFunction<void(const FHitResult&/* hit */)> onhit_delegate;
    typedef TFunction<void(size_t/* begin */, size_t/* end */, EContainmentType/* collision */)> hitrange_delegate;
    typedef TFunction<bool(const FRay&/* ray */, size_t/* begin */, size_t/* end */, FHitResult* /* firstHit */)> raycast_item_delegate;
    typedef TFunction<bool(const FRay&/* ray */, size_t/* begin */, size_t/* end */, const onhit_delegate&/* onhit */)> raycast_onhit_delegate;

    void Build(
        size_t maxItemsPerLeaf,
        const FBoundingBox& bounds,
        const TMemoryView<u32>& reindexation,
        const TMemoryView<const FBoundingBox>& aabbs);

    void Clear();

    bool Intersects(const FRay& ray, size_t count, const raycast_item_delegate& raycast) const;
    bool Intersects(const FRay& ray, FHitResult* firstHit, size_t count, const raycast_item_delegate& raycast) const;
    bool Intersects(const FRay& ray, const onhit_delegate& onHit, size_t count, const raycast_onhit_delegate& raycast) const;

    size_t BatchIntersects(const TMemoryView<const FRay>& rays, const TMemoryView<FHitResult>& firstHits, size_t count, const raycast_item_delegate& raycast) const;

    size_t Intersects(const FBoundingBox& box, const hitrange_delegate& onhit, size_t count) const;
    size_t Intersects(const FSphere& sphere, const hitrange_delegate& onhit, size_t count) const;
    size_t Intersects(const FFrustum& frustum, const hitrange_delegate& onhit, size_t count) const;

private:
    FBIHNode* _root;
    FBoundingBox _bounds;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
class TBIHTree;
//----------------------------------------------------------------------------
template <typename T>
struct TBIHTraits {
    static FBoundingBox MakeBounds(const T& volume) = delete;
    static bool Intersects(const FRay& ray, const T& volume, float* distance) = delete;
};
//----------------------------------------------------------------------------
template <typename T, typename _Traits = TBIHTraits<T> >
class TBIHTree : _Traits {
public:
    typedef _Traits traits_type;
    typedef typename FBasicBIHTree::FHitResult FHitResult;
    typedef typename FBasicBIHTree::onhit_delegate onhit_delegate;
    typedef typename FBasicBIHTree::hitrange_delegate hitrange_delegate;

    STATIC_CONST_INTEGRAL(size_t, DefaultMaxItemsPerLeaf, 4);

    explicit TBIHTree(const TMemoryView<T>& items);
    TBIHTree(const TMemoryView<T>& items, const traits_type& traits);

    TBIHTree(const TBIHTree&) = delete;
    TBIHTree& operator =(const TBIHTree&) = delete;

    TBIHTree(TBIHTree&& rvalue);
    TBIHTree& operator =(TBIHTree&& rvalue);

    const FBoundingBox& Bounds() const { return _bih.Bounds(); }
    TMemoryView<const T> Items() const { return _items; }

    bool Intersects(const FRay& ray) const;
    bool Intersects(const FRay& ray, FHitResult* firstHit) const;
    bool Intersects(const FRay& ray, const onhit_delegate& onHit) const;

    size_t BatchIntersects(const TMemoryView<const FRay>& rays, const TMemoryView<FHitResult>& firstHits) const;

    size_t Intersects(const FBoundingBox& box, const hitrange_delegate& onhit) const;
    size_t Intersects(const FFrustum& frustum, const hitrange_delegate& onhit) const;
    size_t Intersects(const FSphere& sphere, const hitrange_delegate& onhit) const;

    void RebuildTree(size_t maxItemsPerLeaf = DefaultMaxItemsPerLeaf);
    void RebuildTree(const TMemoryView<T>& items, size_t maxItemsPerLeaf = DefaultMaxItemsPerLeaf);

private:
    bool IntersectsFirstHit_(const FRay& ray, size_t begin, size_t end, FHitResult* firstHit) const;
    bool IntersectsOnEachHit_(const FRay& ray, size_t begin, size_t end, const onhit_delegate& onHit) const;

    FBasicBIHTree _bih;
    TMemoryView<T> _items;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core

#include "Core/Maths/BoundingIntervalHierarchy-inl.h"

/*
namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <> struct TBIHTraits<FBoundingBox>;
extern CORE_API template struct TBIHTraits<FBoundingBox>;
template <> struct TBIHTraits<FSphere>;
extern CORE_API template struct TBIHTraits<FSphere>;
//----------------------------------------------------------------------------
extern CORE_API template class TBIHTree<FBoundingBox, TBIHTraits<FBoundingBox> >;
typedef TBIHTree<FBoundingBox, TBIHTraits<FBoundingBox> > FBoundingBoxBIHTree;
extern CORE_API template class TBIHTree<FSphere, TBIHTraits<FSphere> >;
typedef TBIHTree<FSphere, TBIHTraits<FSphere> > FSphereBIHTree;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
*/