#pragma once

#include "Application_fwd.h"

#include "Input/Action/InputAction.h"
#include "Input/InputKey.h"

#include "Container/SparseArray.h"
#include "Input/InputDevice.h"
#include "Misc/Event.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FInputActionKeyMapping {
    FInputKey Key;

    PInputAction Action;

    using FModifierEvent    = FInputAction::FModifierEvent;
    PUBLIC_EVENT(Modifiers,     FModifierEvent);

    using FTriggerEvent     = TFunction<void(const FInputActionInstance&, const FInputKey&)>;
    PUBLIC_EVENT(OnStarted,     FTriggerEvent);
    PUBLIC_EVENT(OnTriggered,   FTriggerEvent);
    PUBLIC_EVENT(OnCompleted,   FTriggerEvent);

    FInputActionKeyMapping() = default;
    FInputActionKeyMapping(FInputActionKeyMapping&& ) = default;

    PPE_APPLICATION_API FInputActionKeyMapping(
        const FInputKey& key, const PInputAction& action ) NOEXCEPT;

    PPE_APPLICATION_API FInputActionKeyMapping(
        const FInputKey& key, const PInputAction& action,
        FTriggerEvent&& trigger );

    PPE_APPLICATION_API FInputActionKeyMapping(
        const FInputKey& key, const PInputAction& action,
        FModifierEvent&& modifier );

    PPE_APPLICATION_API FInputActionKeyMapping(
        const FInputKey& key, const PInputAction& action,
        FModifierEvent&& modifier,
        FTriggerEvent&& trigger );

    friend class FInputListener;

    bool operator ==(const FInputActionKeyMapping& other) const {
        return (Key == other.Key && Action == other.Action);
    }
};
//----------------------------------------------------------------------------
class PPE_APPLICATION_API IInputActionKeyMappingProvider {
public:
    virtual ~IInputActionKeyMappingProvider() = default;

    virtual void InputActionKeyMappings(TAppendable<FInputActionKeyMapping> keyMappings) const NOEXCEPT = 0;
};
//----------------------------------------------------------------------------
using FInputKeyMappings = SPARSEARRAY_INSITU(Input, FInputActionKeyMapping);
//----------------------------------------------------------------------------
class FInputMapping : public FRefCountable {
public:
    PPE_APPLICATION_API explicit FInputMapping(FStringLiteral description) NOEXCEPT;

    NODISCARD FStringLiteral Description() const { return _description; }

    NODISCARD const FInputActionKeyMapping& KeyMapping(size_t mappingIndex) const {
        return _mappings.LockShared()->At(mappingIndex);
    }

    PPE_APPLICATION_API void MapAll(const IInputActionKeyMappingProvider& provider);
    PPE_APPLICATION_API FInputActionKeyMapping& MapKey(const SInputAction& action, const FInputKey& key);
    PPE_APPLICATION_API bool UnmapKey(const SCInputAction& action, const FInputKey& key);

    PPE_APPLICATION_API void UnmapAll();
    PPE_APPLICATION_API void UnmapAll(const IInputActionKeyMappingProvider& provider);
    PPE_APPLICATION_API void UnmapAllKeysFromAction(const SCInputAction& action);

private:
    friend class FInputListener;

    FStringLiteral _description;

    TThreadSafe<FInputKeyMappings, EThreadBarrier::DataRaceCheck> _mappings;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
