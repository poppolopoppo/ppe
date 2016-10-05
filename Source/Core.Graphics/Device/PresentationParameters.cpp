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
struct FPresentationParametersData {
    typedef Meta::TBit<u32>::TFirst<1>::type bitfullscreen_type;
    typedef Meta::TBit<u32>::TAfter<bitfullscreen_type>::TField<1>::type bittriplebuffer_type;
    typedef Meta::TBit<u32>::TAfter<bittriplebuffer_type>::TField<3>::type bitpresentinterval_type;
    typedef Meta::TBit<u32>::TAfter<bitpresentinterval_type>::FRemain::type bitmultisample_type;
};
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FPresentationParameters::FPresentationParameters()
:   FPresentationParameters(0, 0, ESurfaceFormatType::UNKNOWN, ESurfaceFormatType::UNKNOWN, false, false, 0, EPresentInterval::Immediate) {}
//----------------------------------------------------------------------------
FPresentationParameters::FPresentationParameters(
    u32 backBufferWidth,
    u32 backBufferHeight,
    ESurfaceFormatType backBufferFormat,
    ESurfaceFormatType depthStencilFormat,
    bool fullscreen,
    bool tripleBuffer,
    u32 multiSampleCount,
    EPresentInterval presentationInterval )
:   FPresentationParameters( backBufferWidth, backBufferHeight, backBufferFormat, depthStencilFormat, fullscreen, tripleBuffer, multiSampleCount, presentationInterval,
                            ViewportF(0, 0, float(backBufferWidth), float(backBufferHeight), 0, 1)) {}
//----------------------------------------------------------------------------
FPresentationParameters::FPresentationParameters(
    u32 backBufferWidth,
    u32 backBufferHeight,
    ESurfaceFormatType backBufferFormat,
    ESurfaceFormatType depthStencilFormat,
    bool fullscreen,
    bool tripleBuffer,
    u32 multiSampleCount,
    EPresentInterval presentationInterval,
    const ViewportF& viewport)
:   _backBufferWidth(backBufferWidth)
,   _backBufferHeight(backBufferHeight)
,   _backBufferFormat(backBufferFormat)
,   _depthStencilFormat(depthStencilFormat)
,   _data(0)
,   _viewport(viewport) {
    FPresentationParametersData::bitfullscreen_type::InplaceSet(_data, fullscreen);
    FPresentationParametersData::bittriplebuffer_type::InplaceSet(_data, tripleBuffer);
    FPresentationParametersData::bitpresentinterval_type::InplaceSet(_data, static_cast<u32>(presentationInterval));
    FPresentationParametersData::bitmultisample_type::InplaceSet(_data, multiSampleCount);
}
//----------------------------------------------------------------------------
FPresentationParameters::~FPresentationParameters() {}
//----------------------------------------------------------------------------
bool FPresentationParameters::FullScreen() const {
    return FPresentationParametersData::bitfullscreen_type::Get(_data);
}
//----------------------------------------------------------------------------
bool FPresentationParameters::TripleBuffer() const {
    return FPresentationParametersData::bittriplebuffer_type::Get(_data);
}
//----------------------------------------------------------------------------
u32 FPresentationParameters::MultiSampleCount() const {
    return FPresentationParametersData::bitmultisample_type::Get(_data);
}
//----------------------------------------------------------------------------
EPresentInterval FPresentationParameters::PresentationInterval() const {
    return static_cast<EPresentInterval>(FPresentationParametersData::bitpresentinterval_type::Get(_data));
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
