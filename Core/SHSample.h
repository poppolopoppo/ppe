#pragma once

#include "Core.h"

#include "MemoryView.h"
#include "RawStorage.h"
#include "ScalarVector.h"
#include "Vector.h"

//#define WITH_CORE_SPHERICALHARMONICS_DBLPRECISION //%_NOCOMMIT%
// TODO : template ?

namespace Core {
class RandomGenerator;

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// Usefull knowledge about spherical harmonics :
// ---------------------------------------------
// * http://www.research.scea.com/gdc2003/spherical-harmonic-lighting.html
// * http://www.ppsloan.org/publications/StupidSH36.pdf
// * http://cg.tuwien.ac.at/research/publications/2008/Habel_08_SSH/
// * http://sebastien.hillaire.free.fr/index.php?option=com_content&view=article&id=60&Itemid=71
//----------------------------------------------------------------------------
#ifdef WITH_CORE_SPHERICALHARMONICS_DBLPRECISION
typedef double SHScalar;
#else
typedef float SHScalar;
#endif
//----------------------------------------------------------------------------
typedef ScalarVector<SHScalar, 2> SHSphericalCoord; // Theta/Phi
typedef ScalarVector<SHScalar, 3> SHDirection;      // Nx/Ny/Nz
//----------------------------------------------------------------------------
inline SHDirection SHSphericalCoordToDirection(const SHSphericalCoord& thetaPhi) {
    const SHScalar sinTheta = std::sin(thetaPhi.x());
    return SHDirection( sinTheta * std::cos(thetaPhi.y()),
                        sinTheta * std::sin(thetaPhi.y()),
                        std::cos(thetaPhi.x()) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct SHSample {
    SHSphericalCoord ThetaPhi;
    SHDirection Direction;
    MemoryView<const SHScalar> Coefficients;
};
//----------------------------------------------------------------------------
SHScalar SHPointSampleFunction(int l, int m, const SHSphericalCoord& thetaPhi);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
