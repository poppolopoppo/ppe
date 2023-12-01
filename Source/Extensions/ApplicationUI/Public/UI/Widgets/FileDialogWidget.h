#pragma once

#include "UI/Imgui.h"

#include "Container/RingBuffer.h"
#include "Container/Vector.h"
#include "IO/Filename.h"
#include "IO/Dirpath.h"
#include "IO/String.h"
#include "Time/Timestamp.h"
#include "Misc/Event.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FFileDialogWidget {
public:
    FString Title{ ICON_CI_FOLDER_LIBRARY " File dialog" };
    FDirpath InitialDirectory;

    enum class EView {
        Details,
        List,
        Large,
    };

    EView ViewMode{ EView::Large };

    enum class ESort {
        None,

        NameAsc,
        NameDesc,

        LastModifiedAsc,
        LastModifiedDesc,

        SizeAsc,
        SizeDesc,

        TypeAsc,
        TypeDesc,
    };

    ESort SortBy{ ESort::None };

    bool Multiselect{ false };
    bool NeedRefresh{ true };
    bool ShowReadonlyFiles{ true };
    bool WindowVisible{ true };

    ImGuiTextFilter NameFilter;

    VECTORINSITU(UI, FFilename, 1) SelectedFiles;

    struct FEntry {
        FFilename Name;
        FTimestamp LastModified{};
        u64 FileSize : 62;
        u64 IsFile : 1;
        u64 IsReadOnly : 1;

        FEntry(const FDirpath& directory) NOEXCEPT
            : Name{ directory, FBasename{} }, FileSize(0)
            , IsFile(false), IsReadOnly(false)
        {}
        FEntry(const FFilename& file) NOEXCEPT
            : Name(file), FileSize(0)
            , IsFile(true), IsReadOnly(false)
        {}
    };

    VECTOR(UI, FEntry) VisibleEntries;

    struct {
        TStaticArray<FDirectory, 8> Directories;
        u32 Read{ 0 };
        u32 Write{ 0 };
        u32 Count{ 0 };
    }   History;

    NODISCARD PPE_APPLICATIONUI_API bool Show();

    PPE_APPLICATIONUI_API void ChangeDirectory(const FDirpath& in);
    PPE_APPLICATIONUI_API bool GoBackInHistory();
    PPE_APPLICATIONUI_API bool GoForwardInHistory();
    PPE_APPLICATIONUI_API void RefreshVisibleEntries();
};
//----------------------------------------------------------------------------
class FOpenFileDialogWidget : public FFileDialogWidget {
public:
    FOpenFileDialogWidget() = default;

    NODISCARD PPE_APPLICATIONUI_API bool Show();
};
//----------------------------------------------------------------------------
class FSaveFileDialogWidget : public FFileDialogWidget {
public:
    FSaveFileDialogWidget() = default;

    NODISCARD PPE_APPLICATIONUI_API bool Show();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
