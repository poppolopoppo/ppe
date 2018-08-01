#pragma once

#include "Core/Time/Timely.h"

namespace Core {
namespace Timely {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FNormalized::FNormalized() : _start(0), _stop(0) {}
//----------------------------------------------------------------------------
inline FNormalized::~FNormalized() {}
//----------------------------------------------------------------------------
inline FNormalized::FNormalized(const FTimeline& time, const Timespan& duration) {
    Start(time, duration);
}
//----------------------------------------------------------------------------
inline FNormalized::FNormalized(const FTimepoint& start, const Timespan& duration) {
    Start(start, duration);
}
//----------------------------------------------------------------------------
inline void FNormalized::Start(const FTimeline& time, const Timespan& duration) {
    Assert(duration.Value() > 0);
    _start = time.Now();
    _stop = _start + duration;
}
//----------------------------------------------------------------------------
inline void FNormalized::Start(const FTimepoint& start, const Timespan& duration) {
    Assert(duration.Value() > 0);
    _start = start;
    _stop = _start + duration;
}
//----------------------------------------------------------------------------
inline float FNormalized::Eval(const FTimeline& time) const {
    return Saturate(LinearStep(time.Now().Value(), _start.Value(), _stop.Value()));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
inline FPulsar::FPulsar() : _start(0), _stop(0) {}
//----------------------------------------------------------------------------
inline FPulsar::~FPulsar() {}
//----------------------------------------------------------------------------
inline FPulsar::FPulsar(const FTimeline& time, const Timespan& duration) {
    Start(time, duration);
}
//----------------------------------------------------------------------------
inline FPulsar::FPulsar(const FTimepoint& start, const Timespan& duration) {
    Start(start, duration);
}
//----------------------------------------------------------------------------
inline void FPulsar::Start(const FTimeline& time, const Timespan& duration) {
    Assert(duration.Value() > 0);
    _start = time.Now();
    _stop = _start + duration;
}
//----------------------------------------------------------------------------
inline void FPulsar::Start(const FTimepoint& start, const Timespan& duration) {
    Assert(duration.Value() > 0);
    _start = _start;
    _stop = _start + duration;
}
//----------------------------------------------------------------------------
inline float FPulsar::Eval(const FTimeline& time) const {
    const float t = LinearStep(time.Now().Value(), _start.Value(), _stop.Value());
    const float n = std::floor(t);
    const int i = static_cast<int>(n);
    return (i & 1) ? (1 - (t - n)) : (t - n);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
TLerp<T, _Eval>::TLerp(const T& v0, const T& v1)
:   _v0(v0), _v1(v1) {}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
TLerp<T, _Eval>::~TLerp() {}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
TLerp<T, _Eval>::TLerp(const T& v0, const T& v1, const FTimepoint& start, const Timespan& duration)
:   TLerp(v0, v1) {
    Start(start, duration);
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
TLerp<T, _Eval>::TLerp(const T& v0, const T& v1, const FTimeline& time, const Timespan& duration)
:   TLerp(v0, v1) {
    Start(time, duration);
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
void TLerp<T, _Eval>::SetRange(const T& v0, const T& v1) {
    _v0 = v0;
    _v1 = v1;
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
T TLerp<T, _Eval>::Eval(const FTimeline& time) const {
    return Core::TLerp(_v0, _v1, _Eval::Eval(time));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
TSmoothstep<T, _Eval>::TSmoothstep(const T& v0, const T& v1)
:   _v0(v0), _v1(v1) {}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
TSmoothstep<T, _Eval>::~TSmoothstep() {}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
TSmoothstep<T, _Eval>::TSmoothstep(const T& v0, const T& v1, const FTimepoint& start, const Timespan& duration)
:   TSmoothstep(v0, v1) {
    Start(start, duration);
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
TSmoothstep<T, _Eval>::TSmoothstep(const T& v0, const T& v1, const FTimeline& time, const Timespan& duration)
:   TSmoothstep(v0, v1) {
    Start(time, duration);
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
void TSmoothstep<T, _Eval>::SetRange(const T& v0, const T& v1) {
    _v0 = v0;
    _v1 = v1;
}
//----------------------------------------------------------------------------
template <typename T, typename _Eval>
T TSmoothstep<T, _Eval>::Eval(const FTimeline& time) const {
    const float t = _Eval::Eval(time);
    return Core::TLerp(_v0, _v1, t*t*(3 - 2 * t));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Timely
} //!namespace Core
