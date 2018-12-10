#pragma once

#include "Core.h"

#include "Container/RawStorage.h"
#include "Container/Vector.h"
#include "Maths/ScalarVector.h"
#include "Memory/MemoryView.h"

//#define WITH_PPE_SPHERICALHARMONICS_DBLPRECISION //%_NOCOMMIT%
// TODO : template ?

namespace PPE {
class FRandomGenerator;

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
#ifdef WITH_PPE_SPHERICALHARMONICS_DBLPRECISION
typedef double SHScalar;
#else
typedef float SHScalar;
#endif
//----------------------------------------------------------------------------
typedef TScalarVector<SHScalar, 2> SHSphericalCoord; // Theta/Phi
typedef TScalarVector<SHScalar, 3> SHDirection;      // Nx/Ny/Nz
//----------------------------------------------------------------------------
inline SHDirection SHSphericalCoordToDirection(const SHSphericalCoord& thetaPhi) {
    const SHScalar sinTheta = std::sin(thetaPhi.x);
    return SHDirection( sinTheta * std::cos(thetaPhi.y),
                        sinTheta * std::sin(thetaPhi.y),
                        std::cos(thetaPhi.x) );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FSHSample {
    SHSphericalCoord ThetaPhi;
    SHDirection Direction;
    TMemoryView<const SHScalar> Coefficients;
};
//----------------------------------------------------------------------------
SHScalar SHPointSampleFunction(int l, int m, const SHSphericalCoord& thetaPhi);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
