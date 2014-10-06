#pragma once

#include "Core.h"

#include "MathHelpers.h"
#include "Timeline.h"
#include "Timepoint.h"

namespace Core {
namespace Timely {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Normalized {
public:
    Normalized();
    ~Normalized();

    Normalized(const Timeline& time, const Timespan& duration);
    Normalized(const Timepoint& start, const Timespan& duration);

    void Start(const Timeline& time, const Timespan& duration);
    void Start(const Timepoint& start, const Timespan& duration);

    float Eval(const Timeline& time) const;

private:
    Timepoint _start;
    Timepoint _stop;
};
//----------------------------------------------------------------------------
class Pulsar {
public:
    Pulsar();
    ~Pulsar();

    Pulsar(const Timeline& time, const Timespan& duration);
    Pulsar(const Timepoint& start, const Timespan& duration);

    void Start(const Timeline& time, const Timespan& duration);
    void Start(const Timepoint& start, const Timespan& duration);

    float Eval(const Timeline& time) const;

private:
    Timepoint _start;
    Timepoint _stop;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Eval = Normalized>
class Lerp : private _Eval {
public:
    Lerp(const T& v0, const T& v1);
    ~Lerp();

    Lerp(const T& v0, const T& v1, const Timeline& time, const Timespan& duration);
    Lerp(const T& v0, const T& v1, const Timepoint& start, const Timespan& duration);

    using _Eval::Start;

    void SetRange(const T& v0, const T& v1);

    T Eval(const Timeline& time) const;

private:
    T _v0, _v1;
};
//----------------------------------------------------------------------------
template <typename T, typename _Eval = Normalized>
class Smoothstep : private _Eval {
public:
    Smoothstep(const T& v0, const T& v1);
    ~Smoothstep();

    Smoothstep(const T& v0, const T& v1, const Timeline& time, const Timespan& duration);
    Smoothstep(const T& v0, const T& v1, const Timepoint& start, const Timespan& duration);

    using _Eval::Start;

    void SetRange(const T& v0, const T& v1);

    T Eval(const Timeline& time) const;

private:
    T _v0, _v1;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
using NormalizedLerp = Lerp<T, Normalized>;
//----------------------------------------------------------------------------
template <typename T>
using NormalizedSmoothstep = Smoothstep<T, Normalized>;
//----------------------------------------------------------------------------
template <typename T>
using PulsarLerp = Lerp<T, Pulsar>;
//----------------------------------------------------------------------------
template <typename T>
using PulsarSmoothstep = Smoothstep<T, Pulsar>;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Timely
} //!namespace Core

#include "Timely-inl.h"
