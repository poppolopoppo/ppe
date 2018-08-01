#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Time/Timeline.h"

#include "Core.Graphics/Window/BasicWindow.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceEncapsulator;
//----------------------------------------------------------------------------
FWD_REFPTR(GraphicsWindow);
class FGraphicsWindow : public FBasicWindow {
public:
    FGraphicsWindow(
        const wchar_t *title,
        int left, int top,
        size_t width, size_t height,
        FBasicWindow *parent = nullptr);
    virtual ~FGraphicsWindow();

    bool FixedTimeStep() const { return _fixedTimeStep; }
    void SetFixedTimeStep(bool value) { _fixedTimeStep = value; }

    void RenderLoop(FDeviceEncapsulator *deviceEncapsulator);

    virtual void Initialize(const FTimeline& time);
    virtual void Destroy();

    virtual void LoadContent();
    virtual void UnloadContent();

    virtual void Update(const FTimeline& time);
    virtual void Draw(const FTimeline& time);

    virtual void Present();

private:
    bool _fixedTimeStep;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
