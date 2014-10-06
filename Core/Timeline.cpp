#include "stdafx.h"

#include "Timeline.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
Timeline::Timeline()
:   _now(0), _last(0), _start(0) {}
//----------------------------------------------------------------------------
Timeline::Timeline(const Timepoint& start)
:   _now(start), _last(start), _start(start) {}
//----------------------------------------------------------------------------
Timeline::~Timeline() {}
//----------------------------------------------------------------------------
Timeline::Timeline(const Timeline& other)
:   _now(other._now)
,   _last(other._last)
,   _start(other._start) {}
//----------------------------------------------------------------------------
Timeline& Timeline::operator =(const Timeline& other) {
    _now = other._now;
    _last = other._last;
    _start = other._start;
    return *this;
}
//----------------------------------------------------------------------------
void Timeline::Reset() {
    _now = _last =_start = Timepoint::Now();
}
//----------------------------------------------------------------------------
void Timeline::Tick() {
    _last = _now;
    _now = Timepoint::Now();
}
//----------------------------------------------------------------------------
void Timeline::Tick(const Timeline& other) {
    _last = _now;
    _now = other._now;
}
//----------------------------------------------------------------------------
bool Timeline::Tick_Every(const Timespan& target, Timespan& elapsed) {
    _now = Timepoint::Now();

    const Timespan d = Timepoint::Duration(_last, _now);
    if (d < target)
        return false;

    _last = _now;
    elapsed = d;

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
