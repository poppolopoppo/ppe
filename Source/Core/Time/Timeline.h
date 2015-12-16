#pragma once

#include "Core/Core.h"

#include "Core/Time/Timepoint.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class Timeline {
public:
    static Timeline StartNow() { return Timeline(Timepoint::Now()); }

    Timeline();
    explicit Timeline(const Timepoint& start);
    ~Timeline();

    Timeline(const Timeline& other);
    Timeline& operator =(const Timeline& other);

    const Timepoint& Now() const { return _now; }
    const Timepoint& Last() const { return _last; }
    const Timepoint& Start() const { return _start; }

    Timespan Elapsed() const { return Timepoint::Duration(_last, _now); }
    Timespan Total() const { return Timepoint::Duration(_start, _now); }

    void Reset();

    void Tick();
    void Tick(const Timeline& other);
    void Tick(const Timeline& other, float speed);

    bool Tick_Every(const Timespan& target, Timespan& elapsed);
    bool Tick_Every(const Timeline& other, const Timespan& target, Timespan& elapsed);
    bool Tick_Every(const Timepoint& now, const Timespan& target, Timespan& elapsed);

    FORCE_INLINE bool Tick_Target60FPS(Timespan& elapsed) { return Tick_Every(Timespan_60hz(), elapsed); }
    FORCE_INLINE bool Tick_Target30FPS(Timespan& elapsed) { return Tick_Every(Timespan_30hz(), elapsed); }
    FORCE_INLINE bool Tick_Target15FPS(Timespan& elapsed) { return Tick_Every(Timespan_15hz(), elapsed); }

private:
    Timepoint _now;
    Timepoint _last;
    Timepoint _start;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
