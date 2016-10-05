#pragma once

#include "Core/Core.h"

#include "Core/Time/Timepoint.h"

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FTimeline {
public:
    static FTimeline StartNow() { return FTimeline(FTimepoint::Now()); }

    FTimeline();
    explicit FTimeline(const FTimepoint& start);
    ~FTimeline();

    FTimeline(const FTimeline& other);
    FTimeline& operator =(const FTimeline& other);

    const FTimepoint& Now() const { return _now; }
    const FTimepoint& Last() const { return _last; }
    const FTimepoint& Start() const { return _start; }

    Timespan Elapsed() const { return FTimepoint::Duration(_last, _now); }
    Timespan Total() const { return FTimepoint::Duration(_start, _now); }

    void Reset();

    void Tick();
    void Tick(const FTimeline& other);
    void Tick(const FTimeline& other, float speed);

    bool Tick_Every(const Timespan& target, Timespan& elapsed);
    bool Tick_Every(const FTimeline& other, const Timespan& target, Timespan& elapsed);
    bool Tick_Every(const FTimepoint& now, const Timespan& target, Timespan& elapsed);

    FORCE_INLINE bool Tick_Target60FPS(Timespan& elapsed) { return Tick_Every(Timespan_60hz(), elapsed); }
    FORCE_INLINE bool Tick_Target30FPS(Timespan& elapsed) { return Tick_Every(Timespan_30hz(), elapsed); }
    FORCE_INLINE bool Tick_Target15FPS(Timespan& elapsed) { return Tick_Every(Timespan_15hz(), elapsed); }

private:
    FTimepoint _now;
    FTimepoint _last;
    FTimepoint _start;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
