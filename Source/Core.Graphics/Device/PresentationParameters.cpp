#include "stdafx.h"

#include "PresentationParameters.h"

#include "Texture/SurfaceFormat.h"

#include "Core/Meta/BitField.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
struct PresentationParametersData {
    typedef Meta::Bit<u32>::First<1>::type bitfullscreen_type;
    typedef Meta::Bit<u32>::After<bitfullscreen_type>::Field<1>::type bittriplebuffer_type;
    typedef Meta::Bit<u32>::After<bittriplebuffer_type>::Field<3>::type bitpresentinterval_type;
    typedef Meta::Bit<u32>::After<bitpresentinterval_type>::Remain::type bitmultisample_type;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
PresentationParameters::PresentationParameters()
:   PresentationParameters(0, 0, nullptr, nullptr, false, false, 0, PresentInterval::Default) {}
//----------------------------------------------------------------------------
PresentationParameters::PresentationParameters(
    u32 backBufferWidth,
    u32 backBufferHeight,
    const SurfaceFormat *backBufferFormat,
    const SurfaceFormat *depthStencilFormat,
    bool fullscreen,
    bool tripleBuffer,
    u32 multiSampleCount,
    PresentInterval presentationInterval )
:   PresentationParameters( backBufferWidth, backBufferHeight, backBufferFormat, depthStencilFormat, fullscreen, tripleBuffer, multiSampleCount, presentationInterval,
                            ViewportF(0, 0, float(backBufferWidth), float(backBufferHeight), 0, 1)) {}
//----------------------------------------------------------------------------
PresentationParameters::PresentationParameters(
    u32 backBufferWidth,
    u32 backBufferHeight,
    const SurfaceFormat *backBufferFormat,
    const SurfaceFormat *depthStencilFormat,
    bool fullscreen,
    bool tripleBuffer,
    u32 multiSampleCount,
    PresentInterval presentationInterval,
    const ViewportF& viewport)
:   _backBufferWidth(backBufferWidth)
,   _backBufferHeight(backBufferHeight)
,   _backBufferFormat(backBufferFormat)
,   _depthStencilFormat(depthStencilFormat)
,   _data(0)
,   _viewport(viewport) {
    AssertRelease(backBufferFormat);
    AssertRelease(  backBufferFormat->IsRGB() &&
                    backBufferFormat->SupportRenderTarget());

    AssertRelease(depthStencilFormat);
    AssertRelease(  depthStencilFormat->IsDepth() &&
                    depthStencilFormat->SupportRenderTarget());

    PresentationParametersData::bitfullscreen_type::InplaceSet(_data, fullscreen);
    PresentationParametersData::bittriplebuffer_type::InplaceSet(_data, tripleBuffer);
    PresentationParametersData::bitpresentinterval_type::InplaceSet(_data, static_cast<u32>(presentationInterval));
    PresentationParametersData::bitmultisample_type::InplaceSet(_data, multiSampleCount);
}
//----------------------------------------------------------------------------
PresentationParameters::~PresentationParameters() {}
//----------------------------------------------------------------------------
bool PresentationParameters::FullScreen() const {
    return PresentationParametersData::bitfullscreen_type::Get(_data);
}
//----------------------------------------------------------------------------
bool PresentationParameters::TripleBuffer() const {
    return PresentationParametersData::bittriplebuffer_type::Get(_data);
}
//----------------------------------------------------------------------------
u32 PresentationParameters::MultiSampleCount() const {
    return PresentationParametersData::bitmultisample_type::Get(_data);
}
//----------------------------------------------------------------------------
PresentInterval PresentationParameters::PresentationInterval() const {
    return static_cast<PresentInterval>(PresentationParametersData::bitpresentinterval_type::Get(_data));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
