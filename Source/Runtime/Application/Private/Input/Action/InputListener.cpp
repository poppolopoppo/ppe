// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Input/Action/InputListener.h"

#include "Input/InputDevice.h"
#include "Input/Action/InputMapping.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FInputListener::HasMapping(const PInputMapping& mapping) const NOEXCEPT {
    return (!!_data.LockShared()->Mappings.GetIFP(FInputMappingWPriority {
        .InputMapping = mapping,
        .Priority = 0 // priority is ignored by FInputMappingWPriority::operator ==
    }));
}
//----------------------------------------------------------------------------
void FInputListener::AddMapping(const PInputMapping& mapping, i32 priority) {
    const auto exclusiveData = _data.LockExclusive();

    exclusiveData->Mappings.Insert_Overwrite(FInputMappingWPriority{
        .InputMapping = mapping,
        .Priority = priority,
    });

    RebuildKeyMappings_(*exclusiveData);
}
//----------------------------------------------------------------------------
bool FInputListener::RemoveMapping(const PInputMapping& mapping) {
    const auto exclusiveData = _data.LockExclusive();

    if (exclusiveData->Mappings.Erase(FInputMappingWPriority{
        .InputMapping = mapping,
        .Priority = 0 // priority is ignored by FInputMappingWPriority::operator ==
    })) {
        RebuildKeyMappings_(*exclusiveData);
        return true;
    }

    return false;
}
//----------------------------------------------------------------------------
void FInputListener::ClearAllMappings() {
    const auto exclusiveData = _data.LockExclusive();

    exclusiveData->ActionInstances.clear();
    exclusiveData->Mappings.clear();
}
//----------------------------------------------------------------------------
bool FInputListener::IsKeyHandledByAction(const FInputKey& key) const NOEXCEPT {
    return _data.LockShared()->Keys.contains(key);
}
//----------------------------------------------------------------------------
FInputValue FInputListener::ActionValue(const PInputAction& action) const NOEXCEPT {
    return _data.LockShared()->ActionInstances[action].ActionValue();
}
//----------------------------------------------------------------------------
bool FInputListener::InputKey(const FInputMessage& message) {
    AssertMessage_NoAssume("AnyKey is not supported as input", message.Key != EInputKey::AnyKey);
    const auto exclusiveData = _data.LockExclusive();

    const auto range = exclusiveData->Keys.equal_range(message.Key);
    if (range.first == range.second)
        return false;

    forrange(it, range.first, range.second) {
        const FInputActionKeyMapping& keyMapping = KeyMapping_(*exclusiveData, it->second);
        Assert_NoAssume(message.Key == keyMapping.Key || keyMapping.Key == EInputKey::AnyKey);

        FInputActionInstance& instance = exclusiveData->ActionInstances[keyMapping.Action];

        instance._value = message.Value;

        instance._sourceAction->_Modifiers(message.DeltaTime, instance._value);
        keyMapping._Modifiers(message.DeltaTime, instance._value);

        switch (message.Event) {
        case Pressed:
            instance._triggerState = EInputTriggerEvent::Started;
            instance._elapsedTriggeredTime = message.DeltaTime;

            keyMapping._OnStarted(instance, message.Key);
            instance._sourceAction->_OnStarted(instance, message.Key);
            break;

        case Repeat:
            instance._triggerState = EInputTriggerEvent::Triggered;
            instance._elapsedTriggeredTime += message.DeltaTime;

            keyMapping._OnTriggered(instance, message.Key);
            instance._sourceAction->_OnTriggered(instance, message.Key);
            break;

        case Released:
            instance._triggerState = EInputTriggerEvent::Completed;

            keyMapping._OnCompleted(instance, message.Key);
            instance._sourceAction->_OnCompleted(instance, message.Key);

            instance._elapsedTriggeredTime = 0;
            break;

        case DoubleClick: FALLTHROUGH();
        case Axis:
            instance._triggerState = EInputTriggerEvent::Triggered;
            instance._elapsedTriggeredTime = message.DeltaTime;

            keyMapping._OnTriggered(instance, message.Key);
            instance._sourceAction->_OnTriggered(instance, message.Key);
            break;
        }
    }

    return true;
}
//----------------------------------------------------------------------------
const FInputActionKeyMapping& FInputListener::KeyMapping_(const FInternalData_& data, FInputBinding binding) {
    return data.Mappings.MakeView().at(binding.InputMappingIndex)
        .InputMapping->_mappings.LockShared()->At(binding.KeyMappingIndex);
}
//----------------------------------------------------------------------------
void FInputListener::RebuildKeyMappings_(FInternalData_& data) {
    data.Keys.clear(); // rebuilding keys and action instances

    FInputActionInstances backup = std::move(data.ActionInstances);
    data.ActionInstances.reserve(backup.size());

    const auto mapKey = [&](const FInputKey& key, const PInputAction& action, const FInputBinding& binding) -> bool {
        auto [first, last] = data.Keys.equal_range(key);
        if (first != last)
            last = std::prev(last);

        if (data.Keys.end() != last &&
            last->first == key &&
            KeyMapping_(data, last->second).Action->HasConsumeInput())
            return false;

        if (const auto it = backup.find(action); backup.end() == it) {
            data.ActionInstances.insert({ action, FInputActionInstance(action) });
        }
        else {
            data.ActionInstances.insert({ action, std::move(it->second)});
            backup.erase(it);
        }

        data.Keys.insert(last, { key, binding });
        return true;
    };

    forrange(mapped, data.Mappings.begin(), data.Mappings.end()) {
        const auto keys = mapped->InputMapping->_mappings.LockShared();
        forrange(keyMappingIndex, 0, keys->size()) {
            const FInputActionKeyMapping& mapping = keys->At(keyMappingIndex);

            const FInputBinding binding{
                .InputMappingIndex = checked_cast<u32>(std::distance(data.Mappings.begin(), mapped)),
                .KeyMappingIndex = checked_cast<u32>(keyMappingIndex),
            };

            if (Likely(mapping.Key != EInputKey::AnyKey)) {
                mapKey(mapping.Key, mapping.Action, binding);
            }
            else { // expand AnyKey to all possible keys
                EInputKey::EachKey(MakeAppendable<FInputKey>([&](const FInputKey& key) {
                    mapKey(key, mapping.Action, binding);
                }));
            }
        }
    }

    backup.clear_ReleaseMemory();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
