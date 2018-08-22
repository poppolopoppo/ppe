#pragma once

#include "Application_fwd.h"

#include "Container/Vector.h"
#include "Misc/Function.h"

namespace PPE {
namespace Application {
class FWindowsWindow;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
// highly platform specific, not meant to be used generically
enum class EGenericMessageType {
    Null
};
//----------------------------------------------------------------------------
// highly platform specific, not meant to be used generically
struct FGenericMessage {
    EGenericMessageType Type;
};
//----------------------------------------------------------------------------
struct PPE_APPLICATION_API FGenericPlaformMessageHandler {
public:
    using EMessageType = EGenericMessageType;
    using FMessage = FGenericMessage;
    using FMessageHandler = TFunction<void(const FGenericWindow&, FGenericMessage*)>;

    static bool PumpMessages(FWindowsWindow* windowIFP) = delete;

};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
