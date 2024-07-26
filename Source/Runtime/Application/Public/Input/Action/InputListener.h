#pragma once

#include "Application_fwd.h"

#include "Input/Action/InputAction.h"
#include "Input/Action/InputMapping.h"
#include "Input/InputKey.h"

#include "Container/FlatSet.h"
#include "Container/HashMap.h"
#include "Container/MultiMap.h"
#include "Time/Time_fwd.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EInputListenerEvent : u8 {
    // listener do not trigger any mapping
    Unhandled           = 0,
    // listener trigger at least one mapping, but did not consume the input
    Handled,
    // listener trigger at least one mapping, and input was consumed (message won't be handled by any other listener)
    Consumed,

    Unknown             = Unhandled,
};
//----------------------------------------------------------------------------
class FInputListener : public FRefCountable {
public:
    FInputListener() = default;

    NODISCARD PPE_APPLICATION_API bool HasMapping(const PCInputMapping& mapping) const NOEXCEPT;

    NODISCARD EInputListenerEvent Mode() const {
        return _data.LockShared()->Mode;
    }
    void SetMode(EInputListenerEvent value) {
        _data.LockExclusive()->Mode = value;
    }

    PPE_APPLICATION_API void AddMapping(const PInputMapping& mapping, i32 priority);
    PPE_APPLICATION_API bool RemoveMapping(const PInputMapping& mapping);
    PPE_APPLICATION_API void ClearAllMappings();

    NODISCARD PPE_APPLICATION_API bool IsKeyHandledByAction(const FInputKey& key) const NOEXCEPT;
    NODISCARD PPE_APPLICATION_API FInputValue ActionValue(const PInputAction& action) const NOEXCEPT;

    PPE_APPLICATION_API EInputListenerEvent InputKey(const FInputMessage& message);

private:
    struct FInputMappingWPriority {
        PInputMapping InputMapping;
        i32 Priority{ 0 };

        NODISCARD bool operator ==(const PCInputMapping& mapping) const NOEXCEPT {
            return (InputMapping == mapping);
        }
        NODISCARD bool operator ==(const FInputMappingWPriority& other) const NOEXCEPT {
            return (InputMapping == other.InputMapping);
        }
        NODISCARD bool operator < (const FInputMappingWPriority& other) const NOEXCEPT {
            return (Priority == other.Priority
                ? InputMapping.get() < other.InputMapping.get()
                : Priority > other.Priority);
        }
    };

    struct FInputBinding {
        u32 InputMappingIndex{ 0 };
        u32 KeyMappingIndex{ 0 };
    };

    using FInputMappingsWPriority = FLATSET_INSITU(Input, FInputMappingWPriority, 3);
    using FInputActionInstances = HASHMAP(Input, SInputAction, FInputActionInstance);

    struct FInternalData_ {
        FInputMappingsWPriority Mappings;
        FInputActionInstances ActionInstances;
        MULTIMAP(Input, FInputKey, FInputBinding) Keys;
        EInputListenerEvent Mode{ EInputListenerEvent::Consumed };
    };

    static void RebuildKeyMappings_(FInternalData_& data);
    NODISCARD static const FInputActionKeyMapping& KeyMapping_(const FInternalData_& data, FInputBinding binding);

    TThreadSafe<FInternalData_, EThreadBarrier::DataRaceCheck> _data;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
