// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Time/Timeline.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FTimeline::FTimeline()
:   _now(0), _last(0), _start(0) {}
//----------------------------------------------------------------------------
FTimeline::FTimeline(const FTimepoint& start)
:   _now(start), _last(start), _start(start) {}
//----------------------------------------------------------------------------
FTimeline::~FTimeline() = default;
//----------------------------------------------------------------------------
FTimeline::FTimeline(const FTimeline& other)
:   _now(other._now)
,   _last(other._last)
,   _start(other._start) {}
//----------------------------------------------------------------------------
FTimeline& FTimeline::operator =(const FTimeline& other) {
    _now = other._now;
    _last = other._last;
    _start = other._start;
    return *this;
}
//----------------------------------------------------------------------------
void FTimeline::Reset() {
    _now = _last =_start = FTimepoint::Now();
}
//----------------------------------------------------------------------------
void FTimeline::ResetToZero() {
    _now = _last =_start = Zero;
}
//----------------------------------------------------------------------------
void FTimeline::Tick() {
    _last = _now;
    _now = FTimepoint::Now();
}
//----------------------------------------------------------------------------
void FTimeline::Tick(FTimespan dt) {
    _last = _now;
    _now = _now + dt;
}
//----------------------------------------------------------------------------
void FTimeline::Tick(const FTimeline& other) {
    _last = _now;
    _now = other._now;
}
//----------------------------------------------------------------------------
void FTimeline::Tick(const FTimeline& other, float speed) {
    if (speed != 0.0f)
        Tick({ other.Elapsed().Value() * speed });
}
//----------------------------------------------------------------------------
bool FTimeline::Tick_Every(const FTimespan& target, FTimespan& elapsed) {
    return Tick_Every(FTimepoint::Now(), target, elapsed);
}
//----------------------------------------------------------------------------
bool FTimeline::Tick_Every(const FTimeline& other, const FTimespan& target, FTimespan& elapsed) {
    return Tick_Every(other._now, target, elapsed);
}
//----------------------------------------------------------------------------
bool FTimeline::Tick_Every(const FTimepoint& now, const FTimespan& target, FTimespan& elapsed) {
    _now = now;

    const FTimespan d = FTimepoint::Duration(_last, _now);
    if (d < target)
        return false;

    _last = _now;
    elapsed = d;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
