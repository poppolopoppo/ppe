#pragma once

#include "Graphics.h"

#include "Core/ScalarRectangle.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class SurfaceFormat;
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
        const SurfaceFormat *backBufferFormat,
        const SurfaceFormat *depthStencilFormat,
        bool fullscreen,
        bool tripleBuffer,
        u32 multiSampleCount,
        PresentInterval presentationInterval
        );
    PresentationParameters(
        u32 backBufferWidth,
        u32 backBufferHeight,
        const SurfaceFormat *backBufferFormat,
        const SurfaceFormat *depthStencilFormat,
        bool fullscreen,
        bool tripleBuffer,
        u32 multiSampleCount,
        PresentInterval presentationInterval,
        const ViewportF& viewport
        );
    ~PresentationParameters();

    u32 BackBufferWidth() const { return _backBufferWidth; }
    u32 BackBufferHeight() const { return _backBufferHeight; }

    const SurfaceFormat *BackBufferFormat() const { return _backBufferFormat; }
    const SurfaceFormat *DepthStencilFormat() const { return _depthStencilFormat; }

    bool FullScreen() const;
    bool TripleBuffer() const;

    u32 MultiSampleCount() const;
    PresentInterval PresentationInterval() const;

    ViewportF Viewport() const { return _viewport; }

private:
    u32 _backBufferWidth;
    u32 _backBufferHeight;

    const SurfaceFormat *_backBufferFormat;
    const SurfaceFormat *_depthStencilFormat;

    u32 _data; // fullscreen + present + multisample

    ViewportF _viewport;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
