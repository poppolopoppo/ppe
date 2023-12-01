#pragma once

#include "UI/Imgui.h"

#include "Container/FlatSet.h"
#include "Container/RingBuffer.h"
#include "Diagnostic/Logger.h"
#include "IO/String.h"
#include "Memory/UniquePtr.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLogViewerWidget {
public:
    using FSiteInfo =  FLogger::FSiteInfo;
    using EVerbosity =  FLogger::EVerbosity;

    FString Title{ ICON_CI_OUTPUT " Log viewer" };

    bool AutoScroll{ true };
    bool LoggerWasRegistered{ false };
    bool NeedRefreshVisibleMessages{ true };
    bool ShowCategoriesPanel{ true };
    bool WindowVisible{ true };

    class FHistoryLogger;

    struct FHistoryCategory : TPtrRef<const FLoggerCategory> {
        bool Visible{ true };

        using TPtrRef::operator*;
        using TPtrRef::operator->;

        explicit FHistoryCategory(const FLoggerCategory& category) : TPtrRef(category) {}

        bool operator ==(const FHistoryCategory& other) const { return (static_cast<const TPtrRef&>(*this) == other); }
        bool operator !=(const FHistoryCategory& other) const { return (static_cast<const TPtrRef&>(*this) != other); }

        bool operator < (const FHistoryCategory& other) const { return ((*this)->Name <  other->Name); }
        bool operator >=(const FHistoryCategory& other) const { return ((*this)->Name >= other->Name); }
    };

    struct FHistoryMessage : FLoggerMessage {
        FAllocatorBlock Block;
        bool PassFilter{ true };
    };

    using PHistoryMessage = TPtrRef<const FHistoryMessage>;

    STATIC_CONST_INTEGRAL(size_t, HistoryCapacity, 1000);

    u32 LastMessagePosted{ INDEX_NONE };
    u32 LastMessageVisible{ INDEX_NONE };
    RINGBUFFER(UI, PHistoryMessage, HistoryCapacity) VisibleMessages;

    FLATSET(UI, FHistoryCategory) Categories;

    TUniquePtr<FHistoryLogger> Logger;
    ImGuiTextFilter LogFilter;

    PPE_APPLICATIONUI_API FLogViewerWidget() NOEXCEPT;
    PPE_APPLICATIONUI_API ~FLogViewerWidget();

    NODISCARD PPE_APPLICATIONUI_API bool Show();

    PPE_APPLICATIONUI_API void FlushVisibleMessages();
    PPE_APPLICATIONUI_API void RefreshVisibleMessages();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
