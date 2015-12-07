#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationWindow.h"
#include "Core.Graphics/Device/PresentationParameters.h"

namespace Core {
class Timeline;
namespace Graphics {
enum class DeviceAPI;
class DeviceEncapsulator;
}}

namespace Core {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class ApplicationGraphics : public ApplicationWindow
{
public:
    ApplicationGraphics(const wchar_t *appname,
                        int left, int top,
                        const Graphics::PresentationParameters& pp,
                        Graphics::DeviceAPI api,
                        bool fixedTimeStep = true );
    virtual ~ApplicationGraphics();

    bool FixedTimeStep() const { return _fixedTimeStep; }
    void SetFixedTimeStep(bool value) { _fixedTimeStep = value; }

    const Graphics::PresentationParameters& PresentationParameters() const { return _pp; }
    const Graphics::DeviceEncapsulator& DeviceEncapsulator() const { return *_deviceEncapsulator; }

    virtual void Start() override;
    virtual void Shutdown() override;

    void RenderLoop();

    virtual void LoadContent();
    virtual void UnloadContent();

    virtual void Update(const Timeline& time);
    virtual void Draw(const Timeline& time);

    virtual void Present();

private:
    Graphics::PresentationParameters _pp;
    UniquePtr<Graphics::DeviceEncapsulator> _deviceEncapsulator;
    Graphics::DeviceAPI _api;
    bool _fixedTimeStep;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
