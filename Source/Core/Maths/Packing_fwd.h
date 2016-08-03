#pragma once

#include "Core/Core.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
class ScalarVector;
//----------------------------------------------------------------------------
struct UX10Y10Z10W2N;
//----------------------------------------------------------------------------
struct HalfFloat;
typedef HalfFloat half;
typedef ScalarVector<HalfFloat, 2> half2;
typedef ScalarVector<HalfFloat, 4> half4;
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
struct BasicNorm;
template <typename T>
struct UNormTraits;
template <typename T>
struct SNormTraits;
//----------------------------------------------------------------------------
template <typename T>
using UNorm = BasicNorm<T, UNormTraits<T> >;
typedef UNorm<u8> ubyten;
typedef UNorm<u16> ushortn;
typedef UNorm<u32> uwordn;
//----------------------------------------------------------------------------
template <typename T>
using SNorm = BasicNorm<T, SNormTraits<T> >;
typedef SNorm<u8> byten;
typedef SNorm<u16> shortn;
typedef SNorm<u32> wordn;
//----------------------------------------------------------------------------
typedef ScalarVector<SNorm<u8>, 2> byte2n;
typedef ScalarVector<SNorm<u8>, 4> byte4n;
typedef ScalarVector<UNorm<u8>, 2> ubyte2n;
typedef ScalarVector<UNorm<u8>, 4> ubyte4n;
//----------------------------------------------------------------------------
typedef ScalarVector<SNorm<u16>, 2> short2n;
typedef ScalarVector<SNorm<u16>, 4> short4n;
typedef ScalarVector<UNorm<u16>, 2> ushort2n;
typedef ScalarVector<UNorm<u16>, 4> ushort4n;
//----------------------------------------------------------------------------
typedef ScalarVector<SNorm<u32>, 2> word2n;
typedef ScalarVector<SNorm<u32>, 3> word3n;
typedef ScalarVector<SNorm<u32>, 4> word4n;
typedef ScalarVector<UNorm<u32>, 2> uword2n;
typedef ScalarVector<UNorm<u32>, 3> uword3n;
typedef ScalarVector<UNorm<u32>, 4> uword4n;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
