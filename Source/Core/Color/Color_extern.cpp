#include "stdafx.h"

#include "Color.h"

#include "Color_extern.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template class BasicColor< UNorm<u8>, ColorShuffleBGRA >;
template class BasicColor< UNorm<u8>, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
template class BasicColor< UNorm<u16>, ColorShuffleBGRA >;
template class BasicColor< UNorm<u16>, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
template class BasicColor< float, ColorShuffleBGRA >;
template class BasicColor< float, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
