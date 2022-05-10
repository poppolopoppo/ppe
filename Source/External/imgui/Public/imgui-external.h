#pragma once

#include "HAL/PlatformMacros.h"

#ifdef CPP_CLANG
#    pragma clang system_header
#endif

#ifdef CPP_GCC
#    pragma GCC system_header
#endif

#ifndef EXPORT_PPE_EXTERNAL_IMGUI
    PRAGMA_MSVC_WARNING_PUSH()

#   ifdef __clang__
#       pragma clang diagnostic push
#   endif
#endif

#pragma include_alias("imconfig.h", "External/imgui/imgui.git/imconfig.h")
#pragma include_alias("imgui.h", "External/imgui/imgui.git/imgui.h")
#pragma include_alias("imgui_internal.h", "External/imgui/imgui.git/imgui_internal.h")

#ifndef EXPORT_PPE_EXTERNAL_IMGUI

#   pragma include_alias("imstb_rectpack.h", "External/imgui/imgui.git/imstb_rectpack.h")
#   pragma include_alias("imstb_textedit.h", "External/imgui/imgui.git/imstb_textedit.h")
#   pragma include_alias("imstb_truetype.h", "External/imgui/imgui.git/imstb_truetype.h")

#   include "External/imgui/imgui.git/imgui.h"

#   ifdef __clang__
#       pragma clang diagnostic pop
#   endif

    PRAGMA_MSVC_WARNING_POP()

#endif //!EXPORT_PPE_EXTERNAL_IMGUI
