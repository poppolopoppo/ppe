#pragma once

#include "Timely.h"

namespace Core {
namespace Timely {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline Normalized::Normalized() : _start(0), _stop(0) {}
//----------------------------------------------------------------------------
inline Normalized::~Normalized() {}
//----------------------------------------------------------------------------
inline Normalized::Normalized(const Timeline& time, const Timespan& duration) {
    Start(time, duration);
}
//----------------------------------------------------------------------------
inline Normalized::Normalized(const Timepoint& start, const Timespan& duration) {
    Start(start, duration);
}
//----------------------------------------------------------------------------
inline void Normalized::Start(const Timeline& time, const Timespan& duration) {
    Assert(duration.Value() > 0);
    _start = time.Now();
    _stop = _start + duration;
}
//----------------------------------------------------------------------------
inline void Normalized::Start(const Timepoint& start, const Timespan& duration) {
    Assert(duration.Value() > 0);
    _start = start;
    _stop = _start + duration;
}
//----------------------------------------------------------------------------
inline float Normalized::Eval(const Timeline& time) const {
    return Saturate(LinearStep(time.Now().Value(), _start.Value(), _stop.Value()));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline Pulsar::Pulsar() : _start(0), _stop(0) {}
//----------------------------------------------------------------------------
inline Pulsar::~Pulsar() {}
//----------------------------------------------------------------------------
inline Pulsar::Pulsar(const Timeline& time, const Timespan& duration) {
    Start(time, duration);
}
//----------------------------------------------------------------------------
inline Pulsar::Pulsar(const Timepoint& start, const Timespan& duration) {
    Start(start, duration);
}
//----------------------------------------------------------------------------
inline void Pulsar::Start(const Timeline& time, const Timespan& duration) {
    Assert(duration.Value() > 0);
    _start = time.Now();
    _stop = _start + duration;
}
//----------------------------------------------------------------------------
inline void Pulsar::Start(const Timepoint& start, const Timespan& duration) {
    Assert(duration.Value() > 0);
    _start = _start;
    _stop = _start + duration;
}
//----------------------------------------------------------------------------
inline float Pulsar::Eval(const Timeline& time) const {
    const float t = LinearStep(time.Now().Value(), _start.Value(), _stop.Value());
    const float n = std::floor(t);
    const int i = static_cast<int>(n);
    return (i & 1) ? (1 - (t - n)) : (t - n);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
Lerp<T, _Eval>::Lerp(const T& v0, const T& v1)
:   _v0(v0), _v1(v1) {}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
Lerp<T, _Eval>::~Lerp() {}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
Lerp<T, _Eval>::Lerp(const T& v0, const T& v1, const Timepoint& start, const Timespan& duration)
:   Lerp(v0, v1) {
    Start(start, duration);
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
Lerp<T, _Eval>::Lerp(const T& v0, const T& v1, const Timeline& time, const Timespan& duration)
:   Lerp(v0, v1) {
    Start(time, duration);
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
void Lerp<T, _Eval>::SetRange(const T& v0, const T& v1) {
    _v0 = v0;
    _v1 = v1;
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
T Lerp<T, _Eval>::Eval(const Timeline& time) const {
    return Core::Lerp(_v0, _v1, _Eval::Eval(time));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
Smoothstep<T, _Eval>::Smoothstep(const T& v0, const T& v1)
:   _v0(v0), _v1(v1) {}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
Smoothstep<T, _Eval>::~Smoothstep() {}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
Smoothstep<T, _Eval>::Smoothstep(const T& v0, const T& v1, const Timepoint& start, const Timespan& duration)
:   Smoothstep(v0, v1) {
    Start(start, duration);
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
Smoothstep<T, _Eval>::Smoothstep(const T& v0, const T& v1, const Timeline& time, const Timespan& duration)
:   Smoothstep(v0, v1) {
    Start(time, duration);
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
void Smoothstep<T, _Eval>::SetRange(const T& v0, const T& v1) {
    _v0 = v0;
    _v1 = v1;
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
T Smoothstep<T, _Eval>::Eval(const Timeline& time) const {
    const float t = _Eval::Eval(time);
    return Core::Lerp(_v0, _v1, t*t*(3 - 2 * t));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Timely
} //!namespace Core