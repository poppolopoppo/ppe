#pragma once

#include "Core/Core.h"

#include "Core/Maths/Geometry/ScalarVector_fwd.h"
#include "Core/Maths/Packing/PackingHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using BasicColorData = ScalarVector<T, 4>;
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
class BasicColor;
//----------------------------------------------------------------------------
struct ColorShuffleRGBA;
struct ColorShuffleBGRA;
//----------------------------------------------------------------------------
typedef BasicColor< UNorm<u8>, ColorShuffleBGRA > ColorBGRA;
typedef BasicColor< UNorm<u8>, ColorShuffleRGBA > ColorRGBA;
//----------------------------------------------------------------------------
typedef BasicColor< UNorm<u16>, ColorShuffleBGRA > ColorBGRA16;
typedef BasicColor< UNorm<u16>, ColorShuffleRGBA > ColorRGBA16;
//----------------------------------------------------------------------------
typedef BasicColor< float, ColorShuffleBGRA > ColorBGRAF;
typedef BasicColor< float, ColorShuffleRGBA > ColorRGBAF;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
