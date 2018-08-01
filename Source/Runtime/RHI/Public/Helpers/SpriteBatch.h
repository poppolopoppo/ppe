#pragma once

#include "Graphics.h"

#include "Device/DeviceAPI_fwd.h"

#include "Color/Color.h"
#include "Maths/Transform.h"
#include "Maths/ScalarRectangle_fwd.h"
#include "Maths/ScalarVector_fwd.h"
#include "Memory/UniquePtr.h"

#include <functional>

namespace PPE {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class ESpriteSortMode {
    Deferred = 0,
    Immediate,
    Texture,
    BackToFront,
    FrontToBack,

    Default = Deferred
};
//----------------------------------------------------------------------------
enum class ESpriteEffect {
    None = 0,
    FlipHorizontally = 1,
    FlipVertically = 2,
    FlipBoth = FlipHorizontally|FlipVertically,
};
//----------------------------------------------------------------------------
class FSpriteBatch {
public:
    explicit FSpriteBatch(IDeviceAPIContext* deviceContext);
    virtual ~FSpriteBatch();

    FSpriteBatch(const FSpriteBatch&) = delete;
    FSpriteBatch& operator =(const FSpriteBatch&) = delete;

    FSpriteBatch(FSpriteBatch&& rvalue);
    FSpriteBatch& operator =(FSpriteBatch&& rvalue);

    const FViewport& Viewport() const;
    void SetViewport(const FViewport& viewport);

    virtual void Begin(
        ESpriteSortMode sortMode = ESpriteSortMode::Default,
        const FBlendState* blendState = nullptr,
        const FSamplerState* samplerState = nullptr,
        const FDepthStencilState* depthStencilState = nullptr,
        const FRasterizerState* rasterizerState = nullptr,
        TFunction<void()> customContext = nullptr,
        const FTransform& transform = FTransform::Identity );
    virtual void End();

    void Draw(FTexture2D* texture, const float2& position, const FLinearColor& color = FLinearColor::PaperWhite);
    void Draw(FTexture2D* texture, const float2& position, const FRectangleF& source, const FLinearColor& color = FLinearColor::PaperWhite,
                float rotation = 0.f, const float2& origin = Constants::Float2_Zero, const float2& scale = Constants::Float2_One, ESpriteEffect effect = ESpriteEffect::None, float layerDepth = 0);

    void Draw(FTexture2D* texture, const FRectangleF& destination, const FLinearColor& color = FLinearColor::PaperWhite);
    void Draw(FTexture2D* texture, const FRectangleF& destination, const FRectangleF& source, const FLinearColor& color = FLinearColor::PaperWhite,
                float rotation = 0.f, const float2& origin = Constants::Float2_Zero, const float2& scale = Constants::Float2_Zero, ESpriteEffect effect = ESpriteEffect::None, float layerDepth = 0);

private:
    class FSpriteBatchImpl;
    TUniquePtr<FSpriteBatchImpl> _pimpl
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace PPE
