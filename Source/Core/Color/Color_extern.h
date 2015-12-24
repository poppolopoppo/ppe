#pragma once

#include "Core/Core.h"

#include "Core/Color/Color_fwd.h"
#include "Core/Maths/Geometry/ScalarVector_extern.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
extern template class BasicColor< UNorm<u8>, ColorShuffleBGRA >;
extern template class BasicColor< UNorm<u8>, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
extern template class BasicColor< UNorm<u16>, ColorShuffleBGRA >;
extern template class BasicColor< UNorm<u16>, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
extern template class BasicColor< float, ColorShuffleBGRA >;
extern template class BasicColor< float, ColorShuffleRGBA >;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
