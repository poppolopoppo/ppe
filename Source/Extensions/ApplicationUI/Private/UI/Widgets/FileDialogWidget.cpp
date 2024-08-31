// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "UI/Widgets/FileDialogWidget.h"

#include "UI/ImGui.h"
#include "imgui-internal.h"

#include "UI/UIService.h"
#include "Input/Action/InputListener.h"
#include "Input/InputService.h"

#include "Application/ApplicationService.h"

#include "Color/Color.h"
#include "Diagnostic/Logger.h"
#include "HAL/PlatformFile.h"
#include "IO/ConstChar.h"
#include "IO/ConstNames.h"
#include "IO/Dirpath.h"
#include "IO/Filename.h"
#include "IO/Format.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"
#include "Modular/ModularDomain.h"

#include "VFSModule.h"
#include "VirtualFileSystem_fwd.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATIONUI_API, UI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static FLinearColor FileColorFromExtname_(const FExtname& ext) NOEXCEPT {
    return FLinearColor::FromHash(ext.HashValue()).SetLuminance(1.f);
}
//----------------------------------------------------------------------------
static FConstChar FileIconFromExtname_(const FExtname& ext) NOEXCEPT {
    if (ext == FFS::Bin()) return ICON_FK_FILES_O;
    if (ext == FFS::Bnx()) return ICON_FK_FILES_O;
    if (ext == FFS::Raw()) return ICON_FK_FILES_O;

    if (ext == FFS::Csv()) return ICON_FK_FILE_EXCEL_O;
    if (ext == FFS::Json()) return ICON_FK_FILE_CODE_O;
    if (ext == FFS::Txt()) return ICON_FK_FILE_TEXT_O;
    if (ext == FFS::Xml()) return ICON_FK_FILE_CODE_O;

    if (ext == FFS::Bkm()) return ICON_FK_FILE_EPUB;
    if (ext == FFS::Dae()) return ICON_FK_FILE_EPUB;
    if (ext == FFS::Fx()) return ICON_FK_FILE_EPUB;
    if (ext == FFS::Obj()) return ICON_FK_FILE_EPUB;
    if (ext == FFS::Ply()) return ICON_FK_FILE_EPUB;

    if (ext == FFS::Bmp()) return ICON_FK_FILE_IMAGE_O;
    if (ext == FFS::Dds()) return ICON_FK_FILE_IMAGE_O;
    if (ext == FFS::Gif()) return ICON_FK_FILE_IMAGE_O;
    if (ext == FFS::Hdr()) return ICON_FK_FILE_IMAGE_O;
    if (ext == FFS::Jpg()) return ICON_FK_FILE_IMAGE_O;
    if (ext == FFS::Pgm()) return ICON_FK_FILE_IMAGE_O;
    if (ext == FFS::Png()) return ICON_FK_FILE_IMAGE_O;
    if (ext == FFS::Ppm()) return ICON_FK_FILE_IMAGE_O;
    if (ext == FFS::Pic()) return ICON_FK_FILE_IMAGE_O;
    if (ext == FFS::Psd()) return ICON_FK_FILE_IMAGE_O;
    if (ext == FFS::Tga()) return ICON_FK_FILE_IMAGE_O;
    if (ext == L".ico") return ICON_FK_FILE_IMAGE_O;

    if (ext == FFS::Z()) return ICON_FK_FILE_ARCHIVE_O;
    if (ext == L".zip") return ICON_FK_FILE_ARCHIVE_O;
    if (ext == L".7z") return ICON_FK_FILE_ARCHIVE_O;
    if (ext == L".rar") return ICON_FK_FILE_ARCHIVE_O;
    if (ext == L".gz") return ICON_FK_FILE_ARCHIVE_O;

    if (ext == L".ttf") return ICON_FK_FILE_WORD_O;
    if (ext == L".ods") return ICON_FK_FILE_WORD_O;

    return ICON_FK_FILE;
}
//----------------------------------------------------------------------------
static void FileDialog_Select_(FFileDialogWidget& dialog, u32 entryIndex) {
    FFileDialogWidget::FEntry& entry = dialog.VisibleEntries[entryIndex];
    if (entry.IsSelected) {
        if (not dialog.Multiselect) {
            for (u32 id : dialog.SelectedEntries)
                dialog.VisibleEntries[id].IsSelected = false;
            dialog.SelectedEntries.clear();
        }
        dialog.SelectedEntries.Insert_AssertUnique(entryIndex);
    }
    else {
        dialog.SelectedEntries.Remove_AssertExists(entryIndex);
    }
}
//----------------------------------------------------------------------------
static void FileDialog_ShowDetailsView_(FFileDialogWidget& dialog) {
    char tmpStringBuf[MAX_PATH] = {};
    FFixedSizeTextWriter tmp(tmpStringBuf);

    if (not ImGui::BeginTable("##FileDialog::Details", 5,
        ImGuiTableFlags_Hideable |
        ImGuiTableFlags_NoBordersInBody |
        ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_Reorderable |
        ImGuiTableFlags_Resizable |
        ImGuiTableFlags_SizingFixedFit ))
        return;
    DEFERRED{ ImGui::EndTable(); };

    ImGui::TableSetupScrollFreeze(1, 1);

    using ESort = FFileDialogWidget::ESort;
    ImGui::TableSetupColumn("Icon",
        ImGuiTableColumnFlags_NoReorder |
        ImGuiTableColumnFlags_NoHeaderLabel);
    ImGui::TableSetupColumn(
        dialog.SortBy == ESort::NameAsc
            ? "Name " ICON_FK_SORT_ASC :
        dialog.SortBy == ESort::NameDesc
            ? "Name " ICON_FK_SORT_DESC
            : "Name",
        ImGuiTableColumnFlags_NoHide |
        ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn(
        dialog.SortBy == ESort::LastModifiedAsc
        ? "Last modified " ICON_FK_SORT_ASC :
        dialog.SortBy == ESort::LastModifiedDesc
        ? "Last modified " ICON_FK_SORT_DESC
        : "Last modified");
    ImGui::TableSetupColumn(
        dialog.SortBy == ESort::SizeAsc
            ? "Size " ICON_FK_SORT_ASC :
        dialog.SortBy == ESort::SizeDesc
            ? "Size " ICON_FK_SORT_DESC
            : "Size");
    ImGui::TableSetupColumn(
        dialog.SortBy == ESort::TypeAsc
            ? "Type " ICON_FK_SORT_ASC :
        dialog.SortBy == ESort::TypeDesc
            ? "Type " ICON_FK_SORT_DESC
            : "Type");

    ImGui::TableNextRow(ImGuiTableRowFlags_Headers);

    if (ImGui::TableNextColumn()) {
        ImGui::TableHeader("##Icons");
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
            dialog.NeedRefresh = true;
            dialog.SortBy = ESort::None;
        }
    }
    if (ImGui::TableNextColumn()) {
        ImGui::TableHeader(ImGui::TableGetColumnName(1));
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            dialog.NeedRefresh = true;
            dialog.SortBy = (dialog.SortBy == ESort::NameAsc ? ESort::NameDesc : ESort::NameAsc);
        }
    }
    if (ImGui::TableNextColumn()) {
        ImGui::TableHeader(ImGui::TableGetColumnName(2));
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            dialog.NeedRefresh = true;
            dialog.SortBy = (dialog.SortBy == ESort::LastModifiedAsc ? ESort::LastModifiedDesc : ESort::LastModifiedAsc);
        }
    }
    if (ImGui::TableNextColumn()) {
        ImGui::TableHeader(ImGui::TableGetColumnName(3));
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            dialog.NeedRefresh = true;
            dialog.SortBy = (dialog.SortBy == ESort::SizeAsc ? ESort::SizeDesc : ESort::SizeAsc);
        }
    }
    if (ImGui::TableNextColumn()) {
        ImGui::TableHeader(ImGui::TableGetColumnName(4));
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            dialog.NeedRefresh = true;
            dialog.SortBy = (dialog.SortBy == ESort::TypeAsc ? ESort::TypeDesc : ESort::TypeAsc);
        }
    }

    ImGuiListClipper imgui_clipper;
    imgui_clipper.Begin(checked_cast<int>(dialog.VisibleEntries.size()));
    DEFERRED{ imgui_clipper.End(); };

    while (imgui_clipper.Step()) {
        forrange(i, imgui_clipper.DisplayStart, imgui_clipper.DisplayEnd) {
            FFileDialogWidget::FEntry& entry = dialog.VisibleEntries[i];

            tmp.Reset();
            if (entry.IsFile) {
                if (entry.Name.BasenameNoExt().empty())
                    tmp << entry.Name.Basename();
                else
                    tmp << entry.Name.BasenameNoExt();
            }
            else {
                tmp << entry.Name.Directory().LastDirname();
            }
            tmp << Eos;

            ImGui::TableNextRow();

            if (ImGui::TableNextColumn()) {
                ImGui::AlignTextToFramePadding();
                ImGui::Dummy({ 3.f, 0.f });
                ImGui::SameLine();

                if (entry.IsFile) {
                    ImGui::PushStyleColor(ImGuiCol_Text, float4(FileColorFromExtname_(entry.Name.Extname())));
                    ImGui::TextUnformatted(FileIconFromExtname_(entry.Name.Extname()));
                    ImGui::PopStyleColor();
                }
                else
                    ImGui::TextUnformatted(ICON_FK_FOLDER);
            }

            if (ImGui::TableNextColumn()) {
                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.f, .5f });
                if (ImGui::Selectable(tmpStringBuf, &entry.IsSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                    FileDialog_Select_(dialog, checked_cast<u32>(i));
                    if (not entry.IsFile)
                        dialog.ChangeDirectory(entry.Name.Directory());
                }
                ImGui::PopStyleVar();

                if (ImGui::BeginItemTooltip()) {
                    tmp.Reset();
                    if (entry.IsFile)
                        Format(tmp, "{0} - {1:f2}\0", entry.Name, Fmt::SizeInBytes(entry.FileSize));
                    else
                        tmp << entry.Name << Eos;

                    ImGui::TextUnformatted(tmpStringBuf);
                    ImGui::EndTooltip();
                }
            }

            if (ImGui::TableNextColumn()) {
                tmp.Reset();
                tmp << entry.LastModified << Eos;

                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(tmpStringBuf);
            }

            if (ImGui::TableNextColumn()) {
                tmp.Reset();
                if (entry.IsFile)
                    Format(tmp, "{0:f2}\0", Fmt::SizeInBytes(entry.FileSize));
                else
                    tmp << "-" << Eos;

                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(tmpStringBuf);
            }

            if (ImGui::TableNextColumn()) {
                tmp.Reset();
                tmp << entry.Name.Extname() << Eos;

                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(tmpStringBuf);
            }
        }
    }
}
//----------------------------------------------------------------------------
static void FileDialog_ShowListView_(FFileDialogWidget& dialog) {
    const float2 child_sz = float2{ 230, 0 };

    const int entriesPerRow = Max(1,
        FPlatformMaths::RoundToInt(ImGui::GetContentRegionAvail().x / (
            child_sz.x +
            ImGui::CalcTextSize(ICON_FK_FOLDER).x +
            ImGui::GetStyle().ItemInnerSpacing.x * 2 +
            ImGui::GetStyle().ItemSpacing.x * 2) +
            0.1f /* bias for early visibility */));

    ImGuiListClipper imgui_clipper;
    imgui_clipper.Begin(checked_cast<int>(dialog.VisibleEntries.size() + entriesPerRow - 1) / entriesPerRow);
    DEFERRED{ imgui_clipper.End(); };

    char tmpStringBuf[MAX_PATH] = {};

    while (imgui_clipper.Step()) {
        forrange(row, imgui_clipper.DisplayStart, imgui_clipper.DisplayEnd) {
            forrange(col, 0, entriesPerRow) {
                if (static_cast<size_t>(row * entriesPerRow + col) >= dialog.VisibleEntries.size())
                    break;

                const u32 entryIndex = checked_cast<u32>(row * entriesPerRow + col);
                FFileDialogWidget::FEntry& entry = dialog.VisibleEntries[entryIndex];

                ImGui::PushID(&entry);
                DEFERRED{ ImGui::PopID(); };

                if (col > 0)
                    ImGui::SameLine();

                FFixedSizeTextWriter tmp(tmpStringBuf);

                if (entry.IsFile) {
                    if (entry.Name.BasenameNoExt().empty())
                        tmp << entry.Name.Basename();
                    else
                        tmp << entry.Name.BasenameNoExt();
                }
                else {
                    tmp << entry.Name.Directory().LastDirname();
                }
                tmp << Eos;

                ImGui::AlignTextToFramePadding();
                if (entry.IsFile) {
                    ImGui::PushStyleColor(ImGuiCol_Text, float4(FileColorFromExtname_(entry.Name.Extname())));
                    ImGui::TextUnformatted(FileIconFromExtname_(entry.Name.Extname()));
                    ImGui::PopStyleColor();
                }
                else {
                    ImGui::TextUnformatted(ICON_FK_FOLDER);
                }

                ImGui::SameLine();

                ImGui::AlignTextToFramePadding();

                ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, { 0.f, .5f });
                if (ImGui::Selectable(tmpStringBuf, &entry.IsSelected, ImGuiSelectableFlags_SelectOnClick, child_sz)) {
                    FileDialog_Select_(dialog, entryIndex);
                    if (not entry.IsFile)
                        dialog.ChangeDirectory(entry.Name.Directory());
                }
                ImGui::PopStyleVar();

                if (entry.IsFile) {
                    if (ImGui::BeginItemTooltip()) {
                        tmp.Reset();
                        if (entry.IsFile)
                            Format(tmp, "{0} - {1:f2}\0", entry.Name, Fmt::SizeInBytes(entry.FileSize));
                        else
                            tmp << entry.Name << Eos;

                        ImGui::TextUnformatted(tmpStringBuf);
                        ImGui::EndTooltip();
                    }
                }
            }
        }
    }
}
//----------------------------------------------------------------------------
static void FileDialog_ShowLargeView_(FFileDialogWidget& dialog) {
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 3, 3 });
    DEFERRED{ ImGui::PopStyleVar(2); };

    const ImGuiStyle& imgui_style = ImGui::GetStyle();
    const float2 child_sz = float2{ 100, 160 };

    const int entriesPerRow = Max(1,
        FPlatformMaths::RoundToInt(ImGui::GetContentRegionAvail().x / (
            child_sz.x +
            imgui_style.WindowPadding.x * 2 +
            ImGui::GetStyle().ItemSpacing.x) +
            0.2f /* bias for early visibility */));

    ImGuiListClipper imgui_clipper;
    imgui_clipper.Begin(checked_cast<int>(dialog.VisibleEntries.size() + entriesPerRow - 1) / entriesPerRow);
    DEFERRED{ imgui_clipper.End(); };

    char tmpStringBuf[MAX_PATH];

    while (imgui_clipper.Step()) {
        forrange(row, imgui_clipper.DisplayStart, imgui_clipper.DisplayEnd) {
            forrange(col, 0, entriesPerRow) {
                if (static_cast<size_t>(row * entriesPerRow + col) >= dialog.VisibleEntries.size())
                    break;

                const u32 entryIndex = checked_cast<u32>(row * entriesPerRow + col);
                FFileDialogWidget::FEntry& entry = dialog.VisibleEntries[entryIndex];

                ImGui::PushID(&entry);
                DEFERRED{ ImGui::PopID(); };

                if (col > 0)
                    ImGui::SameLine();

                FFixedSizeTextWriter tmp(tmpStringBuf);

                if (entry.IsFile) {
                    if (entry.Name.BasenameNoExt().empty())
                        tmp << entry.Name.Basename();
                    else
                        tmp << entry.Name.BasenameNoExt();
                }
                else
                    tmp << entry.Name.Directory().LastDirname();
                tmp << Eos;

                {
                    ImGui::BeginChild(static_cast<ImGuiID>(entry.Name.HashValue()), child_sz + (imgui_style.WindowPadding * 2),
                        ImGuiChildFlags_Border,
                        ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoScrollWithMouse);
                    DEFERRED{ ImGui::EndChild(); };

                    ImFont* const imgui_font = ImGui::GetFont();
                    const float font_scale = imgui_font->Scale;
                    imgui_font->Scale = 1.5f;
                    ImGui::PushFont(imgui_font);

                    if (entry.IsFile) {
                        ImGui::PushStyleColor(ImGuiCol_Button, {0,0,0,0});
                        DEFERRED{ ImGui::PopStyleColor(); };

                        const ImVec2 cursorPos = ImGui::GetCursorPos();
                        if (ImGui::Selectable("##Selectable", &entry.IsSelected, ImGuiSelectableFlags_SelectOnClick, float2(child_sz.xx)))
                            FileDialog_Select_(dialog, entryIndex);

                        ImGui::SetCursorPos(cursorPos);
                        if (ImGui::Button(FileIconFromExtname_(entry.Name.Extname()), float2(child_sz.xx)))
                            entry.IsSelected = !entry.IsSelected;
                    }
                    else if (ImGui::Button(ICON_FK_FOLDER, float2(child_sz.xx)))
                        dialog.ChangeDirectory(entry.Name.Directory());

                    imgui_font->Scale = font_scale;
                    ImGui::PopFont();

                    if (ImGui::BeginItemTooltip()) {
                        if (entry.IsFile)
                            ImGui::TextUnformatted(/*alloc tolerated in loop since it's called only once*/*INLINE_FORMAT(MAX_PATH + 10,
                                "{0} - {1:f2}\0", entry.Name, Fmt::SizeInBytes(entry.FileSize)));
                        else
                            ImGui::TextUnformatted(/*alloc tolerated in loop since it's called only once*/*INLINE_FORMAT(MAX_PATH,
                                "{0}\0", entry.Name));
                        ImGui::EndTooltip();
                    }

                    if (entry.IsFile) {
                        ImGui::PushStyleColor(ImGuiCol_Separator, float4(FileColorFromExtname_(entry.Name.Extname())));
                        ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 2.f);
                        ImGui::PopStyleColor();
                    }
                    else {
                        ImGui::Separator();
                    }

                    ImGui::TextWrapped(tmpStringBuf);
                }
            }
        }
    }
}
//----------------------------------------------------------------------------
static void FileDialog_ShowHistoryNavigation_(FFileDialogWidget& dialog) {
    ImGui::BeginDisabled(dialog.History.Read + 1 >= dialog.History.Count);
    if (ImGui::Button(ICON_FK_ARROW_LEFT))
        Verify(dialog.GoBackInHistory());
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(dialog.History.Read == 0);
    if (ImGui::Button(ICON_FK_ARROW_RIGHT))
        Verify(dialog.GoForwardInHistory());
    ImGui::EndDisabled();
}
//----------------------------------------------------------------------------
static void FileDialog_ShowDirectoryBreadCrumbs_(FFileDialogWidget& dialog, const FDirectory& path) {
    const TMemoryView<const FFileSystemToken> entries = path.ExpandTokens();
    Assert(not entries.empty());

    char tmpStringBuf[MAX_PATH] = {};
    FFixedSizeTextWriter tmp(tmpStringBuf);

    forrange(i, 0, entries.size()) {
        ImGui::SameLine();
        ImGui::PushID(&entries[i]);

        ImGui::Button(i ? ICON_FK_CHEVRON_RIGHT : ICON_FK_HOME);
        if (ImGui::BeginPopupContextItem("##FileDialog::BreadCrumbs::ArrowPopup", ImGuiPopupFlags_MouseButtonLeft)) {
            if (i > 0) {
                VFS_EnumerateDir(
                    FDirectory::FromTokens(entries.CutBefore(i)),
                    false,
                    [&](const FDirectory& directory) {
                        tmp.Reset();
                        tmp << directory.LastDirname() << Eos;

                        if (ImGui::MenuItem(tmpStringBuf))
                            dialog.ChangeDirectory(directory);
                    },
                    Default );
            }
            else {
                VFS_EnumerateMountingPoints([&](const FMountingPoint& mount) {
                    tmp.Reset();
                    tmp << mount << Eos;

                    if (ImGui::MenuItem(tmpStringBuf))
                        dialog.ChangeDirectory(FDirectory(mount));
                    });
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();

        if (i == 0)
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 3.0f, 3.0f });

        tmp.Reset();
        tmp << entries[i] << Eos;

        ImGui::SameLine();
        if (ImGui::Button(tmpStringBuf))
            dialog.ChangeDirectory(FDirectory::FromTokens(entries.CutBefore(i + 1)));
    }

    ImGui::PopStyleVar();
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
void FFileDialogWidget::ChangeDirectory(const FDirectory& in) {
    if (in == InitialDirectory)
        return;

    NeedRefresh = true;
    InitialDirectory = in;

    // remember visited directories in a circular buffer

    if (History.Read != 0) {
        // we offset the write pointer and reset the history navigation when
        // changing to a new directory while navigating in history
        History.Write = (History.Write - History.Read) % History.Count;
        History.Read = 0;
        History.Count = 1;
    }

    History.Directories[History.Write] = in;
    History.Write = History.Write + 1;
    History.Count = Max(History.Write, History.Count);

    if (History.Write == History.Directories.size())
        History.Write = 0;
}
//----------------------------------------------------------------------------
bool FFileDialogWidget::GoBackInHistory() {
    if (History.Read + 1 >= History.Count)
        return false;

    NeedRefresh = true;
    InitialDirectory = History.Directories[(History.Write - ++History.Read - 1) % History.Count];
    return true;
}
//----------------------------------------------------------------------------
bool FFileDialogWidget::GoForwardInHistory() {
    if (History.Read == 0)
        return false;

    NeedRefresh = true;
    InitialDirectory = History.Directories[(History.Write - --History.Read - 1) % History.Count];
    return true;
}
//----------------------------------------------------------------------------
void FFileDialogWidget::RefreshVisibleEntries() {
    NeedRefresh = false;

    FLATSET_INSITU(UI, FFilename, 1) previouslySelectedFiles;
    previouslySelectedFiles.reserve(SelectedEntries.size());
    for (u32 entryIndex : SelectedEntries)
        previouslySelectedFiles.Insert_AssertUnique(VisibleEntries[entryIndex].Name);

    SelectedEntries.clear();
    VisibleEntries.clear();

    char tmpStringBuf[MAX_PATH] = {};

    VFS_EnumerateDir(InitialDirectory, false,
        [&](const FDirectory& directory) {
            if (NameFilter.IsActive()) {
                FFixedSizeTextWriter tmp(tmpStringBuf);
                tmp << directory.LastDirname() << Eos;

                if (not NameFilter.PassFilter(tmpStringBuf))
                    return;
            }

            VisibleEntries.emplace_back(directory);
        },
        [&](const FFilename& file) {
            if (NameFilter.IsActive()) {
                FFixedSizeTextWriter tmp(tmpStringBuf);
                tmp << file.Basename() << Eos;

                if (not NameFilter.PassFilter(tmpStringBuf))
                    return;
            }

            FFileStat stat;
            Verify(VFS_FileStats(&stat, file));

            FEntry entry(file);
            entry.FileSize = stat.SizeInBytes;
            entry.LastModified = stat.LastModified;

            if (not (stat.Mode ^ FFileStat::Write)) {
                if (not ShowReadonlyFiles)
                    return;
                entry.IsReadOnly = true;
            }

            if (previouslySelectedFiles.find(entry.Name) != previouslySelectedFiles.end()) {
                entry.IsSelected = true;
                SelectedEntries.Insert_AssertUnique(checked_cast<u32>(VisibleEntries.size()));
            }

            VisibleEntries.push_back(std::move(entry));
        });

    switch (SortBy) {
    case ESort::None: break;
    case ESort::NameAsc:
        std::stable_sort(VisibleEntries.begin(), VisibleEntries.end(), [](const FEntry& a, const FEntry& b) NOEXCEPT -> bool{
            return a.Name < b.Name;
        });
        break;
    case ESort::NameDesc:
        std::stable_sort(VisibleEntries.begin(), VisibleEntries.end(), [](const FEntry& a, const FEntry& b) NOEXCEPT -> bool{
            return b.Name < a.Name;
        });
        break;
    case ESort::LastModifiedAsc:
        std::stable_sort(VisibleEntries.begin(), VisibleEntries.end(), [](const FEntry& a, const FEntry& b) NOEXCEPT -> bool{
            return a.LastModified < b.LastModified;
        });
        break;
    case ESort::LastModifiedDesc:
        std::stable_sort(VisibleEntries.begin(), VisibleEntries.end(), [](const FEntry& a, const FEntry& b) NOEXCEPT -> bool{
            return b.LastModified < a.LastModified;
        });
        break;
    case ESort::SizeAsc:
        std::stable_sort(VisibleEntries.begin(), VisibleEntries.end(), [](const FEntry& a, const FEntry& b) NOEXCEPT -> bool{
            return a.FileSize < b.FileSize;
        });
        break;
    case ESort::SizeDesc:
        std::stable_sort(VisibleEntries.begin(), VisibleEntries.end(), [](const FEntry& a, const FEntry& b) NOEXCEPT -> bool{
            return a.FileSize > b.FileSize;
        });
        break;
    case ESort::TypeAsc:
        std::stable_sort(VisibleEntries.begin(), VisibleEntries.end(), [](const FEntry& a, const FEntry& b) NOEXCEPT -> bool{
            return a.Name.Extname() < b.Name.Extname();
        });
        break;
    case ESort::TypeDesc:
        std::stable_sort(VisibleEntries.begin(), VisibleEntries.end(), [](const FEntry& a, const FEntry& b) NOEXCEPT -> bool{
            return b.Name.Extname() < a.Name.Extname();
        });
        break;
    }
}
//----------------------------------------------------------------------------
bool FFileDialogWidget::Show() {
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);

    DEFERRED{ ImGui::End(); };
    if (not ImGui::Begin(*Title, &WindowVisible, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar))
        return false;

    InnerDraw();

    return true;
}
//----------------------------------------------------------------------------
void FFileDialogWidget::InnerDraw() {
    if (InitialDirectory.empty()) {
        const auto& vfs = FVFSModule::Get(FModularDomain::Get());
        ChangeDirectory(FDirpath(vfs.DataDir()));
    }

    if (NeedRefresh) {
        RefreshVisibleEntries();
    }

    if (ImGui::BeginMenuBar()) {
        DEFERRED{ ImGui::EndMenuBar(); };

        if (ImGui::BeginMenu("Sort")) {
            if (ImGui::MenuItemEx("None", nullptr, nullptr, SortBy == ESort::None)) {
                NeedRefresh = true;
                SortBy = ESort::None;
            }

            ImGui::Separator();

            if (ImGui::MenuItemEx("Name", SortBy == ESort::NameDesc ? ICON_FK_SORT_ALPHA_DESC : ICON_FK_SORT_ALPHA_ASC, nullptr, SortBy == ESort::NameAsc || SortBy == ESort::NameDesc)) {
                NeedRefresh = true;
                SortBy = (SortBy == ESort::NameAsc ? ESort::NameDesc : ESort::NameAsc);
            }
            if (ImGui::MenuItemEx("Last modified", SortBy == ESort::LastModifiedDesc ? ICON_FK_SORT_NUMERIC_DESC : ICON_FK_SORT_NUMERIC_ASC, nullptr, SortBy == ESort::LastModifiedAsc || SortBy == ESort::LastModifiedDesc)) {
                NeedRefresh = true;
                SortBy = (SortBy == ESort::LastModifiedAsc ? ESort::LastModifiedDesc : ESort::LastModifiedAsc);
            }
            if (ImGui::MenuItemEx("Size", SortBy == ESort::SizeDesc ? ICON_FK_SORT_NUMERIC_DESC : ICON_FK_SORT_NUMERIC_ASC, nullptr, SortBy == ESort::SizeAsc || SortBy == ESort::SizeDesc)) {
                NeedRefresh = true;
                SortBy = (SortBy == ESort::SizeAsc ? ESort::SizeDesc : ESort::SizeAsc);
            }
            if (ImGui::MenuItemEx("Type", SortBy == ESort::TypeDesc ? ICON_FK_SORT_AMOUNT_DESC : ICON_FK_SORT_AMOUNT_ASC, nullptr, SortBy == ESort::TypeAsc || SortBy == ESort::TypeDesc)) {
                NeedRefresh = true;
                SortBy = (SortBy == ESort::TypeAsc ? ESort::TypeDesc : ESort::TypeAsc);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItemEx("Details", ICON_FK_TH_LIST, nullptr, ViewMode == EView::Details)) {
                NeedRefresh = true;
                ViewMode = EView::Details;
            }
            if (ImGui::MenuItemEx("List", ICON_FK_TH, nullptr, ViewMode == EView::List)) {
                NeedRefresh = true;
                ViewMode = EView::List;
            }
            if (ImGui::MenuItemEx("Large", ICON_FK_TH_LARGE, nullptr, ViewMode == EView::Large)) {
                NeedRefresh = true;
                ViewMode = EView::Large;
            }

            ImGui::Separator();

            if (ImGui::MenuItemEx("Show read-only files", ICON_FK_LOCK, nullptr, ShowReadonlyFiles)) {
                NeedRefresh = true;
                ShowReadonlyFiles = not ShowReadonlyFiles;
            }

            ImGui::EndMenu();
        }
    }

    // Top toolbar
    {
        ImGui::BeginChild("FileDialog##BreadCrumbs", ImVec2(0, ImGui::GetFrameHeightWithSpacing()));
        DEFERRED{ ImGui::EndChild(); };

        FileDialog_ShowHistoryNavigation_(*this);

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();

        FileDialog_ShowDirectoryBreadCrumbs_(*this, InitialDirectory);

        // align search filter to the right
        ImGui::SameLine();
        const float menu_total_w = ImGui::GetContentRegionAvail().x;
        const float search_w = Max(18.f, Min(282.f, menu_total_w));
        const float dummy_w = menu_total_w - search_w;

        ImGui::Dummy({ dummy_w, 0 });
        ImGui::SameLine();
        if (NameFilter.Draw(ICON_FK_SEARCH, search_w - 18 - ImGui::GetStyle().ItemSpacing.x))
            NeedRefresh = true;
    }

    // Left
    //int selected = 0;
    //{
    //    ImGui::BeginChild("left pane", ImVec2(150, 0));
    //    for (int i = 0; i < 100; i++)
    //    {
    //        // FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
    //        char label[128];
    //        Format(label, "MyObject {0}", i);
    //        ImGui::Selectable(label, selected == i);
    //    }
    //    ImGui::EndChild();
    //}
    //ImGui::SameLine();

    // Right
    {
        ImGui::BeginChild("FileDialog##ListView", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));  // Leave room for 1 line below us
        DEFERRED{ ImGui::EndChild(); };

        switch (ViewMode) {
        case EView::Details:
            FileDialog_ShowDetailsView_(*this);
            break;
        case EView::List:
            FileDialog_ShowListView_(*this);
            break;
        case EView::Large:
            FileDialog_ShowLargeView_(*this);
            break;
        }
    }

    if (ImGui::IsItemHovered()) {
        // not defined by Imgui, guess those are not stable across all platforms
        constexpr int ImGuiMouseButton_ThumbDown = 3;
        constexpr int ImGuiMouseButton_ThumbUp   = 4;
        if (ImGui::IsMouseClicked(ImGuiMouseButton_ThumbDown))
            GoBackInHistory();
        if (ImGui::IsMouseClicked(ImGuiMouseButton_ThumbUp))
            GoForwardInHistory();
    }

    // Bottom
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 3.0f, 3.0f });
        DEFERRED{ ImGui::PopStyleVar(); };

        ImGui::AlignTextToFramePadding();
        ImGui::Text("%u element%s", VisibleEntries.size(), VisibleEntries.size() < 2 ? "" : "s");
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FOpenFileDialogWidget::PopupModal(const FModalEvent& onResult) {
    // Always center this window when appearing
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);

    if (not ImGui::BeginPopupModal(*Title, &WindowVisible, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoScrollbar))
        return false;
    DEFERRED{ ImGui::EndPopup(); };

    FFileDialogWidget::InnerDraw();

    ImGui::SameLine();

    ImGui::BeginDisabled(SelectedEntries.empty());
    if (ImGui::Button("OK", {120, 0})) {
        ImGui::CloseCurrentPopup();

        VECTORINSITU(UI, FEntry, 3) selection;
        selection.reserve(SelectedEntries.size());
        for (u32 index : SelectedEntries)
            selection.push_back(VisibleEntries[index]);

        onResult(true, FSelection{selection});
    }
    ImGui::EndDisabled();

    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();

    if (ImGui::Button("Cancel", {120, 0})) {
        ImGui::CloseCurrentPopup();

        onResult(false, FSelection{});
    }

    return true;
}
//----------------------------------------------------------------------------
void FOpenFileDialogWidget::OpenModal(IApplicationService& app, FStringView title, const FDirectory& path, FModalEvent&& onResult) {
    const EInputListenerEvent originalInputMode = app.Services().Get<IUIService>().ToggleFocus(
        app.Services(),
        EInputListenerEvent::Consumed );

    TRefPtr widget = NEW_REF(UI, FOpenFileDialogWidget);
    widget->Title.assign(title);
    widget->InitialDirectory = path;
    widget->Multiselect = false;

    widget->_onResult = std::move(onResult);
    app.OnApplicationTick().Emplace([widget, originalInputMode](const IApplicationService& app, FTimespan dt) -> bool {
        Unused(app, dt);

        ImGui::OpenPopup(*widget->Title, ImGuiPopupFlags_NoOpenOverExistingPopup);

        bool bKeepAlive = true;
        widget->PopupModal([widget{widget.get()}, &bKeepAlive](bool validated, const FSelection& selection) {
            widget->_onResult(validated, selection);
            bKeepAlive = false;
        });

        if (not ImGui::IsPopupOpen(*widget->Title))
            bKeepAlive = false;

        if (not bKeepAlive)
            app.Services().Get<IUIService>().ToggleFocus(
                app.Services(),
                originalInputMode );

        return bKeepAlive;
    });
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool FSaveFileDialogWidget::Show() {
    return FFileDialogWidget::Show();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
