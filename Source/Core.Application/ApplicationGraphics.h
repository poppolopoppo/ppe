#pragma once

#include "Core.Application/Application.h"

#include "Core.Application/ApplicationWindow.h"
#include "Core.Graphics/Device/PresentationParameters.h"

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
    FApplicationGraphics(const wchar_t *appname,
                        int left, int top,
                        const Graphics::FPresentationParameters& pp,
                        Graphics::EDeviceAPI api,
                        bool fixedTimeStep = true );
    virtual ~FApplicationGraphics();

    bool FixedTimeStep() const { return _fixedTimeStep; }
    void SetFixedTimeStep(bool value) { _fixedTimeStep = value; }

    const Graphics::FPresentationParameters& FPresentationParameters() const { return _pp; }
    const Graphics::FDeviceEncapsulator& FDeviceEncapsulator() const { return *_deviceEncapsulator; }

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
    bool _fixedTimeStep;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace Core
