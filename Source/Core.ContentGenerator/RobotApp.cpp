#include "stdafx.h"

#include "RobotApp.h"

#include "Core/Color/Color.h"
#include "Core/Diagnostic/DialogBox.h"
#include "Core/Maths/Maths.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceEncapsulator.h"
#include "Core.Graphics/Device/Texture/SurfaceFormat.h"

#include "Core.Application/ApplicationConsole.h"

namespace Core {
namespace ContentGenerator {

extern void Test_Format();
extern void Test_Containers();
extern void Test_RTTI();

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
    Test_RTTI();
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
    const float t = float(Frac(totalSeconds*0.1));
    const float3 hsv(t, 1.0f, 0.5f);
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
