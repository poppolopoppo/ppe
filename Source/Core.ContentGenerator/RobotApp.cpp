#include "stdafx.h"

#include "RobotApp.h"

#include "Core/Color/Color.h"
#include "Core/Diagnostic/DialogBox.h"
#include "Core/Maths/Maths.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core.Application/ApplicationConsole.h"
#include "Core.Application/Input/GamepadInputHandler.h"

namespace Core {
namespace ContentGenerator {

extern void Test_Format();
extern void Test_Containers();
extern void Test_Pixmap();
extern void Test_RTTI();
extern void Test_Thread();
extern void Test_XML();

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
RobotApp::RobotApp()
: parent_type(
    L"RobotApp", 100, 100,
    Graphics::PresentationParameters(
        640, 480,
        Graphics::SurfaceFormatType::B8G8R8A8_SRGB,
        Graphics::SurfaceFormatType::D24S8,
        false,
        false,
        0,
        Graphics::PresentInterval::Default ),
    Graphics::DeviceAPI::DirectX11,
    true ) {

    Application::ApplicationConsole::RedirectIOToConsole();

    Test_Format();
    Test_Containers();
    Test_Pixmap();
    Test_RTTI();
    Test_Thread();
    Test_XML();
}
//----------------------------------------------------------------------------
void RobotApp::Start() {
    parent_type::Start();

    RenderLoop();
}
//----------------------------------------------------------------------------
void RobotApp::Draw(const Timeline& time) {
    parent_type::Draw(time);

    const double totalSeconds = Units::Time::Seconds(time.Total()).Value();
    float3 hsv(float(Frac(totalSeconds*0.1)), 1.0f, 0.5f);

    const auto* gamepadService = Services().Get<Application::IGamepadService>();
    const Application::GamepadState& gamepad = gamepadService->State().First();

    if (gamepad.IsConnected()) {
        static float3 p(0.5f);

        if (gamepad.RightTrigger()>0.5f) {
            p.x() += float(gamepad.LeftStickX()*time.Elapsed().Value()*0.0005f);
            p.y() += float(gamepad.LeftStickY()*time.Elapsed().Value()*0.0005f);
        }
        else {
            p.x() = hsv.x();
            p.y() = hsv.y();
        }

        p.z() += float(gamepad.RightStickY()*time.Elapsed().Value()*0.0005f);

        p = Saturate(p);

        hsv = p;
    }

    const float3 rgb = HSV_to_RGB(hsv);
    const ColorRGBAF clearColor(rgb, 1.0f);

    Graphics::IDeviceAPIEncapsulator* const device = DeviceEncapsulator().Device();
    device->Clear(device->BackBufferRenderTarget(), clearColor);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace ContentGenerator
} //!namespace Core
