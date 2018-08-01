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

    FTimespan Elapsed() const { return FTimepoint::Duration(_last, _now); }
    FTimespan Total() const { return FTimepoint::Duration(_start, _now); }

    void Reset();

    void Tick();
    void Tick(const FTimeline& other);
    void Tick(const FTimeline& other, float speed);

    bool Tick_Every(const FTimespan& target, FTimespan& elapsed);
    bool Tick_Every(const FTimeline& other, const FTimespan& target, FTimespan& elapsed);
    bool Tick_Every(const FTimepoint& now, const FTimespan& target, FTimespan& elapsed);

    FORCE_INLINE bool Tick_Target120FPS(FTimespan& elapsed) { return Tick_Every(Timespan_120hz(), elapsed); }
    FORCE_INLINE bool Tick_Target60FPS(FTimespan& elapsed) { return Tick_Every(Timespan_60hz(), elapsed); }
    FORCE_INLINE bool Tick_Target30FPS(FTimespan& elapsed) { return Tick_Every(Timespan_30hz(), elapsed); }
    FORCE_INLINE bool Tick_Target15FPS(FTimespan& elapsed) { return Tick_Every(Timespan_15hz(), elapsed); }

private:
    FTimepoint _now;
    FTimepoint _last;
    FTimepoint _start;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
