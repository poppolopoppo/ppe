// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "UI/Widgets/MemoryUsageWidget.h"
#include "UI/Widgets/MemoryUsageWidget.h"

#include "UI/Imgui.h"

#include "Allocator/BitmapHeap.h"
#include "Allocator/MallocBitmap.h"
#include "Color/Color.h"
#include "Container/Stack.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformMemory.h"
#include "IO/FormatHelpers.h"
#include "IO/TextWriter.h"
#include "Maths/MathHelpers.h"
#include "Memory/MemoryDomain.h"
#include "Memory/MemoryTracking.h"
#include "Memory/PtrRef.h"
#include "Modular/Modular_fwd.h"
#include "Meta/Utility.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATIONUI_API, UI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void MemoryUsage_ShowMemoryDomainSnapshot_(
    const FMemoryTracking::FSnapshot& snapshot,
    const FMemoryTracking::FSnapshot* pParent,
    bool isMemoryDomain = true ) {
    char tmpStringBuf[100];
    FFixedSizeTextWriter tmp(tmpStringBuf);

    const auto print = [&](i64 FMemoryTracking::FSnapshot::* member, bool useExp, bool isSizeInBytes) {
        tmp.Reset();

        Format(tmp, "{0}\0", (&snapshot)->*member);

        ImGui::TableNextColumn();

        if (pParent) {
            const i64 v = (&snapshot)->*member;
            const i64 p = (pParent)->*member;
            const float fraction = (p == 0 ? 0.f : Saturate(useExp
                ? Exp(-static_cast<float>(Abs(v - p)) / 50)
                : LinearStep(v,0, p) ));

            const ImGuiStyle& style = ImGui::GetStyle();
            const ImGuiWindow* window = ImGui::GetCurrentWindow();

            const ImVec2 pos = window->DC.CursorPos;
            const ImVec2 size = ImGui::CalcItemSize({ -FLT_MIN, 0 }, ImGui::CalcItemWidth(), GImGui->FontSize + style.FramePadding.y * 2.0f);
            ImRect bb(pos, pos + size);
            bb.Expand(style.CellPadding - ImVec2{1.f, 1.f});

            const FLinearColor bgColor = FLinearColor::FromHeatmap(fraction * .6f).Fade(.33f);

            ImGui::RenderRectFilledRangeH(
                window->DrawList,
                bb,
                bgColor.Quantize(EGammaSpace::Linear).ToPackedABGR(),
                0.0f, fraction, 0);
        }

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(tmpStringBuf);

        if (ImGui::IsItemHovered() && ImGui::BeginTooltip()) {
            tmp.Reset();
            tmp << FTextFormat::Float(FTextFormat::FixedFloat, 3);
            if (isSizeInBytes)
                tmp << Fmt::SizeInBytes((&snapshot)->*member);
            else
                tmp << Fmt::CountOfElements((&snapshot)->*member);

            if (pParent)
                tmp << " - " << Fmt::Percentage((&snapshot)->*member, pParent->*member);
            tmp << Eos;

            ImGui::TextUnformatted(tmpStringBuf);
            ImGui::EndTooltip();
        }
    };

    if (isMemoryDomain) {
        ImGui::TableNextColumn();
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_TableHeaderBg));
    }

    print(&FMemoryTracking::FSnapshot::NumAllocs, false, false);
    print(&FMemoryTracking::FSnapshot::MinSize, true, true);
    print(&FMemoryTracking::FSnapshot::MaxSize, false, true);
    print(&FMemoryTracking::FSnapshot::TotalSize, false, true);
    print(&FMemoryTracking::FSnapshot::PeakAllocs, false, false);
    print(&FMemoryTracking::FSnapshot::PeakSize, false, true);
    print(&FMemoryTracking::FSnapshot::AccumulatedAllocs, false, false);
    print(&FMemoryTracking::FSnapshot::AccumulatedSize, false, true);

    if (isMemoryDomain)
        print(&FMemoryTracking::FSnapshot::SmallAllocs, false, false);
}
//----------------------------------------------------------------------------
static void MemoryUsage_ShowMemoryDomainsTableRow_(FMemoryUsageWidget& widget, const FMemoryUsageWidget::FMemoryDomain& domain) {
    if (not domain.TrackingData->WasEverUsed() && not widget.ShowEmptyMemoryDomains)
        return;

    ImGui::PushID(&domain);
    DEFERRED{ ImGui::PopID(); };

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanFullWidth;
    if (not domain.Children) {
        nodeFlags = ImGuiTreeNodeFlags_Leaf |
            ImGuiTreeNodeFlags_Bullet |
            ImGuiTreeNodeFlags_NoTreePushOnOpen |
            ImGuiTreeNodeFlags_SpanFullWidth;
    }
    else if (not domain.Parent) {
        nodeFlags |=
            ImGuiTreeNodeFlags_Framed |
            ImGuiTreeNodeFlags_DefaultOpen;
    }

    if (domain.Children) {
         if (widget.CollapseMemoryDomains && domain.Parent)
            ImGui::SetNextItemOpen(false);
        if (widget.ExpandMemoryDomains)
            ImGui::SetNextItemOpen(true);
    }

    const bool open = ImGui::TreeNodeEx(domain.TrackingData->Name(), nodeFlags);

    const FMemoryTracking::FSnapshot userSnapshot = domain.TrackingData->User();
    const FMemoryTracking::FSnapshot systemSnapshot = domain.TrackingData->System();

    if (const FMemoryTracking* pParent = domain.TrackingData->Parent(); pParent != nullptr) {
        for (const FMemoryTracking* pRoot = pParent->Parent(); pRoot; pRoot = pRoot->Parent())
            pParent = pRoot;

        const FMemoryTracking::FSnapshot userSnapshotParent = pParent->User();
        const FMemoryTracking::FSnapshot systemSnapshotParent = pParent->System();

        MemoryUsage_ShowMemoryDomainSnapshot_(userSnapshot, &userSnapshotParent);
        MemoryUsage_ShowMemoryDomainSnapshot_(systemSnapshot, &systemSnapshotParent);
    }
    else {
        MemoryUsage_ShowMemoryDomainSnapshot_(userSnapshot, nullptr);
        MemoryUsage_ShowMemoryDomainSnapshot_(systemSnapshot, nullptr);
    }

    if (open && domain.Children) {
        for (FMemoryUsageWidget::SMemoryDomain node = domain.Children; node; node = node->Siblings)
            MemoryUsage_ShowMemoryDomainsTableRow_(widget, *node);

        ImGui::TreePop();
    }
}
//----------------------------------------------------------------------------
static void MemoryUsage_ShowMemoryDomainsTable_(FMemoryUsageWidget& widget) {
    if (not ImGui::BeginTable("##MemoryUsage::MemoryDomains", 21,
        ImGuiTableFlags_BordersInner |
        ImGuiTableFlags_Hideable |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingStretchSame))
        return;
    DEFERRED{ ImGui::EndTable(); };

    ImGui::TableSetupScrollFreeze(1, 2);

    // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder, 5);

    ImGui::TableSetupColumn("User",
        ImGuiTableColumnFlags_WidthFixed |
        ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide,
        ImGui::TableGetHeaderRowHeight());

    ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Stride Min", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Stride Max", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Total Size", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Peak Count", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Peak Size", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Accumulated Count", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_DefaultHide);
    ImGui::TableSetupColumn("Accumulated Size", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_DefaultHide);
    ImGui::TableSetupColumn("Small Allocs", ImGuiTableColumnFlags_AngledHeader);

    ImGui::TableSetupColumn("System",
        ImGuiTableColumnFlags_WidthFixed |
        ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoReorder | ImGuiTableColumnFlags_NoHide,
        ImGui::TableGetHeaderRowHeight());

    ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Stride Min", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Stride Max", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Total Size", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Peak Count", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Peak Size", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Accumulated Count", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_DefaultHide);
    ImGui::TableSetupColumn("Accumulated Size", ImGuiTableColumnFlags_AngledHeader | ImGuiTableColumnFlags_DefaultHide);
    ImGui::TableSetupColumn("Small Allocs", ImGuiTableColumnFlags_AngledHeader);

    ImGui::TableAngledHeadersRow();
    ImGui::TableHeadersRow();

    for (const FMemoryUsageWidget::PMemoryDomain& pDomain : *widget.MemoryDomains.LockShared()) {
        if (not pDomain->Parent) // must start traversal with root ndoes
            MemoryUsage_ShowMemoryDomainsTableRow_(widget, *pDomain);
    }
}
//----------------------------------------------------------------------------
static void MemoryUsage_ShowAllocationSizeHistogram_(FMemoryUsageWidget& widget) {
    Unused(widget);

    TMemoryView<const u32> sizeClasses;
    TMemoryView<const FMemoryTracking> bins;
    if (not FMallocDebug::FetchAllocationHistogram(&sizeClasses, &bins)) {
        ImGui::TextUnformatted("data not available");
        return;
    }

    Assert_NoAssume(sizeClasses.size() == bins.size());

    if (not ImGui::BeginTable("##MemoryUsage::AllocationHistogram", 11,
        ImGuiTableFlags_BordersInner |
        ImGuiTableFlags_Hideable |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingFixedSame))
        return;
    DEFERRED{ ImGui::EndTable(); };

    ImGui::TableSetupScrollFreeze(1, 2);

    // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder, 20);
    ImGui::TableSetupColumn("Binned Size", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder, 150);

    ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Stride Min", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Stride Max", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Total Size", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Peak Count", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Peak Size", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Accumulated Count", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Accumulated Size", ImGuiTableColumnFlags_AngledHeader);

    ImGui::TableSetupColumn("Allocs %", ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableAngledHeadersRow();
    ImGui::TableHeadersRow();

    i64 userAccumulatedAllocs{ 0 };
    //i64 userAccumulatedSize{ 0 };
    forrange(i, 0, bins.size()) {
        const FMemoryTracking::FSnapshot user = bins[i].User();

        userAccumulatedAllocs += user.AccumulatedAllocs;
        //userAccumulatedSize += user.AccumulatedSize;
    }

    forrange(i, 0, bins.size()) {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("%u", i);

        ImGui::TableNextColumn();
        ImGui::Text("%u", sizeClasses[i]);

        const FMemoryTracking::FSnapshot user = bins[i].User();

        MemoryUsage_ShowMemoryDomainSnapshot_(user, nullptr, false);

        ImGui::TableNextColumn();

        ImGui::ProgressBar(LinearStep(user.AccumulatedAllocs, 0, userAccumulatedAllocs));
    }
}
//----------------------------------------------------------------------------
static void MemoryUsage_ShowBitmapPages_(FMemoryUsageWidget& widget) {
    Unused(widget);
#if !USE_PPE_FINAL_RELEASE
    size_t numMediumPages = 16 + // keep some slack for MT
        FMallocBitmap::DumpMediumHeapInfo(nullptr);
    size_t numLargePages = 16 + // keep some slack for MT
        FMallocBitmap::DumpLargeHeapInfo(nullptr);
    STACKLOCAL_POD_ARRAY(FBitmapPageInfo, reservedPages, numMediumPages+numLargePages);

    FBitmapHeapInfo mediumInfos;
    mediumInfos.Pages = reservedPages.CutBefore(numMediumPages);
    numMediumPages = FMallocBitmap::DumpMediumHeapInfo(&mediumInfos);

    FBitmapHeapInfo largeInfos;
    largeInfos.Pages = reservedPages.CutStartingAt(numMediumPages);
    numLargePages = FMallocBitmap::DumpLargeHeapInfo(&largeInfos);

    if (not ImGui::BeginTable("##MemoryUsage::AllocationHistogram", 10,
        ImGuiTableFlags_BordersInner |
        ImGuiTableFlags_Hideable |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingFixedFit))
        return;
    DEFERRED{ ImGui::EndTable(); };

    ImGui::TableSetupScrollFreeze(1, 2);

    // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_NoReorder);
    ImGui::TableSetupColumn("Address Space", ImGuiTableColumnFlags_NoReorder);
    ImGui::TableSetupColumn("Address End", ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_NoReorder);

    ImGui::TableSetupColumn("Allocations", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Total Allocated", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Total Committed", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Occupancy", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Fragmentation", ImGuiTableColumnFlags_AngledHeader);
    ImGui::TableSetupColumn("Largest Free", ImGuiTableColumnFlags_AngledHeader);

    ImGui::TableSetupColumn("Bitmap", ImGuiTableColumnFlags_WidthStretch);

    ImGui::TableAngledHeadersRow();
    ImGui::TableHeadersRow();

    char tmpStringBuf[100];
    FFixedSizeTextWriter tmp(tmpStringBuf);

    float hue = 0;

    FBitmapPageInfo totalInfos{};

    const auto printBitmapPages = [&](const FBitmapHeapInfo& infos, size_t numPages, size_t offset) {
        forrange(i, 0, numPages) {
            const FBitmapPageInfo& page = infos.Pages[i];

            ++totalInfos.Pages;
            totalInfos.Stats.LargestFreeBlock = Max(totalInfos.Stats.LargestFreeBlock, page.Stats.LargestFreeBlock);
            totalInfos.Stats.NumAllocations += page.Stats.NumAllocations;
            totalInfos.Stats.TotalSizeAllocated += page.Stats.TotalSizeAllocated;
            totalInfos.Stats.TotalSizeAvailable += page.Stats.TotalSizeAvailable;
            totalInfos.Stats.TotalSizeCommitted += page.Stats.TotalSizeCommitted;

            ImGui::PushID(&page);
            DEFERRED{ ImGui::PopID(); };

            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%u", offset + i);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%p", page.vAddressSpace);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%p", static_cast<u8*>(page.vAddressSpace) + infos.BitmapSize);

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%d", page.Stats.NumAllocations);

            ImGui::TableNextColumn();
            tmp.Reset();
            Format(tmp, "{0:f3}\0", Fmt::SizeInBytes(page.Stats.TotalSizeAllocated));
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(tmpStringBuf);

            ImGui::TableNextColumn();
            tmp.Reset();
            Format(tmp, "{0:f3}\0", Fmt::SizeInBytes(page.Stats.TotalSizeCommitted));
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(tmpStringBuf);

            ImGui::TableNextColumn();
            ImGui::ProgressBar(static_cast<float>(page.Stats.TotalSizeAllocated) / page.Stats.TotalSizeCommitted, { 150, 0 });

            ImGui::TableNextColumn();
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, float4(FLinearColor::FromHeatmap(1 - static_cast<float>(page.Stats.LargestFreeBlock) / page.Stats.TotalSizeAvailable)));
            ImGui::ProgressBar(1 - static_cast<float>(page.Stats.LargestFreeBlock) / page.Stats.TotalSizeAvailable, { 150, 0 });
            ImGui::PopStyleColor();

            ImGui::TableNextColumn();
            tmp.Reset();
            Format(tmp, "{0:f3}\0", Fmt::SizeInBytes(page.Stats.LargestFreeBlock));
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(tmpStringBuf);

            ImGui::TableNextColumn();
            constexpr ImGuiTableFlags table_flags =
                ImGuiTableFlags_BordersInner |
                ImGuiTableFlags_NoClip |
                ImGuiTableFlags_SizingStretchProp;

            if (ImGui::BeginTable("##MemoryUsageWiddget::BitmapPages::PageBlocks", infos.PagesPerBlock, table_flags)) {
                ImGui::TableNextRow(ImGuiTableRowFlags_None, ImGui::GetTextLineHeightWithSpacing());

                hue = FMod(hue + 1.61803398875f, 1.f);
                FColor allocCol = FLinearColor::FromPastel(hue).Quantize(EGammaSpace::Linear);

                forrange(b, 0, infos.PagesPerBlock) {
                    const u64 select = (u64(1) << b);

                    ImGui::TableNextColumn();
                    if (not (page.Pages & select))
                        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, allocCol.Fade(0xCC).ToPackedABGR());

                    if (page.Sizes & select) {
                        hue = FMod(hue + 1.61803398875f, 1.f);
                        allocCol = FLinearColor::FromPastel(hue).Quantize(EGammaSpace::Linear);
                    }
                }

                ImGui::EndTable();
            }
        }
    };

    printBitmapPages(mediumInfos, numMediumPages, 0);
    printBitmapPages(largeInfos, numLargePages, numMediumPages);

    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

    ImGui::TableNextColumn();

    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Total = ");

    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%u pages", totalInfos.Pages);

    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%d", totalInfos.Stats.NumAllocations);

    ImGui::TableNextColumn();
    tmp.Reset();
    Format(tmp, "{0:f3}\0", Fmt::SizeInBytes(totalInfos.Stats.TotalSizeAllocated));
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(tmpStringBuf);

    ImGui::TableNextColumn();
    tmp.Reset();
    Format(tmp, "{0:f3}\0", Fmt::SizeInBytes(totalInfos.Stats.TotalSizeCommitted));
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(tmpStringBuf);

    ImGui::TableNextColumn();
    ImGui::ProgressBar(static_cast<float>(totalInfos.Stats.TotalSizeAllocated) / totalInfos.Stats.TotalSizeCommitted, { 150, 0 });

    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, float4(FLinearColor::FromHeatmap(1 - static_cast<float>(totalInfos.Stats.LargestFreeBlock) / totalInfos.Stats.TotalSizeAvailable)));
    ImGui::ProgressBar(1 - static_cast<float>(totalInfos.Stats.LargestFreeBlock) / totalInfos.Stats.TotalSizeAvailable, { 150, 0 });
    ImGui::PopStyleColor();

    ImGui::TableNextColumn();
    tmp.Reset();
    Format(tmp, "{0:f3}\0", Fmt::SizeInBytes(totalInfos.Stats.LargestFreeBlock));
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(tmpStringBuf);

    ImGui::TableNextColumn();
#endif
}
//----------------------------------------------------------------------------
static void MemoryUsage_ShowMemoryPlot_(FMemoryUsageWidget& widget) {
    STATIC_CONST_INTEGRAL(int, PlotCapacity, FMemoryUsageWidget::PlotCapacity);
    using plotdata_type = FMemoryUsageWidget::plotdata_type;
    const int plotIndex = (ImGui::GetFrameCount() % PlotCapacity);
    const float sizeInMiBOO = 1.f / (1024 * 1024);

    widget.GpuPlot[plotIndex] = FMemoryTracking::GpuMemory().System().TotalSize * sizeInMiBOO;
    widget.UsedPlot[plotIndex] = FMemoryTracking::UsedMemory().System().TotalSize * sizeInMiBOO;
    widget.ReservedPlot[plotIndex] = FMemoryTracking::ReservedMemory().System().TotalSize * sizeInMiBOO;
    widget.PooledPlot[plotIndex] = FMemoryTracking::PooledMemory().System().TotalSize * sizeInMiBOO;
    widget.VirtualPlot[plotIndex] = FMemoryTracking::VirtualMemory().System().TotalSize * sizeInMiBOO;
    widget.UnaccountedPlot[plotIndex] = FMemoryTracking::UnaccountedMemory().System().TotalSize * sizeInMiBOO;

    const FPlatformMemory::FStats processMem = FPlatformMemory::Stats();

    widget.PagefileUsage[plotIndex] = processMem.UsedPhysical * sizeInMiBOO;
    widget.WorkingSetSize[plotIndex] = processMem.UsedVirtual * sizeInMiBOO;

    struct plotfunc_t {
        FConstChar Name;
        TPtrRef<const plotdata_type> Data;
        int FrameIndex;
        bool Selected{ false };
    }   plots[] = {
        { "PageFile", &widget.PagefileUsage, plotIndex },
        { "WorkingSet", &widget.WorkingSetSize, plotIndex },

        { "Gpu" , &widget.GpuPlot, plotIndex },
        { "Used" , &widget.UsedPlot, plotIndex },
        { "Reserved" , &widget.ReservedPlot, plotIndex },
        { "Pooled" , &widget.PooledPlot, plotIndex },
        { "Virtual" , &widget.VirtualPlot, plotIndex },
        { "Unaccounted" , &widget.UnaccountedPlot, plotIndex },
    };

    if (ImPlot::BeginPlot("##MemoryUsage::RollingPlotLines", ImVec2(-1, 250))) {
        ImPlot::SetupAxes(NULL, "MiB",
            ImPlotAxisFlags_NoTickLabels,
            ImPlotAxisFlags_AutoFit |
            ImPlotAxisFlags_LockMin |
            ImPlotAxisFlags_Opposite);
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, PlotCapacity, ImGuiCond_Always);

        for (auto& plot : plots) {
            ImPlot::PlotLine(plot.Name,
                plot.Data->data(),
                PlotCapacity,
                1,
                0,
                ImPlotLineFlags_None,
                plotIndex);

            if (ImPlot::BeginLegendPopup(plot.Name)) {
                ImGui::Text("%10.3f MiB", plot.Data->at(plotIndex));
                ImPlot::EndLegendPopup();
            }
        }

        // custom tool
        if (ImPlot::IsPlotHovered()) {
            const int half_width = 2;
            ImDrawList* const draw_list = ImPlot::GetPlotDrawList();
            const ImPlotPoint mouse = ImPlot::GetPlotMousePos();

            const float tool_y = ImPlot::PlotToPixels(mouse).y;
            const float tool_l = ImPlot::PlotToPixels(mouse.x - half_width * 1.5, mouse.y).x;
            const float tool_r = ImPlot::PlotToPixels(mouse.x + half_width * 1.5, mouse.y).x;
            const float tool_t = ImPlot::GetPlotPos().y;
            const float tool_b = tool_t + ImPlot::GetPlotSize().y;

            // find mouse location index
            const int mouse_idx = ((RoundToInt(float(mouse.x) / PlotCapacity) + plotIndex) % PlotCapacity);

            i32 closestPlot = -1;
            float closestDist = 0;
            forrange(i, 0, lengthof(plots)) {
                const float plot_y = ImPlot::PlotToPixels({ float(mouse_idx), plots[i].Data->at(mouse_idx) }).y;
                const float dist = Abs(plot_y - tool_y);
                if (dist > half_width * 3.f) {
                    continue;
                }
                if (closestPlot < 0 || dist < closestDist) {
                    closestPlot = i;
                    closestDist = dist;
                }
            }

            ImPlot::PushPlotClipRect();
            draw_list->AddRectFilled(ImVec2(tool_l, tool_t), ImVec2(tool_r, tool_b), IM_COL32(128, 128, 128, 64));
            ImPlot::PopPlotClipRect();

            // render tool tip (won't be affected by plot clip rect)
            if (ImGui::BeginTooltip()) {
                if (ImGui::BeginTable("##MemoryUsage::RollingPlotLines::PopupMouse", 2)) {
                    if (closestPlot < 0) {
                        for (auto& plot : plots) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::TextUnformatted(plot.Name);
                            ImGui::TableNextColumn();
                            ImGui::Text("%s%10.3f MiB", plot.Selected ? ">> " : "", plot.Data->at(mouse_idx));
                        }
                    }
                    else {
                        auto& plot = plots[closestPlot];
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(plot.Name);
                        ImGui::TableNextColumn();
                        ImGui::Text("%s%10.3f MiB", plot.Selected ? ">> " : "", plot.Data->at(mouse_idx));
                    }
                    ImGui::EndTable();
                }
                ImGui::EndTooltip();
            }
        }

        ImPlot::EndPlot();
    }
}
//----------------------------------------------------------------------------
auto MemoryUsage_FindDomain_(
    const VECTOR(UI, FMemoryUsageWidget::PMemoryDomain)& domains,
    const FMemoryTracking* pTrackingData) NOEXCEPT {
    if (pTrackingData == nullptr)
        return domains.end();
    const auto it = std::find_if(domains.begin(), domains.end(),
        [pTrackingData](const FMemoryUsageWidget::PMemoryDomain& it) NOEXCEPT -> bool {
            return (it->TrackingData.get() == pTrackingData);
        });
    return it;
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FMemoryUsageWidget::FMemoryUsageWidget() {

}
//----------------------------------------------------------------------------
FMemoryUsageWidget::~FMemoryUsageWidget() {
    if (HasMemoryDomainObserver) {
        HasMemoryDomainObserver = false;
        UnregisterTrackingDataObserver(this);
    }
}
//----------------------------------------------------------------------------
bool FMemoryUsageWidget::Show() {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);

    if (not ImGui::Begin(*Title, &WindowVisible, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar))
        return false;
    DEFERRED{ ImGui::End(); };

    if (not HasMemoryDomainObserver) {
        HasMemoryDomainObserver = true;
        RegisterTrackingDataObserver(this);
    }

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItemEx("Duty cycle for modules", ICON_FK_BELL))
                DutyCycleForModules();
            if (ImGui::MenuItemEx("Release memory in modules", ICON_FK_RECYCLE))
                ReleaseMemoryInModules();

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    MemoryUsage_ShowMemoryPlot_(*this);

    if (ImGui::BeginTabBar("##MemoryUsage::MainTabBar")) {
        if (ImGui::BeginTabItem("Domains")) {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::BeginMenu("Domains")) {

                    if (ImGui::MenuItemEx("Show empty domains", nullptr, nullptr, ShowEmptyMemoryDomains))
                        ShowEmptyMemoryDomains = (not ShowEmptyMemoryDomains);

                    ImGui::Separator();

                    if (ImGui::MenuItemEx("Collapse all domains", ICON_FK_COMPRESS))
                        CollapseMemoryDomains = true;
                    if (ImGui::MenuItemEx("Expand all domains", ICON_FK_EXPAND))
                        ExpandMemoryDomains = true;

                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            MemoryUsage_ShowMemoryDomainsTable_(*this);

            CollapseMemoryDomains = ExpandMemoryDomains = false;

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Allocation Histogram")) {
            MemoryUsage_ShowAllocationSizeHistogram_(*this);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Bitmap Pages")) {
            MemoryUsage_ShowBitmapPages_(*this);
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    return false;
}
//----------------------------------------------------------------------------
void FMemoryUsageWidget::OnRegisterTrackingData(FMemoryTracking* pTrackingData) {
    PMemoryDomain pDomain = NEW_REF(UI, FMemoryDomain, *pTrackingData);

    const auto exclusiveDomains = MemoryDomains.LockExclusive();
    AssertMessage_NoAssume("child memory domain registered before its parent", MemoryUsage_FindDomain_(*exclusiveDomains, pTrackingData) == exclusiveDomains->end());

    if (const auto parent = MemoryUsage_FindDomain_(*exclusiveDomains, pTrackingData->Parent()); exclusiveDomains->cend() != parent) {
        pDomain->Parent = *parent;
        pDomain->Siblings =  (*parent)->Children;
        (*parent)->Children = pDomain;
    }

    exclusiveDomains->push_back(std::move(pDomain));
}
//----------------------------------------------------------------------------
void FMemoryUsageWidget::OnUnregisterTrackingData(FMemoryTracking* pTrackingData) {
    const auto exclusiveDomains = MemoryDomains.LockExclusive();

    PMemoryDomain pDomain;
    {
        const auto it = MemoryUsage_FindDomain_(*exclusiveDomains, pTrackingData);
        AssertRelease(exclusiveDomains->cend() != it);
        pDomain = std::move(exclusiveDomains->at(std::distance(exclusiveDomains->cbegin(), it)));
        exclusiveDomains->erase(it);
    }
    AssertReleaseMessage_NoAssume("parent memory domain unregistered before its children", not pDomain->Children);

    if (FMemoryDomain* const pParent = pDomain->Parent; pParent) {
        SMemoryDomain prev;
        for (SMemoryDomain node = pParent->Children; node; prev = node, node = node->Siblings) {
            if (node.get() == pDomain)
                break;
        }

        if (prev) {
            Assert_NoAssume(prev->Siblings.get() == pDomain);
            prev->Siblings = pDomain->Siblings;
        }
        else {
            Assert_NoAssume(pParent->Children.get() == pDomain);
            pParent->Children = pDomain->Siblings;
        }
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
