// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/Action/InputMapping.h"

#include "Input/InputDevice.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FInputMapping::FInputMapping(FStringLiteral description) NOEXCEPT
:   _description(description) {
    Assert_NoAssume(not _description.empty());
}
//----------------------------------------------------------------------------
FInputActionKeyMapping::FInputActionKeyMapping(const FInputKey& key, const PInputAction& action) NOEXCEPT
:   Key(key)
,   Action(action)
{}
//----------------------------------------------------------------------------
FInputActionKeyMapping::FInputActionKeyMapping(const FInputKey& key, const PInputAction& action,
    FTriggerEvent&& trigger )
:   FInputActionKeyMapping(key, action) {
    OnTriggered().Emplace(std::move(trigger));
}
//----------------------------------------------------------------------------
FInputActionKeyMapping::FInputActionKeyMapping(
    const FInputKey& key, const PInputAction& action,
    FModifierEvent&& modifier )
:   FInputActionKeyMapping(key, action) {
    Modifiers().Emplace(std::move(modifier));
}
//----------------------------------------------------------------------------
FInputActionKeyMapping::FInputActionKeyMapping(
    const FInputKey& key, const PInputAction& action,
    FModifierEvent&& modifier,
    FTriggerEvent&& trigger )
:   FInputActionKeyMapping(key, action) {
    Modifiers().Emplace(std::move(modifier));
    OnTriggered().Emplace(std::move(trigger));
}
//----------------------------------------------------------------------------
void FInputMapping::MapAll(const IInputActionKeyMappingProvider& provider) {
    const auto mappings = _mappings.LockExclusive();

    provider.InputActionKeyMappings(MakeAppendable<FInputActionKeyMapping>(
        [&mappings](FInputActionKeyMapping&& keyMapping) {
            mappings->EmplaceIt(std::move(keyMapping));
        }));
}
//----------------------------------------------------------------------------
FInputActionKeyMapping& FInputMapping::MapKey(const SInputAction& action, const FInputKey& key) {
    Assert_NoAssume(action);
    const auto mappings = _mappings.LockExclusive();

    auto it = std::find_if(mappings->begin(), mappings->end(),
        [&](const FInputActionKeyMapping& it) {
            return (it.Action.get() == action and it.Key == key);
        });

    if (mappings->end() == it) {
        it = mappings->EmplaceIt();
        it->Key = key;
        it->Action.reset(action);
    }

    Assert_NoAssume(it->Key == key and it->Action.get() == action.get());
    return *it;
}
//----------------------------------------------------------------------------
bool FInputMapping::UnmapKey(const SCInputAction& action, const FInputKey& key) {
    Assert_NoAssume(action);

    const auto mappings = _mappings.LockExclusive();
    const auto it = std::find_if(mappings->begin(), mappings->end(),
        [&](const FInputActionKeyMapping& it) {
            return (it.Action.get() == action and it.Key == key);
        });

    if (mappings->end() != it) {
        mappings->Remove(it);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
void FInputMapping::UnmapAll() {
    _mappings.LockExclusive()->Clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
void FInputMapping::UnmapAll(const IInputActionKeyMappingProvider& provider) {
    const auto mappings = _mappings.LockExclusive();

    provider.InputActionKeyMappings(MakeAppendable<FInputActionKeyMapping>(
        [&mappings](const FInputActionKeyMapping& keyMapping) {
            Remove_AssertExists(*mappings, keyMapping);
        }));
}
//----------------------------------------------------------------------------
void FInputMapping::UnmapAllKeysFromAction(const SCInputAction& action) {
    Assert_NoAssume(action);

    _mappings.LockExclusive()->RemoveIf([&](const FInputActionKeyMapping& it) {
        return (it.Action.get() == action);
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
