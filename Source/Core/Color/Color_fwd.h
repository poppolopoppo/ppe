#pragma once

#include "Core/Core.h"

#include "Core/Maths/ScalarVector_fwd.h"
#include "Core/Maths/PackingHelpers.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TBasicColorData = TScalarVector<T, 4>;
//----------------------------------------------------------------------------
template <typename T, typename _Shuffle>
class TBasicColor;
//----------------------------------------------------------------------------
struct FColorShuffleRGBA;
struct FColorShuffleBGRA;
//----------------------------------------------------------------------------
typedef TBasicColor< TUNorm<u8>, FColorShuffleBGRA > ColorBGRA;
typedef TBasicColor< TUNorm<u8>, FColorShuffleRGBA > ColorRGBA;
//----------------------------------------------------------------------------
typedef TBasicColor< TUNorm<u16>, FColorShuffleBGRA > ColorBGRA16;
typedef TBasicColor< TUNorm<u16>, FColorShuffleRGBA > ColorRGBA16;
//----------------------------------------------------------------------------
typedef TBasicColor< float, FColorShuffleBGRA > ColorBGRAF;
typedef TBasicColor< float, FColorShuffleRGBA > ColorRGBAF;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
