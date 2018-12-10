#pragma once

#include "Core_fwd.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, size_t _Dim>
struct TScalarVector;
//----------------------------------------------------------------------------
struct UX10Y10Z10W2N;
//----------------------------------------------------------------------------
struct FHalfFloat;
typedef FHalfFloat half;
typedef TScalarVector<FHalfFloat, 2> half2;
typedef TScalarVector<FHalfFloat, 4> half4;
//----------------------------------------------------------------------------
template <typename T, typename _Traits>
struct TBasicNorm;
template <typename T>
struct TUNormTraits;
template <typename T>
struct TSNormTraits;
//----------------------------------------------------------------------------
template <typename T>
using TUNorm = TBasicNorm<T, TUNormTraits<T> >;
typedef TUNorm<u8> ubyten;
typedef TUNorm<u16> ushortn;
typedef TUNorm<u32> uwordn;
//----------------------------------------------------------------------------
template <typename T>
using TSNorm = TBasicNorm<T, TSNormTraits<T> >;
typedef TSNorm<u8> byten;
typedef TSNorm<u16> shortn;
typedef TSNorm<u32> wordn;
//----------------------------------------------------------------------------
typedef TScalarVector<TSNorm<u8>, 2> byte2n;
typedef TScalarVector<TSNorm<u8>, 4> byte4n;
typedef TScalarVector<TUNorm<u8>, 2> ubyte2n;
typedef TScalarVector<TUNorm<u8>, 4> ubyte4n;
//----------------------------------------------------------------------------
typedef TScalarVector<TSNorm<u16>, 2> short2n;
typedef TScalarVector<TSNorm<u16>, 4> short4n;
typedef TScalarVector<TUNorm<u16>, 2> ushort2n;
typedef TScalarVector<TUNorm<u16>, 4> ushort4n;
//----------------------------------------------------------------------------
typedef TScalarVector<TSNorm<u32>, 2> word2n;
typedef TScalarVector<TSNorm<u32>, 3> word3n;
typedef TScalarVector<TSNorm<u32>, 4> word4n;
typedef TScalarVector<TUNorm<u32>, 2> uword2n;
typedef TScalarVector<TUNorm<u32>, 3> uword3n;
typedef TScalarVector<TUNorm<u32>, 4> uword4n;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
