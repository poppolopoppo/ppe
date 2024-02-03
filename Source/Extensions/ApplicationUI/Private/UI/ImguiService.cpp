// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "UI/ImguiService.h"

#include "RHI/FrameGraph.h"
#include "RHI/SwapchainDesc.h"
#include "RHI/SamplerDesc.h"

#include "HAL/PlatformKeyboard.h"
#include "HAL/PlatformMouse.h"
#include "HAL/PlatformWindow.h"
#include "HAL/RHIService.h"

#include "Application/ApplicationService.h"

#include "Input/GamepadState.h"
#include "Input/InputService.h"
#include "Input/KeyboardState.h"
#include "Input/MouseState.h"

#include "Diagnostic/Logger.h"
#include "IO/Format.h"
#include "IO/Filename.h"
#include "VirtualFileSystem_fwd.h"

#include "UI/Imgui.h"
#include "External/imgui/imgui.git/imgui_internal.h"

namespace PPE {
namespace Application {
EXTERN_LOG_CATEGORY(PPE_APPLICATIONUI_API, UI);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
NODISCARD bool CreateImguiPipeline_(RHI::FGPipelineID* pPipeline, RHI::IFrameGraph& fg) {
    RHI::FGraphicsPipelineDesc desc;

    desc.AddShader(RHI::EShaderType::Vertex, RHI::EShaderLangFormat::VKSL_100, "main", R"#(
        #version 450 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aUV;
        layout(location = 2) in vec4 aColor;

        //layout(push_constant) uniform uPushConstant {
        layout(set=0, binding=1, std140) uniform uPushConstant {
            vec2 uScale;
            vec2 uTranslate;
        } pc;

        out gl_PerVertex{
            vec4 gl_Position;
        };

        layout(location = 0) out struct{
            vec4 Color;
            vec2 UV;
        } Out;

        void main()
        {
            Out.Color = aColor;
            Out.UV = aUV;
            gl_Position = vec4(aPos*pc.uScale+pc.uTranslate, 0, 1);
        })#" ARGS_IF_RHIDEBUG("ImGui/Pipeline_VS"));

    desc.AddShader(RHI::EShaderType::Fragment, RHI::EShaderLangFormat::VKSL_100, "main", R"#(
        #version 450 core
        layout(location = 0) out vec4 out_Color0;

        layout(set=0, binding=0) uniform sampler2D sTexture;

        layout(location = 0) in struct{
            vec4 Color;
            vec2 UV;
        } In;

        void main()
        {
            out_Color0 = In.Color * texture(sTexture, In.UV.st);
        })#" ARGS_IF_RHIDEBUG("ImGui/Pipeline_PS"));

    *pPipeline = fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("ImGui/Pipeline"));
    if (not *pPipeline) {
        PPE_LOG(UI, Error, "failed to create imgui graphics pipeline");
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------*
NODISCARD bool CreateImguiTextureSampler_(RHI::FSamplerID* pSampler, RHI::IFrameGraph& fg) {
    RHI::FSamplerDesc desc;
    desc.SetFilter(RHI::ETextureFilter::Linear, RHI::ETextureFilter::Linear, RHI::EMipmapFilter::Linear);
    desc.SetAddressMode(RHI::EAddressMode::Repeat);
    desc.SetLodRange(-1000.f, 1000.f);

    *pSampler = fg.CreateSampler(desc ARGS_IF_RHIDEBUG("ImGui/TextureSampler"));
    if (not *pSampler) {
        PPE_LOG(UI, Error, "failed to create imgui texture sampler");
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------
static void PollImguiGamepadEvents_(ImGuiIO& io, const IInputService& input) {
    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;

    const FGamepadState* const pGamepad = input.FirstGamepadConnected();
    if (pGamepad == nullptr || not pGamepad->IsConnected())
        return;

    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;

    constexpr float DeadZone = 0.1f;

    const auto pollAnalog = [&](ImGuiKey imGuiKey0, ImGuiKey imGuiKey1, float f) {
        io.AddKeyAnalogEvent(imGuiKey0, f < -DeadZone, Saturate(-f));
        io.AddKeyAnalogEvent(imGuiKey1, f > DeadZone, Saturate(f));
    };

    pollAnalog(ImGuiKey_GamepadLStickLeft, ImGuiKey_GamepadLStickRight, pGamepad->LeftStickRaw().x);
    pollAnalog(ImGuiKey_GamepadLStickDown, ImGuiKey_GamepadLStickUp, pGamepad->LeftStickRaw().y);

    pollAnalog(ImGuiKey_GamepadRStickLeft, ImGuiKey_GamepadRStickRight, pGamepad->RightStickRaw().x);
    pollAnalog(ImGuiKey_GamepadRStickDown, ImGuiKey_GamepadRStickUp, pGamepad->RightStickRaw().y);

    io.AddKeyAnalogEvent(ImGuiKey_GamepadL2, pGamepad->LeftTrigger().Raw() > DeadZone, pGamepad->LeftTrigger().Raw());
    io.AddKeyAnalogEvent(ImGuiKey_GamepadR2, pGamepad->RightTrigger().Raw() > DeadZone, pGamepad->RightTrigger().Raw());

    const auto pollButton = [&](ImGuiKey imGuiKey, EGamepadButton gamepadButton) {
        if (pGamepad->IsButtonDown(gamepadButton))
            io.AddKeyEvent(imGuiKey, true);
        if (pGamepad->IsButtonUp(gamepadButton))
            io.AddKeyEvent(imGuiKey, false);
    };

#define MAP_GAMEPAD(_ImGuiKey, _GamepadButton) \
    pollButton(_ImGuiKey, EGamepadButton::_GamepadButton)

    MAP_GAMEPAD(ImGuiKey_GamepadDpadUp, DPadUp);
    MAP_GAMEPAD(ImGuiKey_GamepadDpadLeft, DPadLeft);
    MAP_GAMEPAD(ImGuiKey_GamepadDpadRight, DPadRight);
    MAP_GAMEPAD(ImGuiKey_GamepadDpadDown, DPadDown);

    MAP_GAMEPAD(ImGuiKey_GamepadStart, Start);
    MAP_GAMEPAD(ImGuiKey_GamepadBack, Back);

    MAP_GAMEPAD(ImGuiKey_GamepadL1, LeftShoulder);
    MAP_GAMEPAD(ImGuiKey_GamepadR1, RightShoulder);

    MAP_GAMEPAD(ImGuiKey_GamepadL3, LeftThumb);
    MAP_GAMEPAD(ImGuiKey_GamepadR3, RightThumb);

    MAP_GAMEPAD(ImGuiKey_GamepadFaceDown, A);
    MAP_GAMEPAD(ImGuiKey_GamepadFaceUp, B);
    MAP_GAMEPAD(ImGuiKey_GamepadFaceUp, Y);
    MAP_GAMEPAD(ImGuiKey_GamepadFaceLeft, X);

#undef MAP_GAMEPAD
}
//----------------------------------------------------------------------------
static void PollImguiKeyboardEvents_(ImGuiIO& io, const IInputService& input) {
    const FKeyboardState& keyb = input.Keyboard();

    if (io.WantTextInput) {
        for (const FKeyboardState::FCharacterInput& ch : input.Keyboard().CharacterInputs())
            io.AddInputCharacterUTF16(ch);
    }

    if (not io.WantCaptureKeyboard)
        return;

    // update key modifiers
    io.AddKeyEvent(ImGuiMod_Ctrl, keyb.IsKeyPressed(EKeyboardKey::Control));
    io.AddKeyEvent(ImGuiMod_Alt, keyb.IsKeyPressed(EKeyboardKey::Alt));
    io.AddKeyEvent(ImGuiMod_Shift, keyb.IsKeyPressed(EKeyboardKey::Shift));
    io.AddKeyEvent(ImGuiMod_Super, keyb.IsKeyPressed(EKeyboardKey::Super));

    const auto pollKey = [&](ImGuiKey imGuiKey, EKeyboardKey keyboardKey) {
        if (keyb.IsKeyDown(keyboardKey))
            io.AddKeyEvent(imGuiKey, true);
        if (keyb.IsKeyUp(keyboardKey))
            io.AddKeyEvent(imGuiKey, false);
    };

#define MAP_KEYBOARD(_ImGuiKey, _KeyboardKey) \
    pollKey(_ImGuiKey, EKeyboardKey::_KeyboardKey)

    MAP_KEYBOARD(ImGuiKey_0, _0);
    MAP_KEYBOARD(ImGuiKey_1, _1);
    MAP_KEYBOARD(ImGuiKey_2, _2);
    MAP_KEYBOARD(ImGuiKey_3, _3);
    MAP_KEYBOARD(ImGuiKey_4, _4);
    MAP_KEYBOARD(ImGuiKey_5, _5);
    MAP_KEYBOARD(ImGuiKey_6, _6);
    MAP_KEYBOARD(ImGuiKey_7, _7);
    MAP_KEYBOARD(ImGuiKey_8, _8);
    MAP_KEYBOARD(ImGuiKey_9, _9);

    MAP_KEYBOARD(ImGuiKey_A, A);
    MAP_KEYBOARD(ImGuiKey_B, B);
    MAP_KEYBOARD(ImGuiKey_C, C);
    MAP_KEYBOARD(ImGuiKey_D, D);
    MAP_KEYBOARD(ImGuiKey_E, E);
    MAP_KEYBOARD(ImGuiKey_F, F);
    MAP_KEYBOARD(ImGuiKey_G, G);
    MAP_KEYBOARD(ImGuiKey_H, H);
    MAP_KEYBOARD(ImGuiKey_I, I);
    MAP_KEYBOARD(ImGuiKey_J, J);
    MAP_KEYBOARD(ImGuiKey_K, K);
    MAP_KEYBOARD(ImGuiKey_L, L);
    MAP_KEYBOARD(ImGuiKey_M, M);
    MAP_KEYBOARD(ImGuiKey_N, N);
    MAP_KEYBOARD(ImGuiKey_O, O);
    MAP_KEYBOARD(ImGuiKey_P, P);
    MAP_KEYBOARD(ImGuiKey_Q, Q);
    MAP_KEYBOARD(ImGuiKey_R, R);
    MAP_KEYBOARD(ImGuiKey_S, S);
    MAP_KEYBOARD(ImGuiKey_T, _T);
    MAP_KEYBOARD(ImGuiKey_U, U);
    MAP_KEYBOARD(ImGuiKey_V, V);
    MAP_KEYBOARD(ImGuiKey_W, W);
    MAP_KEYBOARD(ImGuiKey_X, X);
    MAP_KEYBOARD(ImGuiKey_Y, Y);
    MAP_KEYBOARD(ImGuiKey_Z, Z);

    MAP_KEYBOARD(ImGuiKey_Keypad0, Numpad0);
    MAP_KEYBOARD(ImGuiKey_Keypad1, Numpad1);
    MAP_KEYBOARD(ImGuiKey_Keypad2, Numpad2);
    MAP_KEYBOARD(ImGuiKey_Keypad3, Numpad3);
    MAP_KEYBOARD(ImGuiKey_Keypad4, Numpad4);
    MAP_KEYBOARD(ImGuiKey_Keypad5, Numpad5);
    MAP_KEYBOARD(ImGuiKey_Keypad6, Numpad6);
    MAP_KEYBOARD(ImGuiKey_Keypad7, Numpad7);
    MAP_KEYBOARD(ImGuiKey_Keypad8, Numpad8);
    MAP_KEYBOARD(ImGuiKey_Keypad9, Numpad9);

    MAP_KEYBOARD(ImGuiKey_KeypadAdd, Add);
    MAP_KEYBOARD(ImGuiKey_KeypadSubtract, Subtract);
    MAP_KEYBOARD(ImGuiKey_KeypadMultiply, Multiply);
    MAP_KEYBOARD(ImGuiKey_KeypadDivide, Divide);

    MAP_KEYBOARD(ImGuiKey_F1, F1);
    MAP_KEYBOARD(ImGuiKey_F2, F2);
    MAP_KEYBOARD(ImGuiKey_F3, F3);
    MAP_KEYBOARD(ImGuiKey_F4, F4);
    MAP_KEYBOARD(ImGuiKey_F5, F5);
    MAP_KEYBOARD(ImGuiKey_F6, F6);
    MAP_KEYBOARD(ImGuiKey_F7, F7);
    MAP_KEYBOARD(ImGuiKey_F8, F8);
    MAP_KEYBOARD(ImGuiKey_F9, F9);
    MAP_KEYBOARD(ImGuiKey_F10, F10);
    MAP_KEYBOARD(ImGuiKey_F11, F11);
    MAP_KEYBOARD(ImGuiKey_F12, F12);

    MAP_KEYBOARD(ImGuiKey_UpArrow, Up);
    MAP_KEYBOARD(ImGuiKey_DownArrow, Down);
    MAP_KEYBOARD(ImGuiKey_LeftArrow, Left);
    MAP_KEYBOARD(ImGuiKey_RightArrow, Right);

    MAP_KEYBOARD(ImGuiKey_Escape, Escape);
    MAP_KEYBOARD(ImGuiKey_Space, Space);

    MAP_KEYBOARD(ImGuiKey_Pause, Pause);
    MAP_KEYBOARD(ImGuiKey_PrintScreen, PrintScreen);
    MAP_KEYBOARD(ImGuiKey_ScrollLock, ScrollLock);

    MAP_KEYBOARD(ImGuiKey_Backspace, Backspace);
    MAP_KEYBOARD(ImGuiKey_Enter, Enter);
    MAP_KEYBOARD(ImGuiKey_Tab, Tab);

    MAP_KEYBOARD(ImGuiKey_Home, Home);
    MAP_KEYBOARD(ImGuiKey_End, End);
    MAP_KEYBOARD(ImGuiKey_Insert, Insert);
    MAP_KEYBOARD(ImGuiKey_Delete, Delete);
    MAP_KEYBOARD(ImGuiKey_PageUp, PageUp);
    MAP_KEYBOARD(ImGuiKey_PageDown, PageDown);

    MAP_KEYBOARD(ImGuiKey_LeftCtrl, Control);
    MAP_KEYBOARD(ImGuiKey_RightCtrl, Control);

    MAP_KEYBOARD(ImGuiKey_LeftAlt, Alt);
    MAP_KEYBOARD(ImGuiKey_RightAlt, Alt);

    MAP_KEYBOARD(ImGuiKey_LeftShift, Shift);
    MAP_KEYBOARD(ImGuiKey_RightShift, Shift);

    MAP_KEYBOARD(ImGuiKey_LeftSuper, Super);
    MAP_KEYBOARD(ImGuiKey_RightSuper, Super);

    MAP_KEYBOARD(ImGuiKey_Menu, Menu);

    MAP_KEYBOARD(ImGuiKey_Comma, Comma);
    MAP_KEYBOARD(ImGuiKey_Minus, Minus);
    MAP_KEYBOARD(ImGuiKey_Period, Period);
    MAP_KEYBOARD(ImGuiKey_CapsLock, CapsLock);
    MAP_KEYBOARD(ImGuiKey_NumLock, NumLock);

#undef MAP_KEYBOARD
}
//----------------------------------------------------------------------------
static void PollImguiMouseEvents_(ImGuiIO& io, IInputService& input) {
    const FMouseState& mouse = input.Mouse();

    if (mouse.HasMoved())
        io.AddMousePosEvent(
            static_cast<float>(mouse.ClientX()),
            static_cast<float>(mouse.ClientY()) );

    if (Abs(mouse.WheelDeltaX().Raw()) > 0.f)
        io.AddMouseWheelEvent(mouse.WheelDeltaX().Raw(), 0.f);
    if (Abs(mouse.WheelDeltaY().Raw()) > 0.f)
        io.AddMouseWheelEvent(0.f, mouse.WheelDeltaY().Raw());

    const auto pollButton = [&](int buttonIndex, EMouseButton btn) {
        if (mouse.IsButtonDown(btn))
            io.AddMouseButtonEvent(buttonIndex, true);
        if (mouse.IsButtonUp(btn))
            io.AddMouseButtonEvent(buttonIndex, false);
    };

    pollButton(0, EMouseButton::Button0);
    pollButton(1, EMouseButton::Button1);
    pollButton(2, EMouseButton::Button2);
    pollButton(3, EMouseButton::Thumb0);
    pollButton(4, EMouseButton::Thumb1);

    FGenericWindow* const window = input.FocusedWindow();
    if (not window)
        return;

    window->SetCursorCapture(io.WantCaptureMouse);

    if (io.WantSetMousePos)
        window->SetCursorPosition(
            RoundToInt(io.MousePos.x),
            RoundToInt(io.MousePos.y));

    if (not (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)) {
        const ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();

        if (imguiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor) {
            window->SetCursorType(ECursorType::Invisible);
        }
        else {
            FPlatformMouse::ECursorType newCursor;

            switch (imguiCursor) {
            case ImGuiMouseCursor_Arrow: newCursor = FPlatformMouse::ECursorType::Arrow; break;
            case ImGuiMouseCursor_TextInput: newCursor = FPlatformMouse::ECursorType::IBeam; break;
            case ImGuiMouseCursor_ResizeAll: newCursor = FPlatformMouse::ECursorType::SizeAll; break;
            case ImGuiMouseCursor_ResizeEW: newCursor = FPlatformMouse::ECursorType::SizeLeft; break;
            case ImGuiMouseCursor_ResizeNS: newCursor = FPlatformMouse::ECursorType::SizeBottom; break;
            case ImGuiMouseCursor_ResizeNESW: newCursor = FPlatformMouse::ECursorType::SizeBottomLeft; break;
            case ImGuiMouseCursor_ResizeNWSE: newCursor = FPlatformMouse::ECursorType::SizeBottomRight; break;
            case ImGuiMouseCursor_Hand: newCursor = FPlatformMouse::ECursorType::Hand; break;
            case ImGuiMouseCursor_NotAllowed: newCursor = FPlatformMouse::ECursorType::No; break;
            default:
                newCursor = Default;
                break;
            }

            window->SetCursorType(newCursor);
        }
    }
}
//----------------------------------------------------------------------------
static bool LoadImGuiFonts_(const FFilename& baseFontTTF, float baseFontSize) {
    PPE_LOG(UI, Info, "load base font for text from {0}", baseFontTTF);

    ImGuiIO& io = ImGui::GetIO();

    // using FiraCode as base font for text
    ImFont* const imBaseFont = io.Fonts->AddFontFromFileTTF(
        *ToString(VFS_Unalias(baseFontTTF)), baseFontSize);
    PPE_LOG_CHECK(UI, imBaseFont);
    io.FontDefault = imBaseFont;

    // merged additional fonts to display cute icons
    // order of declarations seems to matter: glyph ranges should be sorted by ascending order
    static CONSTEXPR const struct iconfont_type {
        FWStringView FilenameTTF;
        ImWchar GlyphRanges[3];
        float FontScale;
        float GlyphOffsetY;
    }   GIconFonts[] = {
        // using Kenney for game/pad icons
        { WIDESTRING(FONT_ICON_FILE_NAME_KI), { ICON_MIN_KI, ICON_MAX_16_KI, 0 }, 1.f, 0.f },
        // using Visual Studio Codicons for debugger/symbols icons
        { WIDESTRING(FONT_ICON_FILE_NAME_CI), { ICON_MIN_CI, ICON_MAX_16_CI, 0 }, 1.f, 1.f/5 },
        // using ForkAwesome for base icons
        { WIDESTRING(FONT_ICON_FILE_NAME_FK), { ICON_MIN_FK, ICON_MAX_16_FK, 0 }, 1.f, 0.f },
    };

    for (const iconfont_type& iconsFontInfo : GIconFonts) {
        Assert_NoAssume(iconsFontInfo.GlyphRanges[0] <= iconsFontInfo.GlyphRanges[1]);

        const float iconsFontSize = (baseFontSize * iconsFontInfo.FontScale);

        ImFontConfig iconsConfig{};
        iconsConfig.MergeMode = true; // append to base font
        iconsConfig.PixelSnapH = true;
        iconsConfig.GlyphMinAdvanceX = iconsFontSize;
        iconsConfig.GlyphOffset.y += baseFontSize * iconsFontInfo.GlyphOffsetY;

        const FFilename iconsFontTTF(INLINE_WFORMAT(MAX_PATH,
            L"Data://Fonts/Icons/{0}", iconsFontInfo.FilenameTTF).MakeView());

        PPE_LOG(UI, Info, "load icons font glyphs [{0}, {1}] from {2}",
            int(iconsFontInfo.GlyphRanges[0]),
            int(iconsFontInfo.GlyphRanges[1]),
            iconsFontTTF);

        PPE_LOG_CHECK(UI, !!io.Fonts->AddFontFromFileTTF(
            *ToString(VFS_Unalias(iconsFontTTF)),
            iconsFontSize,
            &iconsConfig,
            iconsFontInfo.GlyphRanges));
    }

    return true;
}
//----------------------------------------------------------------------------
static void* ImGuiMemAlloc_(size_t sz, void* user_data) {
    Unused(user_data);
    return TRACKING_MALLOC(ImGui, sz);
}
//----------------------------------------------------------------------------
static void ImGuiMemFree_(void* ptr, void* user_data) {
    Unused(user_data);
    return TRACKING_FREE(ImGui, ptr);
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FImGuiService::FImGuiService()
:   _clearColor(0.45f, 0.55f, 0.60f, 1.00f) {
    IMGUI_CHECKVERSION();

    ImGui::SetAllocatorFunctions(&ImGuiMemAlloc_, &ImGuiMemFree_);

    _imGuiContext = ImGui::CreateContext();
    AssertRelease_NoAssume(_imGuiContext);

    _imPlotContext = ImPlot::CreateContext();
    AssertRelease_NoAssume(_imPlotContext);
}
//----------------------------------------------------------------------------
FImGuiService::~FImGuiService() {
    ImPlot::DestroyContext(_imPlotContext);
    ImGui::DestroyContext(_imGuiContext);
}
//----------------------------------------------------------------------------
bool FImGuiService::Construct(IApplicationService& app, IInputService& input, IRHIService& rhi) {
    Assert(not _onInputUpdate);
    Assert(not _onRenderFrame);
    Assert(_textureResources.empty());

    PPE_LOG(UI, Info, "creating ImGui service");

    const RHI::SFrameGraph fg = rhi.FrameGraph();

    // initialize options
    {
        ImGuiIO& io = ImGui::GetIO();

        io.BackendPlatformName = "PPE::Application::FImGuiService";
        io.BackendPlatformUserData = this;

        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // Support for large meshes (https://github.com/ocornut/imgui/issues/2591)
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        if (rhi.Features() & ERHIFeature::HighDPIAwareness) {
            io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
            io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
        }

        if (FPlatformMouse::HasMouse) {
            io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
            io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos requests (optional, rarely used)
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
        }

        if (FPlatformKeyboard::HasKeyboard) {
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        }

        // enable docking
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // need to update display size before first call to ImGui::NewFrame()
        RHI::FSwapchainDesc swapchain = fg->Description(rhi.Swapchain());
        io.DisplaySize = {
            static_cast<float>(swapchain.Dimensions.x),
            static_cast<float>(swapchain.Dimensions.y) };

        // setup text and icons fonts
        PPE_LOG_CHECK(UI, LoadImGuiFonts_(L"Data://Fonts/FiraSansExtraCondensed-Regular.ttf", 16.f));
    }

    // initialize style
    {
#if 0
        ImGui::StyleColorsClassic();
        ImGuiStyle& style = ImGui::GetStyle();

        style.FramePadding.y = 2;
        style.FrameRounding = 3;
        style.ItemSpacing.y = 3;
        style.WindowRounding = 4;
        style.GrabRounding = 2;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.05f, 0.11f, 0.85f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.07f, 0.10f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.63f, 0.57f, 0.79f, 0.07f);

#elif 0 // https://github.com/GraphicsProgramming/dear-imgui-styles
        ImGuiStyle& style = ImGui::GetStyle();

        style.WindowMinSize        = ImVec2( 160, 20 );
        style.FramePadding         = ImVec2( 4, 2 );
        style.ItemSpacing          = ImVec2( 6, 2 );
        style.ItemInnerSpacing     = ImVec2( 6, 4 );
        style.Alpha                = 0.95f;
        style.WindowRounding       = 4.0f;
        style.FrameRounding        = 2.0f;
        style.IndentSpacing        = 6.0f;
        style.ItemInnerSpacing     = ImVec2( 2, 4 );
        style.ColumnsMinSpacing    = 50.0f;
        style.GrabMinSize          = 14.0f;
        style.GrabRounding         = 16.0f;
        style.ScrollbarSize        = 12.0f;
        style.ScrollbarRounding    = 16.0f;

        style.Colors[ImGuiCol_Text]                  = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
        style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.86f, 0.93f, 0.89f, 0.28f);
        style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
        style.Colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
        style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
        style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
        style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
        style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
        style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
        style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
        style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_Button]                = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
        style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
        style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_Header]                = ImVec4(0.92f, 0.18f, 0.29f, 0.76f);
        style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
        style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_Separator]             = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
        style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
        style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
        style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.92f, 0.18f, 0.29f, 0.78f);
        style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
        style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
        style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.92f, 0.18f, 0.29f, 0.43f);
        style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.20f, 0.22f, 0.27f, 0.9f);
        style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.20f, 0.22f, 0.27f, 0.73f);

        style.Colors[ImGuiCol_Tab]                = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
        style.Colors[ImGuiCol_TabHovered]         = ImVec4(0.92f, 0.18f, 0.29f, 0.86f);
        style.Colors[ImGuiCol_TabActive]          = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);
#elif 1 // Darcula style by ice1000 from ImThemes
        ImGuiStyle& style = ImGui::GetStyle();

        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.6000000238418579f;
        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.WindowRounding = 5.300000190734863f;
        style.WindowBorderSize = 1.0f;
        style.WindowMinSize = ImVec2(32.0f, 32.0f);
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ChildRounding = 0.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupRounding = 0.0f;
        style.PopupBorderSize = 1.0f;
        style.FramePadding = ImVec2(4.0f, 3.0f);
        style.FrameRounding = 2.299999952316284f;
        style.FrameBorderSize = 1.0f;
        style.ItemSpacing = ImVec2(8.0f, 6.5f);
        style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
        style.CellPadding = ImVec2(4.0f, 2.0f);
        style.IndentSpacing = 21.0f;
        style.ColumnsMinSpacing = 6.0f;
        style.ScrollbarSize = 14.0f;
        style.ScrollbarRounding = 5.0f;
        style.GrabMinSize = 10.0f;
        style.GrabRounding = 2.299999952316284f;
        style.TabRounding = 4.0f;
        style.TabBorderSize = 0.0f;
        style.TabMinWidthForCloseButton = 0.0f;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

        style.Colors[ImGuiCol_Text] = ImVec4(0.7333333492279053f, 0.7333333492279053f, 0.7333333492279053f, 1.0f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.3450980484485626f, 0.3450980484485626f, 0.3450980484485626f, 1.0f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.2352941185235977f, 0.2470588237047195f, 0.2549019753932953f, 0.9399999976158142f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.2352941185235977f, 0.2470588237047195f, 0.2549019753932953f, 0.0f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.2352941185235977f, 0.2470588237047195f, 0.2549019753932953f, 0.9399999976158142f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.3333333432674408f, 0.3333333432674408f, 0.3333333432674408f, 0.5f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 0.0f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.168627455830574f, 0.168627455830574f, 0.168627455830574f, 0.5400000214576721f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.4509803950786591f, 0.6745098233222961f, 0.9960784316062927f, 0.6700000166893005f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.4705882370471954f, 0.4705882370471954f, 0.4705882370471954f, 0.6700000166893005f);

        // customized
        // style.Colors[ImGuiCol_TitleBg] = ImVec4(0.03921568766236305f, 0.03921568766236305f, 0.03921568766236305f, 1.0f);
        // style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.5099999904632568f);
        // style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1568627506494522f, 0.2862745225429535f, 0.47843137383461f, 1.0f);
        style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
        style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
        style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.92f, 0.18f, 0.29f, 1.00f);

        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2705882489681244f, 0.2862745225429535f, 0.2901960909366608f, 0.800000011920929f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2705882489681244f, 0.2862745225429535f, 0.2901960909366608f, 0.6000000238418579f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2196078449487686f, 0.3098039329051971f, 0.4196078479290009f, 0.5099999904632568f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2196078449487686f, 0.3098039329051971f, 0.4196078479290009f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1372549086809158f, 0.1921568661928177f, 0.2627451121807098f, 0.9100000262260437f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.8980392217636108f, 0.8980392217636108f, 0.8980392217636108f, 0.8299999833106995f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.6980392336845398f, 0.6980392336845398f, 0.6980392336845398f, 0.6200000047683716f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.2980392277240753f, 0.2980392277240753f, 0.2980392277240753f, 0.8399999737739563f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.3333333432674408f, 0.3529411852359772f, 0.3607843220233917f, 0.4900000095367432f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.2196078449487686f, 0.3098039329051971f, 0.4196078479290009f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1372549086809158f, 0.1921568661928177f, 0.2627451121807098f, 1.0f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.3333333432674408f, 0.3529411852359772f, 0.3607843220233917f, 0.5299999713897705f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4509803950786591f, 0.6745098233222961f, 0.9960784316062927f, 0.6700000166893005f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.4705882370471954f, 0.4705882370471954f, 0.4705882370471954f, 0.6700000166893005f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.3137255012989044f, 0.3137255012989044f, 0.3137255012989044f, 1.0f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3137255012989044f, 0.3137255012989044f, 0.3137255012989044f, 1.0f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3137255012989044f, 0.3137255012989044f, 0.3137255012989044f, 1.0f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.8500000238418579f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.6000000238418579f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.8999999761581421f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.1764705926179886f, 0.3490196168422699f, 0.5764706134796143f, 0.8619999885559082f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 0.800000011920929f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.196078434586525f, 0.407843142747879f, 0.6784313917160034f, 1.0f);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667014360428f, 0.1019607856869698f, 0.1450980454683304f, 0.9724000096321106f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 1.0f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.6078431606292725f, 0.6078431606292725f, 0.6078431606292725f, 1.0f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6000000238418579f, 0.0f, 1.0f);
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
        style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
        style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.1843137294054031f, 0.3960784375667572f, 0.7921568751335144f, 0.8999999761581421f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
#elif 0 // https://github.com/ocornut/imgui/issues/707#issuecomment-1494706165
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 5.3f;
        style.FrameRounding = 2.3f;
        style.ScrollbarRounding = 0;

        style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
        style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
        style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
        style.Colors[ImGuiCol_Border]                = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
        style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
        style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
        style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
        style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
        style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
        style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
        style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
        style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
        style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
        style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
        style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
        style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
        style.Colors[ImGuiCol_Button]                = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
        style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
        style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_Header]                = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
        style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
        style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
        style.Colors[ImGuiCol_Separator]             = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.70f, 0.60f, 0.60f, 1.00f);
        style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.90f, 0.70f, 0.70f, 1.00f);
        style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
        style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
        style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
        style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);
        style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
#endif
    }

    // initialize font atlas
    {
        unsigned char* textureData;
        int textureWidth, textureHeight;
        _imGuiContext->IO.Fonts->GetTexDataAsRGBA32(&textureData, &textureWidth, &textureHeight);
    }

    PPE_LOG_CHECK(UI, CreateImguiPipeline_(&_pipeline, *fg));
    PPE_LOG_CHECK(UI, CreateImguiTextureSampler_(&_textureSampler, *fg));

    _onBeginTick = app.OnApplicationBeginTick().Bind<&FImGuiService::OnBeginTick>(this);
    _onEndTick = app.OnApplicationEndTick().Bind<&FImGuiService::OnEndTick>(this);

    _onInputUpdate = input.OnInputUpdate().Bind<&FImGuiService::OnUpdateInput>(this);
    _onWindowFocus = input.OnWindowFocus().Bind<&FImGuiService::OnWindowFocus>(this);

    _onRenderFrame = rhi.OnRenderFrame().Bind<&FImGuiService::OnRenderFrame>(this);
    _onWindowResized = rhi.OnWindowResized().Bind<&FImGuiService::OnWindowResized>(this);

    return true;
}
//----------------------------------------------------------------------------
void FImGuiService::TearDown(IApplicationService& app, IInputService& input, IRHIService& rhi) {
    PPE_LOG(UI, Info, "destroying imgui service");

    app.OnApplicationBeginTick().Remove(_onBeginTick);
    app.OnApplicationEndTick().Remove(_onEndTick);

    input.OnInputUpdate().Remove(_onInputUpdate);
    input.OnWindowFocus().Remove(_onWindowFocus);

    rhi.OnRenderFrame().Remove(_onRenderFrame);
    rhi.OnWindowResized().Remove(_onWindowResized);

    _textureResources.clear_ReleaseMemory();

    rhi.FrameGraph()->ReleaseResources(
        _fontTexture,
        _textureSampler,
        _pipeline,
        _indexBuffer,
        _vertexBuffer,
        _uniformBuffer );

    {
        ImGuiIO& io = ImGui::GetIO();

        io.BackendPlatformName = nullptr;
        io.BackendPlatformUserData = nullptr;
    }
}
//----------------------------------------------------------------------------
void FImGuiService::OnBeginTick(const PPE::IApplicationService& ) {
    _wasFrameRendered = false;

    ImGui::NewFrame();
}
//----------------------------------------------------------------------------
void FImGuiService::OnEndTick(const PPE::IApplicationService& ) {
    if (not _wasFrameRendered)
        ImGui::EndFrame();
}
//----------------------------------------------------------------------------
void FImGuiService::OnUpdateInput(IInputService& input, FTimespan dt) {
    ImGuiIO& io = ImGui::GetIO();
    PPE_LOG_CHECKVOID(UI, io.Fonts->IsBuilt());

    io.DeltaTime = static_cast<float>(*FSeconds{ dt });

    PollImguiGamepadEvents_(io, input);
    PollImguiKeyboardEvents_(io, input);
    PollImguiMouseEvents_(io, input);
}
//----------------------------------------------------------------------------
void FImGuiService::OnWindowFocus(const IInputService& input, const FGenericWindow* ) {
    ImGuiIO& io = ImGui::GetIO();

    io.AddFocusEvent(!!input.FocusedWindow());
}
//----------------------------------------------------------------------------
void FImGuiService::OnRenderFrame(const IRHIService& rhi, FTimespan ) {
    using namespace RHI;

    _wasFrameRendered = true;

    ImGui::Render();

    ImDrawData* const pDrawData = ImGui::GetDrawData();
    if (not pDrawData || pDrawData->TotalVtxCount <= 0)
        return;

    const SFrameGraph fg = rhi.FrameGraph();

    FCommandBufferBatch cmd = fg->Begin(FCommandBufferDesc{EQueueType::Graphics}.SetName("ImGui"));
    PPE_LOG_CHECKVOID(UI, !!cmd);

    const FRawImageID swapchainImage = cmd->SwapchainImage(rhi.Swapchain());
    PPE_LOG_CHECKVOID(UI, !!swapchainImage);

    const float2 viewport{ pDrawData->DisplaySize.x, pDrawData->DisplaySize.y };

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{uint2(viewport)}
        .AddViewport(viewport)
        .AddTarget(ERenderTargetID::Color0, swapchainImage, _clearColor, EAttachmentStoreOp::Store));

    PFrameTask tDrawUI = PrepareRenderCommand_(*fg, cmd, renderPass, {});
    PPE_LOG_CHECKVOID(UI, !!tDrawUI);

    PPE_LOG_CHECKVOID(UI, fg->Execute(cmd));
}
//----------------------------------------------------------------------------
void FImGuiService::OnWindowResized(const IRHIService&, const FRHISurfaceCreateInfo& surface) {
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = {
        static_cast<float>(surface.Dimensions.x),
        static_cast<float>(surface.Dimensions.y) };
}
//----------------------------------------------------------------------------
RHI::PFrameTask FImGuiService::PrepareRenderCommand_(
    RHI::IFrameGraph& fg,
    RHI::FCommandBufferBatch& cmd,
    RHI::FLogicalPassID renderPass,
    TMemoryView<const RHI::PFrameTask> dependencies ) {
    PPE_LOG_CHECK(UI, cmd && _pipeline);

    using namespace RHI;

    ImDrawData* const pDrawData = ImGui::GetDrawData();
    PPE_LOG_CHECK(UI, pDrawData);
    Assert(pDrawData->TotalVtxCount > 0);

    FSubmitRenderPass submit{ renderPass };

    submit.DependsOn(CreateFontTexture_(cmd));
    submit.DependsOn(RecreateBuffers_(cmd));
    submit.DependsOn(UpdateUniformBuffer_(cmd));

    for (const auto& dep : dependencies)
        submit.DependsOn(dep);

    FVertexInputState vertexInput;
    vertexInput.Bind(Default, sizeof(ImDrawVert));
    vertexInput.Add("aPos"_vertex, EVertexFormat::Float2, Meta::StandardLayoutOffset(&ImDrawVert::pos));
    vertexInput.Add("aUV"_vertex, EVertexFormat::Float2, Meta::StandardLayoutOffset(&ImDrawVert::uv));
    vertexInput.Add("aColor"_vertex, EVertexFormat::UByte4_Norm, Meta::StandardLayoutOffset(&ImDrawVert::col));

    u32 indexOffset{0}, vertexOffset{0};
    forrange(i, 0, pDrawData->CmdListsCount) {
        const ImDrawList& drawList = *pDrawData->CmdLists[i];

        forrange(j, 0, drawList.CmdBuffer.Size) {
            const ImDrawCmd& drawCmd = drawList.CmdBuffer[j];
            if (drawCmd.ElemCount <= 0)
                continue;

            if (Likely(not drawCmd.UserCallback)) {
                FRectangleI scissor;
                scissor.SetLeft(RoundToInt(drawCmd.ClipRect.x));
                scissor.SetTop(RoundToInt(drawCmd.ClipRect.y));
                scissor.SetRight(RoundToInt(drawCmd.ClipRect.z));
                scissor.SetBottom(RoundToInt(drawCmd.ClipRect.w));

                scissor.SetMinMax(
                    Max(scissor.Min(), int2::Zero),
                    Max(scissor.Max(), int2::Zero));

                FImageID textureCmd{ _fontTexture.Id };
                if (const FResourceHandle textureHandle{ drawCmd.GetTexID() }; textureHandle != Default)
                    textureCmd = FImageID::Unpack(textureHandle);

                PPipelineResources resources = FindOrAddTextureResources_(fg, textureCmd);
                Assert_NoAssume(resources.valid());

                 FDrawIndexed draw = FDrawIndexed{}
                    .SetPipeline(_pipeline)
                    .AddResources("0"_descriptorset, std::move(resources))
                    .SetTopology(EPrimitiveTopology::TriangleList)
                    .SetIndexBuffer(_indexBuffer, 0, IndexAttrib<ImDrawIdx>())
                    .AddVertexBuffer(Default, _vertexBuffer)
                    .SetVertexInput(vertexInput)
                    .AddColorBuffer(ERenderTargetID::Color0, EBlendFactor::SrcAlpha, EBlendFactor::OneMinusSrcAlpha, EBlendOp::Add)
                    .SetEnableDepthTest(false)
                    .SetCullMode(ECullMode::None)
                    .AddScissor(FRectangleU(scissor))
                    .Draw(drawCmd.ElemCount, 1, indexOffset, checked_cast<i32>(vertexOffset + drawCmd.VtxOffset), 0);
                cmd->Task(renderPass, draw);

            } else {
                drawCmd.UserCallback(&drawList, &drawCmd);
            }

            indexOffset += drawCmd.ElemCount;
        }

        vertexOffset += drawList.VtxBuffer.Size;
    }

    GCUnusedTextureResources_(fg);

    return cmd->Task(submit);
}
//----------------------------------------------------------------------------
RHI::PFrameTask FImGuiService::CreateFontTexture_(const RHI::FCommandBufferBatch& cmd) {
    using namespace RHI;

    if (_fontTexture)
        return nullptr;

    int2 textureDim;
    unsigned char* textureData;
    _imGuiContext->IO.Fonts->GetTexDataAsRGBA32(&textureData, &textureDim.x, &textureDim.y);

    const size_t uploadSize = sizeof(char) * 4 * static_cast<size_t>(textureDim.x) * textureDim.y;

    uint2 fontDim = checked_cast<u32>(textureDim);

    _fontTexture = cmd->FrameGraph()->CreateImage(FImageDesc{}
        .SetDimension(fontDim)
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Sampled | EImageUsage::TransferDst),
        Default ARGS_IF_RHIDEBUG("ImGui/FontTexture"));
    PPE_LOG_CHECK(UI, _fontTexture);

    return cmd->Task(FUpdateImage{}
        .SetImage(_fontTexture)
        .SetData(FRawMemoryConst{textureData, uploadSize}, checked_cast<u32>(textureDim)));
}
//----------------------------------------------------------------------------
RHI::PFrameTask FImGuiService::UpdateUniformBuffer_(const RHI::FCommandBufferBatch& cmd) {
    using namespace RHI;

    const ImDrawData& drawData = *ImGui::GetDrawData();

    if (not _uniformBuffer) {
        _uniformBuffer = cmd->FrameGraph()->CreateBuffer(FBufferDesc{
            16_b, EBufferUsage::Uniform | EBufferUsage::TransferDst },
            Default ARGS_IF_RHIDEBUG("ImGui/UniformBuffer"));
        PPE_LOG_CHECK(UI, _uniformBuffer);
    }

    float4 pcData{Meta::NoInit};
    pcData.xy = 2 / float2(drawData.DisplaySize * _imGuiContext->IO.DisplayFramebufferScale);
    pcData.zw = -1 - float2(drawData.DisplayPos) * pcData.xy;

    return cmd->Task(FUpdateBuffer{}
        .SetBuffer(_uniformBuffer)
        .AddData(MakePodConstView(pcData)) );
}
//----------------------------------------------------------------------------
RHI::PFrameTask FImGuiService::RecreateBuffers_(const RHI::FCommandBufferBatch& cmd) {
    using namespace RHI;

    IFrameGraph& fg = *cmd->FrameGraph();
    const ImDrawData& drawData = *ImGui::GetDrawData();

    const size_t indexSize = drawData.TotalIdxCount * sizeof(ImDrawIdx);
    const size_t vertexSize = drawData.TotalVtxCount * sizeof(ImDrawVert);

    if (not _indexBuffer or indexSize > _indexBufferSize) {
        if (_indexBuffer)
            Unused(fg.ReleaseResource(_indexBuffer));

        _indexBufferSize = Meta::RoundToNext(indexSize, 2048);
        _indexBuffer = fg.CreateBuffer(FBufferDesc{
            _indexBufferSize, EBufferUsage::TransferDst | EBufferUsage::Index },
            Default ARGS_IF_RHIDEBUG("ImGui/IndexBuffer") );

        PPE_LOG_CHECK(UI, _indexBuffer);
    }

    if (not _vertexBuffer or vertexSize > _vertexBufferSize) {
        if (_vertexBuffer)
            Unused(fg.ReleaseResource(_vertexBuffer));

        _vertexBufferSize = Meta::RoundToNext(vertexSize, 2048);
        _vertexBuffer = fg.CreateBuffer(FBufferDesc{
            _vertexBufferSize, EBufferUsage::TransferDst | EBufferUsage::Vertex },
            Default ARGS_IF_RHIDEBUG("ImGui/VertexBuffer") );

        PPE_LOG_CHECK(UI, _vertexBuffer);
    }

    size_t indexOffset{0}, vertexOffset{0};

    PFrameTask lastTask;
    forrange(i, 0, drawData.CmdListsCount) {
        const ImDrawList& drawList = *drawData.CmdLists[i];

        lastTask = cmd->Task(FUpdateBuffer{}
            .SetBuffer(_indexBuffer)
            .AddData(drawList.IdxBuffer.Data, drawList.IdxBuffer.size_in_bytes(), indexOffset)
            .DependsOn(lastTask));

        lastTask = cmd->Task(FUpdateBuffer{}
            .SetBuffer(_vertexBuffer)
            .AddData(drawList.VtxBuffer.Data, drawList.VtxBuffer.size_in_bytes(), vertexOffset)
            .DependsOn(lastTask));

        indexOffset += drawList.IdxBuffer.size_in_bytes();
        vertexOffset += drawList.VtxBuffer.size_in_bytes();
    }

    Assert_NoAssume(indexOffset == indexSize);
    Assert_NoAssume(vertexOffset == vertexSize);
    return lastTask;
}
//----------------------------------------------------------------------------
RHI::PPipelineResources FImGuiService::FindOrAddTextureResources_(RHI::IFrameGraph& fg, const RHI::FImageID& texture) {
    Assert_NoAssume(texture.Valid());

    if (const auto it = _textureResources.Find(texture.Pack()); it != _textureResources.end()) {
        Assert_NoAssume(it->second);
        return it->second;
    }

    RHI::PPipelineResources resources = fg.CreatePipelineResources(*_pipeline, "0"_descriptorset);

    if (not resources) {
        PPE_LOG(UI, Error, "failed to initialize imgui pipeline resources");
        return Default;
    }

    resources->BindBuffer("uPushConstant"_uniform, _uniformBuffer);
    resources->BindTexture("sTexture"_uniform, texture, _textureSampler);

    _textureResources.Emplace_Overwrite(texture.Pack(), resources);
    return resources;
}
//----------------------------------------------------------------------------
void FImGuiService::GCUnusedTextureResources_(RHI::IFrameGraph& fg) {
    Unused(fg);

    // release resources when cache is the only referencer:
    for (u32 index = 0; index < _textureResources.size(); ) {
        auto [texture, resources] = _textureResources.Vector().at(index);
        if (resources->RefCount() > 1) {
            ++index;
            continue;
        }

        _textureResources.Erase(_textureResources.begin() + index);
    }
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
