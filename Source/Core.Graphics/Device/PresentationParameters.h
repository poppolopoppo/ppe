#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Maths/ScalarRectangle.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class SurfaceFormatType : u32;
//----------------------------------------------------------------------------
enum class PresentInterval {
    Immediate   = 0,
    One         = 1,
    Two         = 2,
    Three       = 3,
    Four        = 4,

    Default     = One
};
//----------------------------------------------------------------------------
class PresentationParameters {
public:
    PresentationParameters();
    PresentationParameters(
        u32 backBufferWidth,
        u32 backBufferHeight,
        SurfaceFormatType backBufferFormat,
        SurfaceFormatType depthStencilFormat,
        bool fullscreen,
        bool tripleBuffer,
        u32 multiSampleCount,
        PresentInterval presentationInterval
        );
    PresentationParameters(
        u32 backBufferWidth,
        u32 backBufferHeight,
        SurfaceFormatType backBufferFormat,
        SurfaceFormatType depthStencilFormat,
        bool fullscreen,
        bool tripleBuffer,
        u32 multiSampleCount,
        PresentInterval presentationInterval,
        const ViewportF& viewport
        );
    ~PresentationParameters();

    u32 BackBufferWidth() const { return _backBufferWidth; }
    u32 BackBufferHeight() const { return _backBufferHeight; }

    SurfaceFormatType BackBufferFormat() const { return _backBufferFormat; }
    SurfaceFormatType DepthStencilFormat() const { return _depthStencilFormat; }

    bool FullScreen() const;
    bool TripleBuffer() const;

    u32 MultiSampleCount() const;
    PresentInterval PresentationInterval() const;

    ViewportF Viewport() const { return _viewport; }

private:
    u32 _backBufferWidth;
    u32 _backBufferHeight;

    SurfaceFormatType _backBufferFormat;
    SurfaceFormatType _depthStencilFormat;

    u32 _data; // fullscreen + present + multisample

    ViewportF _viewport;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
