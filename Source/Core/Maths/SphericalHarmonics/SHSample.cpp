#include "stdafx.h"

#include "SHSample.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static SHScalar SHAssociatedLegendrePolynomial_(int l, int m, SHScalar x) {
    SHScalar pmm(1);
    if (m > 0) {
        const SHScalar somx2 = std::sqrt( (1-x)*(1+x) );
        SHScalar fact(1);
        for(int i = 1; i <= m; ++i) {
            pmm *= (-fact)*somx2;
            fact += 2;
        }
    }

    if (l == m)
        return pmm;

    SHScalar pmmpl = x*(2*SHScalar(m)+1)*pmm;

    if (l==m+1)
        return pmmpl;

    SHScalar pll(0);
    for(int ll = m + 2; ll <= l; ++ll) {
        pll = ( (2*SHScalar(ll)-1)*x*pmmpl-(SHScalar(ll)+SHScalar(m)-1)*pmm )/(SHScalar(ll)-SHScalar(m));
        pmm = pmmpl;
        pmmpl = pll;
    }

    return pll;
}
//----------------------------------------------------------------------------
static SHScalar SHFactorial_(int x) {
#ifndef WITH_CORE_SPHERICALHARMONICS_DBLPRECISION
    Assert(x < 16); // switch to double precision !
    // use double precision if you plan to use SH with more than 3 bands.
#endif

    static const SHScalar gPrecomputedFactorial[16] = {
        SHScalar(1.0), SHScalar(1.0), SHScalar(2.0), SHScalar(6.0), SHScalar(24.0),
        SHScalar(120.0), SHScalar(720.0), SHScalar(5040.0), SHScalar(40320.0), SHScalar(362880.0),
        SHScalar(3628800.0), SHScalar(39916800.0), SHScalar(479001600.0), SHScalar(6227020800.0), SHScalar(87178291200.0), SHScalar(1307674368000.0) };

    if (x < lengthof(gPrecomputedFactorial))
        return gPrecomputedFactorial[x];    //return precomputed value

    //return non precomputed value starting using the last precomputed value in the array
    SHScalar result = gPrecomputedFactorial[lengthof(gPrecomputedFactorial)-1];
    for (int i = lengthof(gPrecomputedFactorial); i <= x; ++i)
        result = result *= i;

    return result;
}
//----------------------------------------------------------------------------
static SHScalar SHAssiociatedLegendrePolynomialScalingFactor_(int l, int m) {
    const SHScalar tmp = ( (2*SHScalar(l)+1)*SHFactorial_(l-m) )/( SHScalar(4.0*3.1415926535897932384626433832795)*SHFactorial_(l+m) );
    return std::sqrt(tmp);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SHScalar SHPointSampleFunction(int l, int m, const SHSphericalCoord& thetaPhi) {
    const SHScalar sqrt2 = static_cast<SHScalar>(1.41421356237);//std::sqrt(2.0);
    if (m == 0)
        return SHAssiociatedLegendrePolynomialScalingFactor_(l, 0)*SHAssociatedLegendrePolynomial_(l, m, std::cos(thetaPhi.x()));
    else if (m > 0)
        return sqrt2*SHAssiociatedLegendrePolynomialScalingFactor_(l, m)*std::cos(SHScalar(m)*thetaPhi.y())*SHAssociatedLegendrePolynomial_(l, m, std::cos(thetaPhi.x()));
    else
        return sqrt2*SHAssiociatedLegendrePolynomialScalingFactor_(l, -m)*std::sin(-SHScalar(m)*thetaPhi.y())*SHAssociatedLegendrePolynomial_(l, -m, std::cos(thetaPhi.x()));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
