#include "stdafx.h"

#include "ConvexHull.h"

#include "Collision.h"
#include "ScalarVector.h"
#include "ScalarVectorHelpers.h"

#include "Container/Stack.h"
#include "Container/Vector.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
size_t LeastImportantEdge_(const TMemoryView<const float2>& hull, float2& collapsed) {
    Assert(hull.size() > 3);
    const size_t n = hull.size();

    float areaMin = FLT_MAX;
    size_t edgeIndex = n;
    forrange(i, 0, n) {
        const float2& a = hull[(i - 1) % n];
        const float2& b = hull[(i + 0) % n];
        const float2& c = hull[(i + 1) % n];
        const float2& d = hull[(i + 2) % n];

        float2 it;
        if (Collision::LineIntersectsLine(a, b, c, d, it)) {
            const float twiceArea = Abs(Cross(b, c, it));
            if (twiceArea < areaMin) {
                areaMin = twiceArea;
                edgeIndex = i;
                collapsed = it;
            }
        }
    }

    Assert(edgeIndex < n);
    return edgeIndex;
}
//----------------------------------------------------------------------------
size_t LongestEdge_(const TMemoryView<const float2>& hull, float2& center) {
    Assert(hull.size() > 3);
    const size_t n = hull.size();

    float lengthSqMax = 0.0f;
    size_t edgeIndex = n;
    forrange(i, 0, n) {
        const float2& a = hull[(i + 0) % n];
        const float2& b = hull[(i + 1) % n];

        const float lengthSq = DistanceSq2(a, b);
        if (lengthSqMax < lengthSq) {
            lengthSqMax = lengthSq;
            edgeIndex = i;
            center = (a + b) * 0.5f;
        }
    }

    Assert(edgeIndex < n);
    return edgeIndex;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain#C.2B.2B
//----------------------------------------------------------------------------
size_t ConvexHull2D_MonotoneChain(const TMemoryView<size_t>& hull, const TMemoryView<const float2>& points) {
    AssertRelease(points.size() > 2);
    Assert(hull.size() == points.size());

    if (points.size() == 3) {
        hull[0] = 0;
        hull[1] = 1;
        hull[2] = 2;
        return 3;
    }

    STACKLOCAL_POD_ARRAY(size_t, indices, points.size());

    forrange(i, 0, indices.size())
        indices[i] = i;

    // Lexicographic sort
    std::sort(indices.begin(), indices.end(), [&points](size_t a, size_t b) {
        return  (points[a].x() < points[b].x()) ||
                (points[a].x() == points[b].x() && points[a].y() < points[b].y());
    });

    size_t count = 0;

    // Build lower hull
    forrange(i, 0, points.size()) {
        while (count > 1 && Cross(points[hull[count - 2]], points[hull[count - 1]], points[indices[i]]) >= 0)
            count--;

        hull[count++] = indices[i];
    }

    // Build upper hull
    reverseforrange(i, 0, points.size() - 1) {
        while (count > 1 && Cross(points[hull[count - 2]], points[hull[count - 1]], points[indices[i]]) >= 0)
            count--;

        hull[count++] = indices[i];
    }

    Assert(count <= points.size());
    Assert(count > 2);
    return count;
}
//----------------------------------------------------------------------------
void ConvexHull2D_FixedSize(const TMemoryView<float2>& hull, const TMemoryView<const float2>& points) {
    AssertRelease(hull.size() <= points.size());
    Assert(hull.size() > 2);

    const size_t wantedSize = hull.size();

    STACKLOCAL_POD_ARRAY(size_t, indices, points.size());
    const size_t indicesSize = ConvexHull2D_MonotoneChain(indices, points);

    if (indicesSize == wantedSize) {
        forrange(i, 0, wantedSize)
            hull[i] = points[indices[i]];

        return;
    }

    VECTOR_THREAD_LOCAL(Maths, float2) tmp;
    tmp.reserve(indicesSize);
    for (size_t i : indices.CutBefore(indicesSize))
        tmp.push_back(points[i]);

    if (wantedSize < indicesSize) {
        do {
            float2 collapsed;
            const size_t i = LeastImportantEdge_(tmp.MakeConstView(), collapsed);
            tmp.erase(tmp.begin() + i);
            tmp[i % tmp.size()] = collapsed;
        }
        while (wantedSize < tmp.size());
    }
    else {
        Assert(wantedSize > indicesSize);
        do {
            float2 center;
            const size_t i = LongestEdge_(tmp.MakeConstView(), center);
            tmp.insert(tmp.begin() + ((i + 1) % tmp.size()), center);
        }
        while (wantedSize > tmp.size());
    }

    Assert(wantedSize == tmp.size());
    forrange(i, 0, wantedSize)
        hull[i] = tmp[i];
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
