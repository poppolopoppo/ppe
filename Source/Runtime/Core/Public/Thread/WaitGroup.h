#pragma once

#include "Core_fwd.h"

#include <condition_variable>
#include <mutex>

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FWaitGroup : Meta::FNonCopyableNorMovable {
public:
    explicit FWaitGroup(int count = 0) NOEXCEPT
        : _count(count)
    {}

    bool Add(int delta) NOEXCEPT {
        const std::unique_lock lock(_barrier);
        _count += delta;
        Assert_NoAssume(_count >= 0);
        return (_count > 0);
    }

    void Done() NOEXCEPT {
        if (not Add(-1))
            _onDone.notify_one();
    }

    void Wait() NOEXCEPT {
        std::unique_lock lock(_barrier);
        _onDone.wait(lock, [this]() { return (_count == 0); });
    }

private:
    std::mutex _barrier;
    std::condition_variable _onDone;
    int _count;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
