// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Viewport/Viewport.h"

#include "Window/MainWindow.h"

#include "Maths/ScalarBoundingBoxHelpers.h"
#include "Maths/ScalarVectorHelpers.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FViewport::FViewport(const PMainWindow& window, const FRectangle2i& clientRect, EViewportFlags flags) NOEXCEPT
:   _data{FInternalData{
        .Window = window,
        .ClientRect = clientRect,
        .Flags = flags,
    }}
{}
//----------------------------------------------------------------------------
FRectangle2f FViewport::WindowClip() const NOEXCEPT {
    const auto sharedData = _data.LockShared();
    Assert(sharedData->Window);

    const int2 windowPos = sharedData->Window->Position();
    const FRectangle2i windowRect{ windowPos, windowPos + int2(sharedData->Window->Dimensions()) };
    Assert_NoAssume(windowRect.Contains(sharedData->ClientRect));

    // return viewport rect normalized in parent window space
    const float2 windowExtents{ windowRect.Extents() };
    return FRectangle2f{
        Saturate(float2(sharedData->ClientRect.Min()) / windowExtents),
        Saturate(float2(sharedData->ClientRect.Max()) / windowExtents),
    };
}
//----------------------------------------------------------------------------
int2 FViewport::ClientToScreen(const int2& clientPos) const NOEXCEPT {
    const auto sharedData = _data.LockShared();
    Assert(sharedData->Window);
    Assert_NoAssume(FRectangle2i{sharedData->ClientRect.Extents()}.Contains(clientPos));

    int2 screenPos{ clientPos + sharedData->ClientRect.Min() };
    sharedData->Window->ClientToScreen(&screenPos.x, &screenPos.y);

    return screenPos;
}
//----------------------------------------------------------------------------
int2 FViewport::ScreenToClient(const int2& screenPos) const NOEXCEPT {
    const auto sharedData = _data.LockShared();
    Assert(sharedData->Window);

    int2 clientPos{ screenPos };
    sharedData->Window->ScreenToClient(&clientPos.x, &clientPos.y);
    Assert_NoAssume(sharedData->ClientRect.Contains(clientPos));

    return (clientPos - sharedData->ClientRect.Min());
}
//----------------------------------------------------------------------------
float2 FViewport::ClientToTexCoord(const int2& clientPos) const NOEXCEPT {
    const auto sharedData = _data.LockShared();
    Assert_NoAssume(FRectangle2i{sharedData->ClientRect.Extents()}.Contains(clientPos));

    const float2 windowExtents{ float2(sharedData->ClientRect.Extents()) };
    float2 uv = ((float2(clientPos) + 0.5f) / windowExtents);
    uv.y = 1.f - uv.y;

    return Saturate(uv);
}
//----------------------------------------------------------------------------
int2 FViewport::TexCoordToClient(const float2& texCoord) const NOEXCEPT {
    Assert_NoAssume(Saturate(texCoord) == texCoord);
    const auto sharedData = _data.LockShared();

    return TruncToInt(float2(sharedData->ClientRect.Extents()) * texCoord);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
