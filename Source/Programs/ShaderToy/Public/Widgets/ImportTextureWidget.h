#pragma once

#include "Texture_fwd.h"

#include "Texture/Texture.h"
#include "Texture/TextureGeneration.h"

#include "UI/Imgui.h"

#include "IO/Filename.h"
#include "IO/String.h"
#include "Misc/Event.h"
#include "Thread/Task/TaskHelpers.h"

namespace PPE {
namespace Application {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FImportTextureWidget {
public:
    explicit FImportTextureWidget(const ITextureService& textureService) NOEXCEPT;
    ~FImportTextureWidget();

    TPtrRef<const ITextureService> TextureService;

    FString Title{ ICON_FK_PICTURE_O " Import Texture Dialog" };

    FFilename CurrentFile;

    PFuture<ContentPipeline::PTexture> TextureGenerated;
    ContentPipeline::FTextureGeneration TextureGeneration;
    ContentPipeline::PTextureSource TextureSource;

    bool bVisible{ false };

    NODISCARD bool Show();

    using FTextureEvent = TFunction<void(const ContentPipeline::PTexture&)>;
    PUBLIC_EVENT(OnImportSuccess, FTextureEvent);

    using FErrorEvent = TFunction<void(FStringLiteral)>;
    PUBLIC_EVENT(OnImportFailure, FErrorEvent);

    void Import(const FFilename& filename, FTextureEvent&& onSuccess, FErrorEvent&& onFailure);
    void Reset();
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
