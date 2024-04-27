// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Widgets/ImportTextureWidget.h"

#include "Texture/EnumToString.h"
#include "Texture/TextureEnums.h"
#include "Texture/TextureSource.h"
#include "TextureService.h"

#include "RHI/EnumToString.h"
#include "RHI/PixelFormatHelpers.h"
#include "RHI/RenderStateEnums.h"
#include "RHI/ResourceEnums.h"

#include "UI/ImGui.h"

#include "Container/Vector.h"
#include "Diagnostic/Logger.h"
#include "IO/FormatHelpers.h"
#include "IO/StringBuilder.h"

namespace PPE {
namespace Application {
LOG_CATEGORY(, ImportTexture);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FImportTextureWidget::FImportTextureWidget(const ITextureService& textureService) NOEXCEPT
:   TextureService(textureService)
{}
//----------------------------------------------------------------------------
FImportTextureWidget::~FImportTextureWidget() {
    Reset(); // need to wait explicitly for potential compression jobs in flight
}
//----------------------------------------------------------------------------
void FImportTextureWidget::Import(const FFilename& filename, FTextureEvent&& onSuccess, FErrorEvent&& onFailure) {
    Reset();

    bVisible = true;
    CurrentFile = filename;

    if (onSuccess)
        OnImportSuccess().FireAndForget(std::move(onSuccess));

    if (onFailure)
        OnImportFailure().FireAndForget(std::move(onFailure));
}
//----------------------------------------------------------------------------
bool FImportTextureWidget::Show() {
    if (!bVisible)
        return false;

    ImGui::PushID(this);
    DEFERRED{ ImGui::PopID(); };

     // Always center this window when appearing
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    DEFERRED{ ImGui::End(); };
    bVisible = ImGui::Begin(*Title, nullptr,
        ImGuiWindowFlags_Modal | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize);
    if (!bVisible)
        return false;

    ImGui::BeginDisabled(TextureGenerated && not TextureGenerated->Available());
    DEFERRED{ ImGui::EndDisabled(); };

    if (not TextureSource and not CurrentFile.empty()) {
        if (Meta::TOptional<ContentPipeline::FTextureSource> optionalSource{ TextureService->ImportTextureSource2D(CurrentFile) })
            TextureSource = NEW_REF(Texture, ContentPipeline::FTextureSource, std::move(*optionalSource));

        if (TextureSource) {
            TextureGeneration = ContentPipeline::FTextureGeneration{ *TextureService, TextureSource->Properties() };

        } else {
            _OnImportFailure.FireAndForget("failed to import texture source");

            Reset();
            ImGui::CloseCurrentPopup();
        }
    }

    const float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) / 2;

    FStringBuilder caption;
    caption << CurrentFile;

    ImGui::PushFont(ImGui::LargeFont());
    ImGui::TextUnformatted(caption.c_str());
    ImGui::PopFont();

    ImGui::Separator();

    if (TextureSource) {
        const u32 bpp = (ContentPipeline::ETextureSourceFormat_BytesPerPixel(TextureSource->Format()) * 8);

        if (ImGui::CollapsingHeader("Input", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::BeginDisabled();

            ContentPipeline::FTextureSourceProperties oldProperties = TextureSource->Properties();

            ImGui::SmallButton(INLINE_FORMAT(16, "{}", oldProperties.ImageView));
            ImGui::SameLine();
            ImGui::SmallButton(INLINE_FORMAT(16, "{}", oldProperties.Format));
            ImGui::SameLine();
            ImGui::SmallButton(INLINE_FORMAT(16, "{} bpp", bpp));
            ImGui::SameLine();
            ImGui::SmallButton(INLINE_FORMAT(16, "{}", oldProperties.Gamma));

            ImGui::InputScalarN("Dimensions", ImGuiDataType_U32, &oldProperties.Dimensions, 3);
            ImGui::InputScalar("Mips", ImGuiDataType_U32, &oldProperties.NumMips);
            ImGui::InputScalar("Layers", ImGuiDataType_U32, &oldProperties.NumSlices);

            ImGui::EndDisabled();

            ImGui::TextUnformatted(INLINE_FORMAT(32, "Input size: {:f3}",
                Fmt::SizeInBytes(TextureSource->Data().SizeInBytes())));
        }

        if (ImGui::CollapsingHeader("Format", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Color mask");

            bool bChangeColorMask = false;
            u32 colorMask = static_cast<u8>(TextureSource->ColorMask());

            ImGui::SameLine();
            bChangeColorMask |= ImGui::CheckboxFlags("R", &colorMask, static_cast<u8>(RHI::EColorMask::R));
            ImGui::SameLine();
            bChangeColorMask |= ImGui::CheckboxFlags("G", &colorMask, static_cast<u8>(RHI::EColorMask::G));
            ImGui::SameLine();
            bChangeColorMask |= ImGui::CheckboxFlags("B", &colorMask, static_cast<u8>(RHI::EColorMask::B));
            ImGui::SameLine();
            bChangeColorMask |= ImGui::CheckboxFlags("A", &colorMask, static_cast<u8>(RHI::EColorMask::A));

            caption.clear();
            caption << TextureGeneration.Quality;

            if (ImGui::BeginCombo("Quality", caption.c_str())) {
                for (ContentPipeline::ETextureCompressionQuality it : {
                    ContentPipeline::ETextureCompressionQuality::Low,
                    ContentPipeline::ETextureCompressionQuality::Medium,
                    ContentPipeline::ETextureCompressionQuality::High}) {
                    caption.clear();
                    caption << it;

                    bool bSelected = (TextureGeneration.Quality == it);
                    if (ImGui::Selectable(caption.c_str(), &bSelected))
                        TextureGeneration.Quality = it;

                    if (bSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            VECTORINSITU(Texture, ContentPipeline::UTextureCompression, 8) compressions;
            TextureService->TextureCompression(MakeAppendable(compressions), TextureGeneration.Prepare(TextureSource->Properties()), TextureGeneration);
            std::sort(compressions.begin(), compressions.end(), [](ContentPipeline::UTextureCompression& a, ContentPipeline::UTextureCompression& b) -> bool {
                return (EPixelFormat_BitsPerPixel(a->Format(), RHI::EImageAspect::Color) <
                        EPixelFormat_BitsPerPixel(b->Format(), RHI::EImageAspect::Color) );
            });

            if (not TextureGeneration.Compression && not compressions.empty())
                TextureGeneration.Compression = compressions.front();

            caption.clear();
            if (TextureGeneration.Compression)
                caption << TextureGeneration.Compression->Format();

            if (ImGui::BeginCombo("Format", caption.c_str())) {
                for (const ContentPipeline::UTextureCompression& it : compressions) {
                    caption.clear();
                    caption << it->Format();

                    bool bSelected = (TextureGeneration.Compression == it);
                    if (ImGui::Selectable(caption.c_str(), &bSelected))
                        TextureGeneration.Compression = it;

                    if (bSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        }

        if (ImGui::CollapsingHeader("Alpha", ImGuiTreeNodeFlags_DefaultOpen)) {
            u32 sourceFlags = static_cast<u8>(TextureGeneration.ResizeFlags.value_or(TextureSource->Flags()));
            ImGui::BeginDisabled(not TextureSource->HasAlpha());
            if (ImGui::CheckboxFlags("Use masked alpha", &sourceFlags, static_cast<u8>(ContentPipeline::ETextureSourceFlags::MaskedAlpha)))
                TextureGeneration.ResizeFlags = static_cast<ContentPipeline::ETextureSourceFlags>(sourceFlags);
            ImGui::EndDisabled();

            ImGui::BeginDisabled(TextureSource->ImageView() != RHI::EImageView_2D ||
                not (TextureGeneration.ResizeFlags.value_or(TextureSource->Flags()) & ContentPipeline::ETextureSourceFlags::MaskedAlpha));

            ImGui::Checkbox("Preserve alpha test coverage (2D)", &TextureGeneration.bPreserveAlphaTestCoverage2D);
            ImGui::Checkbox("Generate distance field from alpha (2D)", &TextureGeneration.bGenerateAlphaDistanceField2D);

            ImGui::BeginDisabled(not (TextureGeneration.bGenerateAlphaDistanceField2D || TextureGeneration.bPreserveAlphaTestCoverage2D));
            ImGui::SliderFloat("Alpha cutoff", &TextureGeneration.AlphaCutoff, 0, 1);
            ImGui::EndDisabled();

            ImGui::BeginDisabled(not TextureGeneration.bGenerateAlphaDistanceField2D);
            ImGui::SliderFloat("Alpha spread ratio", &TextureGeneration.AlphaSpreadRatio, 0, 1);
            ImGui::EndDisabled();

            ImGui::EndDisabled();
        }

        if (ImGui::CollapsingHeader("Mip-mapping", ImGuiTreeNodeFlags_DefaultOpen)) {
            caption.clear();
            caption << TextureGeneration.MipGeneration;

            if (ImGui::BeginCombo("Mip Generation", caption.c_str())) {
                for (ContentPipeline::ETextureMipGeneration it : {
                    ContentPipeline::ETextureMipGeneration::Default,
                    ContentPipeline::ETextureMipGeneration::Box,
                    ContentPipeline::ETextureMipGeneration::CubicSpine,
                    ContentPipeline::ETextureMipGeneration::CatmullRom,
                    ContentPipeline::ETextureMipGeneration::MitchellNetrevalli,
                    ContentPipeline::ETextureMipGeneration::PointSample,
                    ContentPipeline::ETextureMipGeneration::GaussianBlur3,
                    ContentPipeline::ETextureMipGeneration::GaussianBlur5,
                    ContentPipeline::ETextureMipGeneration::GaussianBlur7,
                    ContentPipeline::ETextureMipGeneration::GaussianBlur9,
                    ContentPipeline::ETextureMipGeneration::ContrastAdaptiveSharpen1,
                    ContentPipeline::ETextureMipGeneration::ContrastAdaptiveSharpen2,
                    ContentPipeline::ETextureMipGeneration::ContrastAdaptiveSharpen3,
                    ContentPipeline::ETextureMipGeneration::ContrastAdaptiveSharpen4,
                    ContentPipeline::ETextureMipGeneration::ContrastAdaptiveSharpen5,
                    ContentPipeline::ETextureMipGeneration::ContrastAdaptiveSharpen6,
                    ContentPipeline::ETextureMipGeneration::ContrastAdaptiveSharpen7,
                    ContentPipeline::ETextureMipGeneration::ContrastAdaptiveSharpen8,
                    ContentPipeline::ETextureMipGeneration::ContrastAdaptiveSharpen9,
                }) {
                    caption.clear();
                    caption << it;

                    bool bSelected = (TextureGeneration.MipGeneration == it);
                    if (ImGui::Selectable(caption.c_str(), &bSelected))
                        TextureGeneration.MipGeneration = it;

                    if (bSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::BeginDisabled(TextureSource->ImageView() != RHI::EImageView_2D);
            ImGui::Checkbox("Generate full mip-map chain (2D)", &TextureGeneration.bGenerateFullMipChain2D);
            ImGui::EndDisabled();

            ImGui::BeginDisabled(not TextureSource->HasAlpha());
            ImGui::Checkbox("Blend transparent pixels with mip-flooding", &TextureGeneration.bFloodMipChainWithAlpha);
            ImGui::EndDisabled();

            u32 sourceFlags = static_cast<u8>(TextureGeneration.ResizeFlags.value_or(TextureSource->Flags()));
            if (ImGui::CheckboxFlags("Preserve image borders", &sourceFlags, static_cast<u8>(ContentPipeline::ETextureSourceFlags::Tilable)))
                TextureGeneration.ResizeFlags = static_cast<ContentPipeline::ETextureSourceFlags>(sourceFlags);
        }

        if (TextureGeneration.Compression) {
            ContentPipeline::FTextureSourceProperties newProperties = TextureGeneration.Prepare(TextureSource->Properties());
            const RHI::FPixelFormatInfo pixelInfo = EPixelFormat_Infos(TextureGeneration.Compression->Format());

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Output", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::BeginDisabled();

                ImGui::SmallButton(INLINE_FORMAT(16, "{}", newProperties.ImageView));
                ImGui::SameLine();
                ImGui::SmallButton(INLINE_FORMAT(16, "{}", pixelInfo.Format));
                ImGui::SameLine();
                ImGui::SmallButton(INLINE_FORMAT(16, "{} bpp", pixelInfo.BitsPerPixel(RHI::EImageAspect::Color)));
                ImGui::SameLine();
                ImGui::SmallButton(INLINE_FORMAT(16, "{}x{}", pixelInfo.BlockDim.x, pixelInfo.BlockDim.y));

                if (pixelInfo.IsSRGB()) {
                    ImGui::SameLine();
                    ImGui::SmallButton("sRGB");
                }

                ImGui::InputScalarN("Dimensions", ImGuiDataType_U32, &newProperties.Dimensions, 3);
                ImGui::InputScalar("Mips", ImGuiDataType_U32, &newProperties.NumMips);
                ImGui::InputScalar("Layers", ImGuiDataType_U32, &newProperties.NumSlices);

                ImGui::EndDisabled();

                ImGui::TextUnformatted(INLINE_FORMAT(32, "Output size: {:f3}",
                    Fmt::SizeInBytes(pixelInfo.SizeInBytes(
                        RHI::EImageAspect::Color,
                        newProperties.Dimensions,
                        newProperties.NumMips,
                        newProperties.NumSlices)) ));
            }
        }
    }
    else {
        ImGui::TextUnformatted("Failed to import this file, is this a valid image format?");
    }

    ImGui::Separator();

    ImGui::BeginDisabled(not (TextureSource and TextureGeneration.Compression));
    if (ImGui::Button("OK", ImVec2(buttonWidth, 0))) {
        TextureGenerated = Future<ContentPipeline::PTexture>([this]() {
            return TextureGeneration.Generate(*TextureSource);
        });
        //ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();

    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();

    if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
        _OnImportFailure.FireAndForget("user canceled import");

        Reset();
        ImGui::CloseCurrentPopup();
    }

    if (TextureGenerated) {
        if (not TextureGenerated->Available()) {
            ImGui::SetCursorPos(float2(ImGui::GetWindowContentRegionMin() + ImGui::GetWindowContentRegionMax()) / 2.f - 40.f);
            ImGui::PushFont(ImGui::LargeFont());
            ImGui::Spinner("Compressing texture...", 40, 8, ImGui::GetColorU32(ImGuiCol_TitleBgActive));
            ImGui::PopFont();
        } else {
            if (const ContentPipeline::PTexture generated = TextureGenerated->Result())
                _OnImportSuccess.FireAndForget(generated);
            else
                _OnImportFailure.FireAndForget("failed to compress the texture");

            Reset();
            ImGui::CloseCurrentPopup();
        }
    }

    return true;
}
//----------------------------------------------------------------------------
void FImportTextureWidget::Reset() {
    if (TextureGenerated) { // make sure to wait pending task before exit
        if (const ContentPipeline::PTexture generated = TextureGenerated->Result())
            _OnImportSuccess.FireAndForget(generated);
        else
            _OnImportFailure.FireAndForget("failed to compress the texture");
    }

    bVisible = false;
    CurrentFile = Default;
    TextureSource = Default;
    TextureGeneration = Default;
    TextureGenerated = Default;

    _OnImportSuccess.Clear();
    _OnImportFailure.Clear();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Application
} //!namespace PPE
