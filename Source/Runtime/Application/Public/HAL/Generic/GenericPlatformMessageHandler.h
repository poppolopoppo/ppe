#pragma once

#include "Application_fwd.h"

namespace PPE {
namespace Application {
class FGenericWindow;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericPlaformMessageHandler {
public:
    static bool PumpGlobalMessages() = delete;
    static bool PumpMessages(FGenericWindow& window) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
