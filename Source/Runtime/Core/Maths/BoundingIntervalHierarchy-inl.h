#pragma once

#include "Core/Maths/BoundingIntervalHierarchy.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
TBIHTree<T, _Traits>::TBIHTree(const TMemoryView<T>& items)
    : _items(items) {
    RebuildTree();
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
TBIHTree<T, _Traits>::TBIHTree(const TMemoryView<T>& items, const traits_type& traits)
    : traits_type(traits)
    , _items(items) {
    RebuildTree();
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
TBIHTree<T, _Traits>::TBIHTree(TBIHTree&& rvalue)
    : _bih(std::move(rvalue._bih))
    , _items(std::move(rvalue._items))
{}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
auto TBIHTree<T, _Traits>::operator =(TBIHTree&& rvalue) -> TBIHTree& {
    _bih.operator =(std::move(rvalue._bih));
    _items.operator =(std::move(rvalue._items));
    return (*this);
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
bool TBIHTree<T, _Traits>::Intersects(const FRay& ray) const {
    return _bih.Intersects(ray, _items.size(), MakeFunction(this, &TBIHTree::IntersectsFirstHit_));
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
bool TBIHTree<T, _Traits>::Intersects(const FRay& ray, FHitResult* firstHit) const {
    return _bih.Intersects(ray, firstHit, _items.size(), MakeFunction(this, &TBIHTree::IntersectsFirstHit_));
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
bool TBIHTree<T, _Traits>::Intersects(const FRay& ray, const onhit_delegate& onHit) const {
    return _bih.Intersects(ray, onHit, _items.size(), MakeFunction(this, &TBIHTree::IntersectsOnEachHit_));
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
size_t TBIHTree<T, _Traits>::BatchIntersects(const TMemoryView<const FRay>& rays, const TMemoryView<FHitResult>& firstHits) const {
    return _bih.BatchIntersects(rays, firstHits, _items.size(), MakeFunction(this, &TBIHTree::IntersectsFirstHit_));
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
size_t TBIHTree<T, _Traits>::Intersects(const FBoundingBox& box, const hitrange_delegate& onhit) const {
    return _bih.Intersects(box, onhit, _items.size());
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
size_t TBIHTree<T, _Traits>::Intersects(const FFrustum& frustum, const hitrange_delegate& onhit) const {
    return _bih.Intersects(frustum, onhit, _items.size());
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
size_t TBIHTree<T, _Traits>::Intersects(const FSphere& sphere, const hitrange_delegate& onhit) const {
    return _bih.Intersects(sphere, onhit, _items.size());
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
void TBIHTree<T, _Traits>::RebuildTree(size_t maxItemsPerLeaf/* = DefaultMaxItemsPerLeaf */) {
    _bih.Clear();

    if (_items.empty())
        return;

    FBoundingBox bounds;

    STACKLOCAL_ASSUMEPOD_ARRAY(FBoundingBox, aabbs, _items.size());
    forrange(i, 0, _items.size())
        bounds.Add(aabbs[i] = traits_type::MakeBounds(_items[i]));

    STACKLOCAL_POD_ARRAY(u32, indices, _items.size());
    forrange(i, 0, checked_cast<u32>(_items.size()))
        indices[i] = i;

    _bih.Build(maxItemsPerLeaf, bounds, indices, aabbs);

    ReindexMemoryView(_items, indices);
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
void TBIHTree<T, _Traits>::RebuildTree(const TMemoryView<T>& items, size_t maxItemsPerLeaf/* = DefaultMaxItemsPerLeaf */) {
    _items = items;

    RebuildTree(maxItemsPerLeaf);
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
bool TBIHTree<T, _Traits>::IntersectsFirstHit_(const FRay& ray, size_t begin, size_t end, FHitResult* firstHit) const {
    bool intersected = false;
    forrange(i, begin, end) {
        float d;
        if (traits_type::Intersects(ray, _items[i], &d) && firstHit->Distance > d) {
            intersected = true;
            firstHit->Distance = d;
            firstHit->Item = checked_cast<u32>(i);
        }
    }
    return intersected;
}
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
bool TBIHTree<T, _Traits>::IntersectsOnEachHit_(const FRay& ray, size_t begin, size_t end, const onhit_delegate& onHit) const {
    FHitResult hit;
    bool intersected = false;
    forrange(i, begin, end) {
        if (traits_type::Intersects(ray, _items[i], &hit.Distance)) {
            intersected = true;
            hit.Item = checked_cast<u32>(i);
            onHit(hit);
        }
    }
    return intersected;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
