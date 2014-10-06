#pragma once

#include "Engine.h"

#include "Core/Timepoint.h"

namespace Core {
class Timeline;

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class WorldTime {
public:
    explicit WorldTime();
    ~WorldTime();

    WorldTime(const WorldTime& ) = delete;
    WorldTime& operator =(const WorldTime& ) = delete;

    float Speed() const { return _speed; }
    void SetSpeed(float value);

    const Timespan& Elapsed() const { return _elapsed; }
    const Timespan& Total() const { return _total; }

    void Reset();
    void Update(const Timeline& timeline);

private:
    float _speed;
    Timespan _elapsed;
    Timespan _total;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
