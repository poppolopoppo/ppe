#pragma once

#include "Core/Core.h"

#include "Core/Thread/Task/Task.h"

#include <functional>

// https://en.wikipedia.org/wiki/Futures_and_promises
// https://en.wikipedia.org/wiki/C%2B%2B11#Threading_facilities

namespace Core {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
template <typename T>
class Future;
//----------------------------------------------------------------------------
template <typename T>
class Promise : public Task {
public:
    Promise(std::function<T>&& async);
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Core
