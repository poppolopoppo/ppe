// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "UI/ImguiService.h"

#include "RHI/FrameGraph.h"
#include "RHI/SwapchainDesc.h"
#include "RHI/SamplerDesc.h"

#include "HAL/PlatformKeyboard.h"
#include "HAL/PlatformMouse.h"
#include "HAL/PlatformWindow.h"
#include "HAL/RHIService.h"

#include "Input/GamepadState.h"
#include "Input/InputService.h"
#include "Input/KeyboardState.h"
#include "Input/MouseState.h"

#include "Diagnostic/Logger.h"
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
NODISCARD bool CreateImguiPipeline_(RHI::FGPipelineID* pPipeline, RHI::PPipelineResources* pResources, RHI::IFrameGraph& fg) {
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
        LOG(UI, Error, L"failed to create imgui graphics pipeline");
        return false;
    }

    *pResources = fg.CreatePipelineResources(*pPipeline, "0"_descriptorset);
    if (not *pResources) {
        LOG(UI, Error, L"failed to initialize imgui pipeline resources");
        return false;
    }

    return true;
}
//----------------------------------------------------------------------------*
NODISCARD bool CreateImguiFontSampler_(RHI::FSamplerID* pSampler, RHI::IFrameGraph& fg) {
    RHI::FSamplerDesc desc;
    desc.SetFilter(RHI::ETextureFilter::Linear, RHI::ETextureFilter::Linear, RHI::EMipmapFilter::Linear);
    desc.SetAddressMode(RHI::EAddressMode::Repeat);
    desc.SetLodRange(-1000.f, 1000.f);

    *pSampler = fg.CreateSampler(desc ARGS_IF_RHIDEBUG("ImGui/FontSampler"));
    if (not *pSampler) {
        LOG(UI, Error, L"failed to create imgui font sampler");
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
    MAP_KEYBOARD(ImGuiKey_T, T);
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
static void PollImguiMouseEvents_(ImGuiIO& io, const IInputService& input) {
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
    if (window) {
        const FPlatformWindow& halWnd = *checked_cast<const FPlatformWindow*>(window);

        if (io.WantSetMousePos) {
            int screenX{ RoundToInt(io.MousePos.x) };
            int screenY{ RoundToInt(io.MousePos.y) };
            FPlatformMouse::ClientToScreen(halWnd, &screenX, &screenY);
            FPlatformMouse::SetCursorPosition(screenX, screenY);
        }

        if (io.WantCaptureMouse)
            FPlatformMouse::SetCapture(halWnd);
        else
            FPlatformMouse::ResetCapture();
    } else {
        FPlatformMouse::ResetCapture();
    }

    if (not (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) && window) {
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
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FImguiService::FImguiService(PImguiContext imguiContext) NOEXCEPT
:   _imguiContext(imguiContext)
,   _clearColor(0.45f, 0.55f, 0.60f, 1.00f) {
    AssertRelease(_imguiContext);
}
//----------------------------------------------------------------------------
FImguiService::~FImguiService() {
    ImGui::DestroyContext(_imguiContext);
}
//----------------------------------------------------------------------------
bool FImguiService::Construct(IInputService& input, IRHIService& rhi) {
    Assert(not _onInputUpdate);
    Assert(not _onRenderFrame);

    LOG(UI, Info, L"creating imgui service");

    const RHI::SFrameGraph fg = rhi.FrameGraph();

    // initialize options
    {
        ImGuiIO& io = ImGui::GetIO();

        io.BackendPlatformName = "PPE::Application::FImguiService";
        io.BackendPlatformUserData = this;

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

        // change default font
        ImFontConfig fontConfig{};
        //fontConfig.GlyphOffset.y = -2;
        const FFilename font = L"Data:/Fonts/FiraSansExtraCondensed-Regular.ttf";
        io.FontDefault = io.Fonts->AddFontFromFileTTF(*ToString(VFS_Unalias(font)), 16.0f, &fontConfig);
    }

    // initialize style
    {
        ImGui::StyleColorsClassic();
        ImGuiStyle& style = ImGui::GetStyle();

        style.FramePadding.y = 2;
        style.FrameRounding = 3;
        style.ItemSpacing.y = 3;
        style.WindowRounding = 4;

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.05f, 0.11f, 0.85f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.07f, 0.10f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.63f, 0.57f, 0.79f, 0.07f);
    }

    // initialize font atlas
    {
        unsigned char* textureData;
        int textureWidth, textureHeight;
        _imguiContext->IO.Fonts->GetTexDataAsRGBA32(&textureData, &textureWidth, &textureHeight);
    }

    LOG_CHECK(UI, CreateImguiPipeline_(&_pipeline, &_resources, *fg));
    LOG_CHECK(UI, CreateImguiFontSampler_(&_fontSampler, *fg));

    _onInputUpdate = input.OnInputUpdate().Bind<&FImguiService::OnUpdateInput>(this);
    _onWindowFocus = input.OnWindowFocus().Bind<&FImguiService::OnWindowFocus>(this);

    _onRenderFrame = rhi.OnRenderFrame().Bind<&FImguiService::OnRenderFrame>(this);
    _onWindowResized = rhi.OnWindowResized().Bind<&FImguiService::OnWindowResized>(this);

    return true;
}
//----------------------------------------------------------------------------
void FImguiService::TearDown(IInputService& input, IRHIService& rhi) {
    LOG(UI, Info, L"destroying imgui service");

    input.OnInputUpdate().Remove(_onInputUpdate);
    input.OnWindowFocus().Remove(_onWindowFocus);

    rhi.OnRenderFrame().Remove(_onRenderFrame);
    rhi.OnWindowResized().Remove(_onWindowResized);

    _resources.reset();

    rhi.FrameGraph()->ReleaseResources(
        _fontTexture,
        _fontSampler,
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
void FImguiService::OnUpdateInput(const IInputService& input, FTimespan dt) {
    ImGuiIO& io = ImGui::GetIO();
    LOG_CHECKVOID(UI, io.Fonts->IsBuilt());

    io.DeltaTime = static_cast<float>(*FSeconds{ dt });

    PollImguiGamepadEvents_(io, input);
    PollImguiKeyboardEvents_(io, input);
    PollImguiMouseEvents_(io, input);

    ImGui::NewFrame();
}
//----------------------------------------------------------------------------
void FImguiService::OnWindowFocus(const IInputService& input, const FGenericWindow* ) {
    ImGuiIO& io = ImGui::GetIO();

    io.AddFocusEvent(!!input.FocusedWindow());
}
//----------------------------------------------------------------------------
void FImguiService::OnRenderFrame(const IRHIService& rhi, FTimespan ) {
    using namespace RHI;

    ImGui::Render();

    ImDrawData* const pDrawData = ImGui::GetDrawData();
    if (not pDrawData || pDrawData->TotalVtxCount <= 0)
        return;

    const SFrameGraph fg = rhi.FrameGraph();

    FCommandBufferBatch cmd = fg->Begin(FCommandBufferDesc{EQueueType::Graphics}.SetName("ImGui"));
    LOG_CHECKVOID(UI, !!cmd);

    const FRawImageID swapchainImage = cmd->SwapchainImage(rhi.Swapchain());
    LOG_CHECKVOID(UI, !!swapchainImage);

    const float2 viewport{ pDrawData->DisplaySize.x, pDrawData->DisplaySize.y };

    FLogicalPassID renderPass = cmd->CreateRenderPass(FRenderPassDesc{uint2(viewport)}
        .AddViewport(viewport)
        .AddTarget(ERenderTargetID::Color0, swapchainImage, _clearColor, EAttachmentStoreOp::Store));

    PFrameTask tDrawUI = PrepareRenderCommand_(cmd, renderPass, {});
    LOG_CHECKVOID(UI, !!tDrawUI);

    LOG_CHECKVOID(UI, fg->Execute(cmd));
    LOG_CHECKVOID(UI, fg->Flush());
}
//----------------------------------------------------------------------------
void FImguiService::OnWindowResized(const IRHIService&, const FRHISurfaceCreateInfo& surface) {
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = {
        static_cast<float>(surface.Dimensions.x),
        static_cast<float>(surface.Dimensions.y) };
}
//----------------------------------------------------------------------------
RHI::PFrameTask FImguiService::PrepareRenderCommand_(
    RHI::FCommandBufferBatch& cmd,
    RHI::FLogicalPassID renderPass,
    TMemoryView<const RHI::PFrameTask> dependencies ) {
    LOG_CHECK(UI, cmd && _pipeline && _resources);

    using namespace RHI;

    ImDrawData* const pDrawData = ImGui::GetDrawData();
    LOG_CHECK(UI, pDrawData);
    Assert(pDrawData->TotalVtxCount > 0);

    FSubmitRenderPass submit{ renderPass };

    submit.DependsOn(CreateFontTexture_(cmd));
    submit.DependsOn(RecreateBuffers_(cmd));
    submit.DependsOn(UpdateUniformBuffer_(cmd));

    for (const auto& dep : dependencies)
        submit.DependsOn(dep);

    FVertexInputState vertexInput;
    vertexInput.Bind(FVertexBufferID{}, sizeof(ImDrawVert));
    vertexInput.Add("aPos"_vertex, EVertexFormat::Float2, Meta::StandardLayoutOffset(&ImDrawVert::pos));
    vertexInput.Add("aUV"_vertex, EVertexFormat::Float2, Meta::StandardLayoutOffset(&ImDrawVert::uv));
    vertexInput.Add("aColor"_vertex, EVertexFormat::UByte4_Norm, Meta::StandardLayoutOffset(&ImDrawVert::col));

    _resources->BindBuffer("uPushConstant"_uniform, _uniformBuffer);
    _resources->BindTexture("sTexture"_uniform, _fontTexture, _fontSampler);

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

                cmd->Task(renderPass, FDrawIndexed{}
                    .SetPipeline(_pipeline)
                    .AddResources("0"_descriptorset, _resources)
                    .SetTopology(EPrimitiveTopology::TriangleList)
                    .SetIndexBuffer(_indexBuffer, 0, IndexAttrib<ImDrawIdx>())
                    .AddVertexBuffer(""_vertexbuffer, _vertexBuffer)
                    .SetVertexInput(vertexInput)
                    .AddColorBuffer(ERenderTargetID::Color0, EBlendFactor::SrcAlpha, EBlendFactor::OneMinusSrcAlpha, EBlendOp::Add)
                    .SetEnableDepthTest(false)
                    .SetCullMode(ECullMode::None)
                    .AddScissor(FRectangleU(scissor))
                    .Draw(drawCmd.ElemCount, 1, indexOffset, checked_cast<i32>(vertexOffset), 0));

            } else {
                drawCmd.UserCallback(&drawList, &drawCmd);
            }

            indexOffset += drawCmd.ElemCount;
        }

        vertexOffset += drawList.VtxBuffer.Size;
    }

    return cmd->Task(submit);
}
//----------------------------------------------------------------------------
RHI::PFrameTask FImguiService::CreateFontTexture_(const RHI::FCommandBufferBatch& cmd) {
    using namespace RHI;

    if (_fontTexture)
        return nullptr;

    int2 textureDim;
    unsigned char* textureData;
    _imguiContext->IO.Fonts->GetTexDataAsRGBA32(&textureData, &textureDim.x, &textureDim.y);

    const size_t uploadSize = sizeof(char) * 4 * static_cast<size_t>(textureDim.x) * textureDim.y;

    _fontTexture = cmd->FrameGraph()->CreateImage(FImageDesc{}
        .SetDimension(checked_cast<u32>(textureDim))
        .SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Sampled | EImageUsage::TransferDst),
        Default ARGS_IF_RHIDEBUG("ImGui/FontTexture"));
    LOG_CHECK(UI, _fontTexture);

    return cmd->Task(FUpdateImage{}
        .SetImage(_fontTexture)
        .SetData(FRawMemoryConst{textureData, uploadSize}, checked_cast<u32>(textureDim)));
}
//----------------------------------------------------------------------------
RHI::PFrameTask FImguiService::UpdateUniformBuffer_(const RHI::FCommandBufferBatch& cmd) {
    using namespace RHI;

    const ImDrawData& drawData = *ImGui::GetDrawData();

    if (not _uniformBuffer) {
        _uniformBuffer = cmd->FrameGraph()->CreateBuffer(FBufferDesc{
            16_b, EBufferUsage::Uniform | EBufferUsage::TransferDst },
            Default ARGS_IF_RHIDEBUG("ImGui/UniformBuffer"));
        LOG_CHECK(UI, _uniformBuffer);
    }

    float4 pcData;
    pcData.xy = float2{
        2.0f / (drawData.DisplaySize.x * _imguiContext->IO.DisplayFramebufferScale.x),
        2.0f / (drawData.DisplaySize.y * _imguiContext->IO.DisplayFramebufferScale.y) };
    pcData.zw = float2{
        -1.f - (drawData.DisplayPos.x * pcData.x),
        -1.f - (drawData.DisplayPos.y * pcData.y) };

    return cmd->Task(FUpdateBuffer{}
        .SetBuffer(_uniformBuffer)
        .AddData(MakeRawConstView(pcData)) );
}
//----------------------------------------------------------------------------
RHI::PFrameTask FImguiService::RecreateBuffers_(const RHI::FCommandBufferBatch& cmd) {
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

        LOG_CHECK(UI, _indexBuffer);
    }

    if (not _vertexBuffer or vertexSize > _vertexBufferSize) {
        if (_vertexBuffer)
            Unused(fg.ReleaseResource(_vertexBuffer));

        _vertexBufferSize = Meta::RoundToNext(vertexSize, 2048);
        _vertexBuffer = fg.CreateBuffer(FBufferDesc{
            _vertexBufferSize, EBufferUsage::TransferDst | EBufferUsage::Vertex },
            Default ARGS_IF_RHIDEBUG("ImGui/VertexBuffer") );

        LOG_CHECK(UI, _vertexBuffer);
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
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
