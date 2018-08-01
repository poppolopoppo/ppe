#pragma once

#include "Core/Core.h"

#include "Core/Maths/MathHelpers.h"
#include "Core/Time/Timeline.h"
#include "Core/Time/Timepoint.h"

namespace Core {
namespace Timely {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FNormalized {
public:
    FNormalized();
    ~FNormalized();

    FNormalized(const FTimeline& time, const FTimespan& duration);
    FNormalized(const FTimepoint& start, const FTimespan& duration);

    void Start(const FTimeline& time, const FTimespan& duration);
    void Start(const FTimepoint& start, const FTimespan& duration);

    float Eval(const FTimeline& time) const;

private:
    FTimepoint _start;
    FTimepoint _stop;
};
//----------------------------------------------------------------------------
class FPulsar {
public:
    FPulsar();
    ~FPulsar();

    FPulsar(const FTimeline& time, const FTimespan& duration);
    FPulsar(const FTimepoint& start, const FTimespan& duration);

    void Start(const FTimeline& time, const FTimespan& duration);
    void Start(const FTimepoint& start, const FTimespan& duration);

    float Eval(const FTimeline& time) const;

private:
    FTimepoint _start;
    FTimepoint _stop;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Eval = FNormalized>
class TLerp : private _Eval {
public:
    TLerp(const T& v0, const T& v1);
    ~TLerp();

    TLerp(const T& v0, const T& v1, const FTimeline& time, const FTimespan& duration);
    TLerp(const T& v0, const T& v1, const FTimepoint& start, const FTimespan& duration);

    using _Eval::Start;

    void SetRange(const T& v0, const T& v1);

    T Eval(const FTimeline& time) const;

private:
    T _v0, _v1;
};
//----------------------------------------------------------------------------
template <typename T, typename _Eval = FNormalized>
class TSmoothstep : private _Eval {
public:
    TSmoothstep(const T& v0, const T& v1);
    ~TSmoothstep();

    TSmoothstep(const T& v0, const T& v1, const FTimeline& time, const FTimespan& duration);
    TSmoothstep(const T& v0, const T& v1, const FTimepoint& start, const FTimespan& duration);

    using _Eval::Start;

    void SetRange(const T& v0, const T& v1);

    T Eval(const FTimeline& time) const;

private:
    T _v0, _v1;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using TNormalizedLerp = TLerp<T, FNormalized>;
//----------------------------------------------------------------------------
template <typename T>
using TNormalizedSmoothstep = TSmoothstep<T, FNormalized>;
//----------------------------------------------------------------------------
template <typename T>
using TPulsarLerp = TLerp<T, FPulsar>;
//----------------------------------------------------------------------------
template <typename T>
using TPulsarSmoothstep = TSmoothstep<T, FPulsar>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Timely
} //!namespace Core

#include "Core/Time/Timely-inl.h"
