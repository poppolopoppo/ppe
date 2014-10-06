#pragma once

#include "Graphics.h"

#include "Core/Timeline.h"

#include "BasicWindow.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceEncapsulator;
//----------------------------------------------------------------------------
FWD_REFPTR(GraphicsWindow);
class GraphicsWindow : public BasicWindow {
public:
    GraphicsWindow(
        const wchar_t *title,
        int left, int top,
        size_t width, size_t height,
        BasicWindow *parent = nullptr);
    virtual ~GraphicsWindow();

    void RenderLoop(DeviceEncapsulator *deviceEncapsulator);

    virtual void Initialize(const Timeline& time);
    virtual void Destroy();

    virtual void LoadContent();
    virtual void UnloadContent();

    virtual void Update(const Timeline& time);
    virtual void Draw(const Timeline& time);

    virtual void Present();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
