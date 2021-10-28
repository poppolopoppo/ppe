#pragma once

#include "RHIVulkan_fwd.h"

#include "Container/Vector.h"
#include "Maths/Range.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename _Access, typename _Range>
struct TVulkanAccessRecords {
    using FAccess = _Access;
    using FRange = TRange<_Range>;
    using FRecords = VECTORINSITU(RHIVulkan, _Access, 3);
    using FIterator = typename FRecords::iterator;

    FRecords Barriers;

    FRecords& operator *() NOEXCEPT { return Barriers; }
    const FRecords& operator *() const NOEXCEPT { return Barriers; }

    FRecords* operator ->() NOEXCEPT { return &Barriers; }
    const FRecords* operator ->() const NOEXCEPT { return &Barriers; }

    FIterator FindFirstAccess(const FRange& range);
    void ReplaceAccessRecords(FIterator it, const FAccess& barrier);
    FIterator EraseAccessRecords(FIterator it, const FRange& range);
};
//----------------------------------------------------------------------------
template <typename _Access, typename _Range>
auto TVulkanAccessRecords<_Access, _Range>::FindFirstAccess(const FRange& range) -> FIterator {
    Assert(range.First < range.Last);

    size_t left = 0;
    for (size_t right = Barriers.size(); left < right; ) {
        const size_t mid = (left + right) >> 1;

        if (Barriers[mid].Range.Last < range.First)
            left = mid + 1;
        else
            right = mid;
    }

    if (left < Barriers.size() and Barriers[left].Range.Last >= range.First)
        return (Barriers.begin() + left);

    return Barriers.end();
}
//----------------------------------------------------------------------------
template <typename _Access, typename _Range>
void TVulkanAccessRecords<_Access, _Range>::ReplaceAccessRecords(FIterator it, const FAccess& barrier) {
    bool replaced = false;

    while (it != Barriers.end()) {
        if (it->Range.First  < barrier.Range.First &&
            it->Range.Last  <= barrier.Range.Last ) {
            //	|1111111|22222|
            //     |bbbbb|          +
            //  |11|....            =
            it->Range.Last = barrier.Range.First;
            ++it;
            continue;
        }

        if (it->Range.First  < barrier.Range.First &&
            it->Range.Last   > barrier.Range.Last ) {
            //  |111111111111111|
            //      |bbbbb|	        +
            //  |111|bbbbb|11111|   =
            const FAccess cpy = *it;

            it->Range.Last = barrier.Range.First;
            it = Barriers.insert(it + 1, barrier);
            replaced = true;

            it = Barriers.insert(it + 1, cpy);
            it->Range.First = barrier.Range.Last;
            break;
        }

        if (it->Range.First >= barrier.Range.First &&
            it->Range.First  < barrier.Range.Last ) {
            if (it->Range.Last > barrier.Range.Last) {
                //  ...|22222222222|
                //   |bbbbbbb|          +
                //  ...bbbbbb|22222|    =
                it->Range.First = barrier.Range.Last;

                if (not replaced) {
                    Barriers.insert(it, barrier);
                    replaced = true;
                }
                break;
            }

            if (replaced) {
                //  ...|22222|33333|
                //   |bbbbbbbbbbb|      +
                //  ...|bbbbbbbbb|...   =
                it = Barriers.erase(it);
            }
            else {
                *it = barrier;
                ++it;
                replaced = true;
            }
            continue;
        }

        break;
    }

    if (not replaced)
        Barriers.insert(it, barrier);
}
//----------------------------------------------------------------------------
template <typename _Access, typename _Range>
auto TVulkanAccessRecords<_Access, _Range>::EraseAccessRecords(FIterator it, const FRange& range) -> FIterator {
    if (Barriers.empty())
        return it;

    while (it != Barriers.end()) {
        if (it->Range.First  < range.First &&
            it->Range.Last   > range.Last ) {
            const FAccess cpy = *it;
            it->Range.Last = range.First;

            it = Barriers.insert(it + 1, cpy);
            it->Range.First = range.Last;
            break;
        }

        if (it->Range.First  < range.First) {
            Assert(it->Range.Last >= range.First);

            it->Range.Last = range.First;
            ++it;
            continue;
        }

        if (it->Range.Last   > range.Last ) {
            Assert(it->Range.First <= range.Last);

            it->Range.First = range.Last;
            break;
        }

        it = Barriers.erase(it);
    }

    return it;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
