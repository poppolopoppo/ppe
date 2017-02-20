#include "stdafx.h"

#include "BinPacking.h"

#include "ScalarBoundingBox.h"
#include "ScalarBoundingBoxHelpers.h"
#include "ScalarVector.h"
#include "ScalarVectorHelpers.h"

#include "Allocator/Alloca.h"
#include "Container/Vector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void Split_(
    FAabb2f& right,
    FAabb2f& bottom,
    const FAabb2f& parent,
    const float2& box ) {
    Assert(parent.HasPositiveExtentsStrict());
    Assert(parent.Extents().AllGreaterOrEqual(box));

    const float2 bmax = parent.Max();
    const float2 bmin = parent.Min();
    const float2 bcut = bmin + box;

    right.SetMinMax(
        float2(bcut.x(), bmin.y()),
        float2(bmax.x(), bcut.y()) );

    bottom.SetMinMax(
        float2(bmin.x(), bcut.y()),
        float2(bmax.x(), bmax.y()) );

    // Try to minimize narrow features
    if (Area(right) > Area(bottom)) {
        right.SetMinMax(
            float2(bcut.x(), bmin.y()),
            float2(bmax.x(), bmax.y()) );

        bottom.SetMinMax(
            float2(bmin.x(), bcut.y()),
            float2(bcut.x(), bmax.y()) );
    }

    Assert(parent.Contains(right));
    Assert(parent.Contains(bottom));
}
//----------------------------------------------------------------------------
FORCE_INLINE static bool BoxLessHeuristic_(const float2& extents0, const float2& extents1) {
    const float e0 = Max(extents0.x(), extents0.y());
    const float e1 = Max(extents1.x(), extents1.y());
    return ((e1 < e0) || ((e1 ==  e0) &&
            (Min(extents1.x(), extents1.y()) < Min(extents0.x(), extents0.y())) ));
    //return (Max(extents1.x(), extents1.y()) < Max(extents0.x(), extents0.y()) ); // Max(w, h)
    //return ((extents1.x() + extents1.y()) < (extents0.x() + extents0.y())); // perimeter <=> mean
    //return ((extents1.x() * extents1.y()) < (extents0.x() * extents0.y())); // area
    //return extents1.x() < extents0.x(); // width
    //return extents1.y() < extents0.y(); // width
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool BinPacking2D(float2& binsize, const TMemoryView<float2>& offsets, const TMemoryView<const float2>& boxes) {
    Assert(offsets.size() == boxes.size());

    binsize = float2(0);

    if (boxes.empty())
        return true;

    if (boxes.size() == 1) {
        binsize = boxes[0];
        offsets[0] = float2(0);
        return true;
    }

    const size_t n = boxes.size();
    STACKLOCAL_POD_ARRAY(size_t, indices, n);
    forrange(i, 0, n)
        indices[i] = i;

    // Sort from wider to thinner
    std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
        return BoxLessHeuristic_(boxes[a], boxes[b]);
    });

    // Initial size if the size of the biggest box
    FAabb2f root(float2(0), boxes[indices[0]]);

    // Fill recursively the bins with the sorted boxes
    VECTOR_THREAD_LOCAL(Maths, FAabb2f) bins;
    bins.reserve(n);
    bins.push_back(root);

    for (size_t i : indices) {
        const float2 box = boxes[i];
        Assert(box.x() > 0 && box.y() > 0);

        bool found = false;
        forrange(j, 0, bins.size()) {
            FAabb2f bin = bins[j];
            const float2 binExtents = bin.Extents();
            Assert(binExtents.AllGreaterThan(float2(0)));

            if (binExtents.AllGreaterOrEqual(box)) {
                found = true;

                offsets[i] = bin.Min();
                binsize = Max(bin.Min() + box, binsize);

                FAabb2f right, bottom;
                Split_(right, bottom, bin, box);

                if (right.HasPositiveExtentsStrict()) {
                    if (bottom.HasPositiveExtentsStrict()) {
                        if (BoxLessHeuristic_(bottom.Extents(), right.Extents())) {
                            bins[j] = bottom;
                            bins.emplace(bins.begin() + j + 1, right);
                        }
                        else {
                            bins[j] = right;
                            bins.emplace(bins.begin() + j + 1, bottom);
                        }
                    }
                    else {
                        bins[j] = right;
                    }
                }
                else if (bottom.HasPositiveExtentsStrict()) {
                    bins[j] = bottom;
                }
                else {
                    bins.erase(bins.begin() + j);
                }

                break;
            }
        }

        // No block found, the binsize will be increased if possible
        if (not found) {
            const bool canGrowDown  = (box.x() <= root.Extents().x());
            const bool canGrowRight = (box.y() <= root.Extents().y());

            // Attempt to keep square-ish by growing right when height is much greater than width
            const bool shouldGrowRight = canGrowRight && (root.Extents().y() >= (root.Extents().x() + box.x()));
            // Attempt to keep square-ish by growing down  when width  is much greater than height
            const bool shouldGrowDown  = canGrowDown  && (root.Extents().x() >= (root.Extents().y() + box.y()));

            bool willGrowRight = false;
            bool willGrowDown = false;

            if (shouldGrowRight)
                willGrowRight = true;
            else if (shouldGrowDown)
                willGrowDown = true;
            else if (canGrowRight)
                willGrowRight = true;
            else if (canGrowDown)
                willGrowDown = true;
            else
                return false; // We can't grow anymore !

            //  If we can grow in both directions, choose the largest one
            if (willGrowRight && willGrowDown) {
                if (box.x() < box.y()) {
                    willGrowDown = true;
                    willGrowRight = false;
                }
                else {
                    willGrowDown = false;
                    willGrowRight = true;
                }
            }

            FAabb2f growth;
            if (willGrowRight) {
                growth = FAabb2f(
                    float2(root.Max().x(), root.Min().y()),
                    float2(root.Max().x() + box.x(), root.Max().y()) );

                FAabb2f right, bottom;
                Split_(right, bottom, growth, box);

                Assert(not right.HasPositiveExtentsStrict());
                if (bottom.HasPositiveExtentsStrict())
                    bins.push_back(bottom);
            }
            else if (willGrowDown) {
                growth = FAabb2f(
                    float2(root.Min().x(), root.Max().y()),
                    float2(root.Max().x(), root.Max().y() + box.y()) );

                FAabb2f right, bottom;
                Split_(right, bottom, growth, box);

                Assert(not bottom.HasPositiveExtentsStrict());
                if (right.HasPositiveExtentsStrict())
                    bins.push_back(right);
            }
            else {
                AssertNotReached();
            }

            offsets[i] = growth.Min();
            binsize = Max(growth.Min() + box, binsize);

            root = FAabb2f(root.Min(), growth.Max());
        }
    }

    Assert(binsize.AllGreaterThan(float2(0)));
    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
