#pragma once

#include "Time_fwd.h"

#include "Time/Timepoint.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class PPE_CORE_API FTimeline {
public:
    static FTimeline StartNow() { return FTimeline(FTimepoint::Now()); }

    FTimeline() = default;

    explicit FTimeline(const FTimepoint& start) NOEXCEPT;

    FTimeline(const FTimeline& other) = default;
    FTimeline& operator =(const FTimeline& other) = default;

    NODISCARD const FTimepoint& Now() const { return _now; }
    NODISCARD const FTimepoint& Last() const { return _last; }
    NODISCARD const FTimepoint& Start() const { return _start; }

    NODISCARD FTimespan Elapsed() const { return FTimepoint::Duration(_last, _now); }
    NODISCARD FTimespan Total() const { return FTimepoint::Duration(_start, _now); }

    void Reset();
    void ResetToZero();

    void Tick();
    void Tick(FTimespan dt);
    void Tick(const FTimeline& other);
    void Tick(const FTimeline& other, float speed);

    NODISCARD bool Tick_Every(const FTimespan& target, FTimespan& elapsed);
    NODISCARD bool Tick_Every(const FTimeline& other, const FTimespan& target, FTimespan& elapsed);
    NODISCARD bool Tick_Every(const FTimepoint& now, const FTimespan& target, FTimespan& elapsed);

    NODISCARD bool Tick_Target120FPS(FTimespan& elapsed) { return Tick_Every(Timespan_120hz, elapsed); }
    NODISCARD bool Tick_Target60FPS(FTimespan& elapsed) { return Tick_Every(Timespan_60hz, elapsed); }
    NODISCARD bool Tick_Target30FPS(FTimespan& elapsed) { return Tick_Every(Timespan_30hz, elapsed); }
    NODISCARD bool Tick_Target15FPS(FTimespan& elapsed) { return Tick_Every(Timespan_15hz, elapsed); }

private:
    FTimepoint _now{ Zero };
    FTimepoint _last{ Zero };
    FTimepoint _start{ Zero };
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
