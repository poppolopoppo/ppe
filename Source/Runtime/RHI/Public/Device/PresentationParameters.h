#pragma once

#include "Graphics.h"

#include "Maths/ScalarRectangle.h"

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ESurfaceFormatType : u32;
//----------------------------------------------------------------------------
enum class EPresentInterval {
    Immediate   = 0,
    One         = 1,
    Two         = 2,
    Three       = 3,
    Four        = 4,

    Default     = One
};
//----------------------------------------------------------------------------
class FPresentationParameters {
public:
    FPresentationParameters();
    FPresentationParameters(
        u32 backBufferWidth,
        u32 backBufferHeight,
        ESurfaceFormatType backBufferFormat,
        ESurfaceFormatType depthStencilFormat,
        bool fullscreen,
        bool tripleBuffer,
        u32 multiSampleCount,
        EPresentInterval presentationInterval
        );
    FPresentationParameters(
        u32 backBufferWidth,
        u32 backBufferHeight,
        ESurfaceFormatType backBufferFormat,
        ESurfaceFormatType depthStencilFormat,
        bool fullscreen,
        bool tripleBuffer,
        u32 multiSampleCount,
        EPresentInterval presentationInterval,
        const FViewport& viewport
        );
    ~FPresentationParameters();

    u32 BackBufferWidth() const { return _backBufferWidth; }
    u32 BackBufferHeight() const { return _backBufferHeight; }

    ESurfaceFormatType BackBufferFormat() const { return _backBufferFormat; }
    ESurfaceFormatType DepthStencilFormat() const { return _depthStencilFormat; }

    bool FullScreen() const;
    bool TripleBuffer() const;

    u32 MultiSampleCount() const;
    EPresentInterval PresentationInterval() const;

    FViewport Viewport() const { return _viewport; }

private:
    u32 _backBufferWidth;
    u32 _backBufferHeight;

    ESurfaceFormatType _backBufferFormat;
    ESurfaceFormatType _depthStencilFormat;

    u32 _data; // fullscreen + present + multisample

    FViewport _viewport;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
