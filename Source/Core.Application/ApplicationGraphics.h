#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationWindow.h"
#include "Core.Graphics/Device/PresentationParameters.h"

#include "Core/Time/Timepoint.h"

namespace Core {
class FTimeline;
namespace Graphics {
enum class EDeviceAPI;
class FDeviceEncapsulator;
}}

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FApplicationGraphics : public FApplicationWindow
{
public:
    FApplicationGraphics(
        const wchar_t *appname,
        int left, int top,
        const Graphics::FPresentationParameters& pp,
        Graphics::EDeviceAPI api,
        FTimespan tickRate = Timespan_60hz() );
    virtual ~FApplicationGraphics();

    // setting to 0 will make the app tick at maximum speed without timestepping
    const FTimespan& TickRate() const { return _tickRate; }
    void SetTickRate(const FTimespan& value) { _tickRate = value; }

    const Graphics::FPresentationParameters& PresentationParameters() const { return _pp; }

    Graphics::FDeviceEncapsulator& DeviceEncapsulator() { return *_deviceEncapsulator; }
    const Graphics::FDeviceEncapsulator& DeviceEncapsulator() const { return *_deviceEncapsulator; }

    virtual void Start() override;
    virtual void Shutdown() override;

    void RenderLoop();

    virtual void LoadContent();
    virtual void UnloadContent();

    virtual void Update(const FTimeline& time);
    virtual void Draw(const FTimeline& time);

    virtual void Present();

private:
    Graphics::FPresentationParameters _pp;
    TUniquePtr<Graphics::FDeviceEncapsulator> _deviceEncapsulator;
    Graphics::EDeviceAPI _api;
    FTimespan _tickRate;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
