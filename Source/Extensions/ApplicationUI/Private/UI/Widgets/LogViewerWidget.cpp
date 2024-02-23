// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "UI/Widgets/LogViewerWidget.h"

#if USE_PPE_LOGGER

#include "UI/Imgui.h"
#include "imgui-internal.h"

#include "Allocator/SlabAllocator.h"
#include "Color/Color.h"
#include "Diagnostic/CurrentProcess.h"
#include "IO/StaticString.h"
#include "IO/StringBuilder.h"
#include "IO/StringView.h"
#include "IO/TextWriter.h"
#include "Meta/Functor.h"
#include "Thread/ThreadContext.h"
#include "Thread/ThreadSafe.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATIONUI_API, UI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct FLogMessageFilterVisitor_ {
    TPtrRef<const ImGuiTextFilter> Filter;
    bool PassFilter{ false };

    template <typename _It>
    void operator ()(_It first, _It last) {
        if (PassFilter) return;
        forrange(it, first, last) {
            operator ()(*it);
            if (PassFilter)
                break;
        }
    }

    void operator ()(std::monostate) {}

    void operator ()(Opaq::boolean) {}
    void operator ()(Opaq::integer) {}
    void operator ()(Opaq::uinteger) {}
    void operator ()(Opaq::floating_point) {}

    void operator ()(FStringView v) {
        if (not PassFilter)
            PassFilter = Filter->PassFilter(v.data(), v.data() + v.size());
    }
    void operator ()(const Opaq::string_view& v) { operator()(Opaq::string_init(v)); }
    void operator ()(const Opaq::string_external& v) { operator()(v.MakeView()); }
    void operator ()(const Opaq::string_literal& v) { operator()(v.MakeView()); }

    void operator ()(FWStringView v) {
        if (not PassFilter)
            operator ()(WCHAR_TO_UTF_8(v).Str());
    }
    void operator ()(const Opaq::wstring_view& v) { operator()(Opaq::wstring_init(v)); }
    void operator ()(const Opaq::wstring_external& v) { operator()(v.MakeView()); }
    void operator ()(const Opaq::wstring_literal& v) { operator()(v.MakeView()); }

    void operator ()(Opaq::array_init v) { operator ()(std::begin(v), std::end(v)); }
    void operator ()(const Opaq::array_view& v) { operator ()(std::begin(v), std::end(v)); }
    void operator ()(const TPtrRef<const Opaq::array_view>& v) { operator ()(*v); }

    void operator ()(Opaq::object_init v) { operator ()(std::begin(v), std::end(v)); }
    void operator ()(const Opaq::object_view& v) { operator ()(std::begin(v), std::end(v)); }
    void operator ()(const TPtrRef<const Opaq::object_view>& v) { operator ()(*v); }

    void operator ()(const Opaq::value_init& v) {
        if (not PassFilter)
            std::visit(*this, v);
    }
    void operator ()(const Opaq::value_view& v) {
        if (not PassFilter)
            std::visit(*this, v);
    }

    void operator ()(const Opaq::key_value_init& v) {
        operator ()(v.key);
        operator ()(v.value);
    }
    void operator ()(const Opaq::key_value_view& v) {
        operator ()(v.key);
        operator ()(v.value);
    }

    void operator ()(const Opaq::string_format& fmt) {
        if (PassFilter)
            return;

        STACKLOCAL_TEXTWRITER(oss, 1024);
        fmt(oss);
        operator ()(oss.Written());
    }
    void operator ()(const Opaq::wstring_format& fmt) {
        if (PassFilter)
            return;

        STACKLOCAL_WTEXTWRITER(oss, 1024);
        fmt(oss);
        operator ()(oss.Written());
    }
};
//----------------------------------------------------------------------------
static bool LogViewer_PassFilter_(const ImGuiTextFilter& logFilter, const FLogViewerWidget::FHistoryMessage& msg) {
    if (logFilter.IsActive() && not logFilter.PassFilter(msg.Text)) {
        if (msg.Data.valid()) {
            FLogMessageFilterVisitor_ visitor{ logFilter };
            visitor(*msg.Data);
            return visitor.PassFilter;
        }
        return false;
    }
    return true;
}
//----------------------------------------------------------------------------
static FColor LogViewer_CategoryColor_(const FLoggerCategory& category) {
    return FColor::FromHash(category.HashValue, 100);
}
//----------------------------------------------------------------------------
static void LogViewer_ShowCategories_(FLogViewerWidget& widget) {
    for (FLogViewerWidget::FHistoryCategory& category : widget.Categories) {
        ImGui::PushStyleColor(ImGuiCol_Header, LogViewer_CategoryColor_(category).ToPackedABGR());

        if (ImGui::Selectable(category->Name.c_str(), category.Visible)) {
            category.Visible = not category.Visible;
            widget.FlushVisibleMessages();
        }

        ImGui::PopStyleColor();
    }
}
//----------------------------------------------------------------------------
static void LogViewer_ShowLogs_(FLogViewerWidget& widget) {
    if (not ImGui::BeginTable("##LogViewer::LogTable", 6,
        ImGuiTableFlags_Hideable |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingFixedFit))
        return;
    DEFERRED{ ImGui::EndTable(); };

    ImGui::TableSetupScrollFreeze(0, 1);

    // Resize all the table when the table has at least one message
    if (widget.LastMessageVisible == INDEX_NONE)
        ImGui::TableSetColumnWidthAutoAll(ImGui::GetCurrentTable());

    // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
    ImGui::TableSetupColumn("Time");
    ImGui::TableSetupColumn("Thread");
    ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_DefaultHide);
    ImGui::TableSetupColumn("Category");
    ImGui::TableSetupColumn("Level");
    ImGui::TableSetupColumn("Text", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableHeadersRow();

    STACKLOCAL_TEXTWRITER(tmp, 250);

    ImGuiListClipper clipper;
    clipper.Begin(checked_cast<int>(widget.VisibleMessages.size()));
    DEFERRED{ clipper.End(); };

    const FTimepoint startedAt = FCurrentProcess::Get().StartTicks();

    const auto printColoredText = [](const char* text, ImU32 color) -> void {
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(text);
        ImGui::PopStyleColor();
    };

    const auto logColor = [](FLogger::EVerbosity level) -> FColor {
        switch (level) {
        case ELoggerVerbosity::Verbose:
            return FColor::LightSeaGreen();
        case ELoggerVerbosity::Info:
            return FColor::GhostWhite();
        case ELoggerVerbosity::Profiling:
            return FColor::Magenta();
        case ELoggerVerbosity::Emphasis:
            return FColor::LawnGreen();
        case ELoggerVerbosity::Warning:
            return FColor::LightYellow();
        case ELoggerVerbosity::Error:
            return FColor::OrangeRed();
        case ELoggerVerbosity::Debug:
            return FColor::DarkCyan();
        case ELoggerVerbosity::Fatal:
            return FColor::Red();
        default:
            break;
        }
        AssertNotReached();
    };

    FStringBuilder sb;

    while (clipper.Step()) {
        forrange(i, clipper.DisplayStart, clipper.DisplayEnd) {
            const FLoggerMessage& msg = widget.VisibleMessages[i];

            ImGui::PushID(&msg);
            DEFERRED{ ImGui::PopID(); };

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();

            tmp.Reset();
            Format(tmp, "{0:#10f3}\0", *Units::Time::FSeconds(FTimepoint::Duration(startedAt, msg.Site.LogTime)));
            printColoredText(tmp.data(), FColor::DimGray().ToPackedABGR());

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();

            tmp.Reset();
            printColoredText(FThreadContext::GetThreadName(msg.Site.ThreadId), FColor::DimGray().ToPackedABGR());

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();

            tmp.Reset();
            tmp << msg.Site.LogTime.Timestamp() << Eos;
            printColoredText(tmp.data(), FColor::DarkGray().ToPackedABGR());

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();

            tmp.Reset();
            tmp << msg.Category->Name << Eos;
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, LogViewer_CategoryColor_(msg.Category).ToPackedABGR());
            ImGui::TextUnformatted(tmp.data());

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();

            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, logColor(msg.Level()).Fade(0x60).ToPackedABGR());
            ImGui::TextUnformatted(ToString(msg.Level()));

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();

            ImGui::PushStyleColor(ImGuiCol_Text, logColor(msg.Level()).ToPackedABGR());

            FStringView text = msg.Text.MakeView();
            FStringView firstLine = EatUntil(text, '\n');
            ImGui::TextUnformatted(firstLine.data(), firstLine.data() + firstLine.size());

            if (EatSpaces(text); not text.empty()) {
                ImGui::SameLine();

                if (ImGui::SmallButton("..."))
                    ImGui::OpenPopup("##LogViewer::TextPopup");

                if (ImGui::BeginPopup("##LogViewer::TextPopup", ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
                    ImGui::PushTextWrapPos(400.0f);
                    ImGui::TextUnformatted(text.data(), text.data() + text.size());
                    ImGui::PopTextWrapPos();

                    ImGui::EndPopup();
                }
            }

            ImGui::PopStyleColor();

            if (msg.Data.valid()) {
                ImGui::SameLine();
                ImGui::Dummy({0,1});

                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {2.f, 0 });

                for (const Opaq::key_value_view& it : *msg.Data) {
                    ImGui::SameLine();

                    const FColor labelColor = FColor::FromHash(hash_string(it.key.MakeView()));
                    ImGui::PushStyleColor(ImGuiCol_Button, labelColor.Fade(80).ToPackedABGR());
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, labelColor.Fade(160).ToPackedABGR());
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, labelColor.ToPackedABGR());

                    sb << FTextFormat::Compact << it << Eos;
                    ImGui::SmallButton(sb.c_str());
                    sb.clear();

                    ImGui::PopStyleColor(3);
                }

                ImGui::PopStyleVar(2);
            }
        }
    }

    if (widget.LastMessagePosted != widget.LastMessageVisible) {
        if (widget.AutoScroll || widget.LastMessageVisible == INDEX_NONE) {
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        widget.LastMessageVisible = widget.LastMessagePosted;
    }
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FLogViewerWidget::FHistoryLogger final : public ILogger {
public:
    STATIC_CONST_INTEGRAL(size_t, HistoryCapacity, FLogViewerWidget::HistoryCapacity * 2);
    TThreadSafe<SLABHEAP(UI), EThreadBarrier::CriticalSection> Heap;
    TThreadSafe<RINGBUFFER(UI, FHistoryMessage, HistoryCapacity), EThreadBarrier::RWLock> History;

    void LogMessage(const FLoggerMessage& msg) override final {
        // duplicate the transient message in local ring buffer
        const size_t textSize = (msg.IsTextAllocated() ? msg.Text.MakeView().SizeInBytes() + 1/*'\0'*/ : 0);
        const size_t dataSize = (msg.Data.valid() ? BlockSize(msg.Data) : 0);

        FHistoryMessage dup;
        dup.Category = msg.Category;
        dup.Site = msg.Site;
        dup.Block = SLAB_ALLOCATOR(UI)(*Heap.LockExclusive()).Allocate(textSize + dataSize);

        // non-formatted strings are static -this is enforced by FLogger- and don't need to be copied
        if (msg.IsTextAllocated()) {
            const TMemoryView<char> text = dup.Block.MakeView().CutBefore(textSize).Cast<char>();
            const size_t lenCopied = Copy(text, msg.Text.MakeView());
            Assert_NoAssume(textSize - 1 == lenCopied);
            text[lenCopied] = '\0';
            dup.Text = text.data();
        }
        else {
            dup.Text = msg.Text;
        }

        // duplicate message opaque data, if any
        if (msg.Data.valid()) {
            const FAllocatorBlock data = FAllocatorBlock::From(dup.Block.MakeView().CutStartingAt(textSize));
            dup.Data = std::get<Opaq::object_view>(*NewBlock(data, msg.Data));
        }

        // push the new log in history, pop oldest message if history is already full
        FHistoryMessage overload;
        if (History.LockExclusive()->push_back_OverflowIFN(&overload, std::move(dup)))
            SLAB_ALLOCATOR(UI)(*Heap.LockExclusive()).Deallocate(overload.Block); // deallocate oldest message
    }

    void Flush(bool synchronous) override final {
        Unused(synchronous);
    }

    void ClearHistory() {
        auto exlusiveHeap = Heap.LockExclusive();
        History.LockExclusive()->clear();
        exlusiveHeap->DiscardAll();
    }
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FLogViewerWidget::FLogViewerWidget() NOEXCEPT = default;
//----------------------------------------------------------------------------
FLogViewerWidget::~FLogViewerWidget() {
    if (LoggerWasRegistered) {
        LoggerWasRegistered = false;
        FLogger::UnregisterLogger(*Logger);
    }

    Logger->ClearHistory();
}
//----------------------------------------------------------------------------
bool FLogViewerWidget::Show() {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);

    if (not ImGui::Begin(*Title, &WindowVisible, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar)) {
        ImGui::End();

        if (LoggerWasRegistered and not LoggerKeepRegistered) {
            LoggerWasRegistered = false;
            FLogger::UnregisterLogger(*Logger);
        }

        return false;
    }
    DEFERRED{ ImGui::End(); };

    RegisterLogger(LoggerKeepRegistered);

    if (NeedRefreshVisibleMessages)
        RefreshVisibleMessages();

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("View")) {

            if (ImGui::MenuItemEx("Show categories", ICON_CI_GROUP_BY_REF_TYPE, nullptr, ShowCategoriesPanel))
                ShowCategoriesPanel = not ShowCategoriesPanel;

            ImGui::Separator();

            if (ImGui::MenuItemEx("Clear", ICON_CI_CLEAR_ALL))
                VisibleMessages.clear();

            if (ImGui::MenuItemEx("Scroll down", ICON_CI_TRIANGLE_DOWN))
                LastMessageVisible = INDEX_NONE;

            if (ImGui::MenuItemEx("Auto scroll", nullptr, nullptr, AutoScroll))
                AutoScroll = (not AutoScroll);

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    {
        if (LogFilter.Draw("Filter messages"))
            FlushVisibleMessages();
    }

    // Left
    if (ShowCategoriesPanel) {
        ImGui::BeginChild("##LogViewer::Categories", ImVec2(150, -ImGui::GetFrameHeightWithSpacing()), // Leave room for 1 line below us
            ImGuiChildFlags_Border );

        LogViewer_ShowCategories_(*this);

        ImGui::EndChild();
        ImGui::SameLine();
    }

    {
        ImGui::BeginChild("##LogViewer::ListView", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));  // Leave room for 1 line below us

        DEFERRED{ ImGui::EndChild(); };

        LogViewer_ShowLogs_(*this);
    }

    {
        if (ImGui::Button(ICON_CI_CLEAR_ALL " Clear")) {
            Logger->ClearHistory();
            FlushVisibleMessages();
        }

        ImGui::SameLine();

        if (ImGui::Button(ICON_CI_TRIANGLE_DOWN " Scroll"))
            LastMessageVisible = INDEX_NONE;

        ImGui::SameLine();

        ImGui::Checkbox("Auto-Scroll", &AutoScroll);
    }

    return false;
}
//----------------------------------------------------------------------------
void FLogViewerWidget::RegisterLogger(bool keepRegistered) {
    if (not LoggerWasRegistered) {
        LoggerWasRegistered = true;
        if (not Logger)
            Logger.create<FHistoryLogger>();
        FLogger::RegisterLogger(*Logger);

        FlushVisibleMessages();
    }
    LoggerKeepRegistered = keepRegistered;
}
//----------------------------------------------------------------------------
void FLogViewerWidget::FlushVisibleMessages() {
    NeedRefreshVisibleMessages = true;
    LastMessagePosted = INDEX_NONE;
    LastMessageVisible = INDEX_NONE;
}
//----------------------------------------------------------------------------
void FLogViewerWidget::RefreshVisibleMessages() {
    if (not Logger)
        return;

    const auto sharedHistory = Logger->History.LockShared();

    if (LastMessagePosted == INDEX_NONE) {
        LastMessagePosted = sharedHistory->begin().Pos;
        VisibleMessages.clear();
    }

    auto first = sharedHistory->begin();
    first.Pos = LastMessagePosted;

    forrange(it, first, sharedHistory->end()) {
        const PHistoryMessage& historyMsg = *it;

        if (not LogViewer_PassFilter_(LogFilter, *historyMsg))
            continue;

        if (Categories.FindOrAdd(FHistoryCategory{historyMsg->Category}, nullptr)->Visible)
            Unused(VisibleMessages.push_back_OverflowIFN(nullptr, historyMsg));
    }

    LastMessagePosted = sharedHistory->end().Pos;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE

#endif //!USE_PPE_LOGGER
