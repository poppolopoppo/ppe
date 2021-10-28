#include "stdafx.h"

#include "RHI_fwd.h"

#if USE_PPE_RHIDEBUG

#include "RHIApi.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
NO_INLINE static void Test_ImageDesc1_() {
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension(2);

        AssertRelease(desc.Type == EImageDim_1D);
        AssertRelease(desc.View == Default);
        AssertRelease(desc.Dimensions == uint3(2,1,1));

        desc.Validate();

        AssertRelease(desc.Type == EImageDim_1D);
        AssertRelease(desc.View == EImageView_1D);
        AssertRelease(desc.Dimensions == uint3(2,1,1));
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 2, 3 });

        AssertRelease(desc.Type == EImageDim_2D);
        AssertRelease(desc.View == Default);
        AssertRelease(desc.Dimensions == uint3(2,3,1));

        desc.Validate();

        AssertRelease(desc.Type == EImageDim_2D);
        AssertRelease(desc.View == EImageView_2D);
        AssertRelease(desc.Dimensions == uint3(2,3,1));
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 2, 3, 4 });

        AssertRelease(desc.Type == EImageDim_3D);
        AssertRelease(desc.View == Default);
        AssertRelease(desc.Dimensions == uint3(2,3,4));

        desc.Validate();

        AssertRelease(desc.Type == EImageDim_3D);
        AssertRelease(desc.View == EImageView_3D);
        AssertRelease(desc.Dimensions == uint3(2,3,4));
    }
}
//----------------------------------------------------------------------------
NO_INLINE static void Test_ImageDesc2_() {
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetView(EImageView_1D);

        AssertRelease(desc.View == EImageView_1D);
        AssertRelease(desc.Type == EImageDim_1D);

        desc.Validate();

        AssertRelease(desc.View == EImageView_1D);
        AssertRelease(desc.Type == EImageDim_1D);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetView(EImageView_1DArray);

        AssertRelease(desc.View == EImageView_1DArray);
        AssertRelease(desc.Type == EImageDim_1D);

        desc.Validate();

        AssertRelease(desc.View == EImageView_1DArray);
        AssertRelease(desc.Type == EImageDim_1D);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetView(EImageView_2D);

        AssertRelease(desc.View == EImageView_2D);
        AssertRelease(desc.Type == EImageDim_2D);

        desc.Validate();

        AssertRelease(desc.View == EImageView_2D);
        AssertRelease(desc.Type == EImageDim_2D);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetView(EImageView_2DArray);

        AssertRelease(desc.View == EImageView_2DArray);
        AssertRelease(desc.Type == EImageDim_2D);

        desc.Validate();

        AssertRelease(desc.View == EImageView_2DArray);
        AssertRelease(desc.Type == EImageDim_2D);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetView(EImageView_3D);

        AssertRelease(desc.View == EImageView_3D);
        AssertRelease(desc.Type == EImageDim_3D);

        desc.Validate();

        AssertRelease(desc.View == EImageView_3D);
        AssertRelease(desc.Type == EImageDim_3D);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetView(EImageView_Cube);

        AssertRelease(desc.View == EImageView_Cube);
        AssertRelease(desc.Type == EImageDim_2D);

        desc.Validate();

        AssertRelease(desc.View == EImageView_Cube);
        AssertRelease(desc.Type == EImageDim_2D);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetView(EImageView_CubeArray);

        AssertRelease(desc.View == EImageView_CubeArray);
        AssertRelease(desc.Type == EImageDim_2D);

        desc.Validate();

        AssertRelease(desc.View == EImageView_CubeArray);
        AssertRelease(desc.Type == EImageDim_2D);
    }
}
//----------------------------------------------------------------------------
NO_INLINE static void Test_ImageDesc3_() {
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension(8);
        desc.SetArrayLayers(4);

        AssertRelease(desc.View == Default);
        AssertRelease(desc.Type == EImageDim_1D);
        AssertRelease(desc.ArrayLayers == 4_layer);
        AssertRelease(desc.MaxLevel == 1_mipmap);
        AssertRelease(not desc.Samples.Enabled());

        desc.Validate();

        AssertRelease(desc.View == EImageView_1DArray);
        AssertRelease(desc.Type == EImageDim_1D);
        AssertRelease(desc.ArrayLayers == 4_layer);
        AssertRelease(desc.MaxLevel == 1_mipmap);
        AssertRelease(not desc.Samples.Enabled());
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 8, 8 });
        desc.SetArrayLayers(4);

        AssertRelease(desc.View == Default);
        AssertRelease(desc.Type == EImageDim_2D);
        AssertRelease(desc.ArrayLayers == 4_layer);
        AssertRelease(desc.MaxLevel == 1_mipmap);
        AssertRelease(not desc.Samples.Enabled());

        desc.Validate();

        AssertRelease(desc.View == EImageView_2DArray);
        AssertRelease(desc.Type == EImageDim_2D);
        AssertRelease(desc.ArrayLayers == 4_layer);
        AssertRelease(desc.MaxLevel == 1_mipmap);
        AssertRelease(not desc.Samples.Enabled());
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 8, 8 });
        desc.SetArrayLayers(4);
        desc.SetMaxMipmaps(16);

        AssertRelease(desc.View == Default);
        AssertRelease(desc.Type == EImageDim_2D);
        AssertRelease(desc.ArrayLayers == 4_layer);
        AssertRelease(desc.MaxLevel == 16_mipmap);
        AssertRelease(not desc.Samples.Enabled());

        desc.Validate();

        AssertRelease(desc.View == EImageView_2DArray);
        AssertRelease(desc.Type == EImageDim_2D);
        AssertRelease(desc.ArrayLayers == 4_layer);
        AssertRelease(desc.MaxLevel == 4_mipmap);
        AssertRelease(not desc.Samples.Enabled());
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 8, 8 });
        desc.SetArrayLayers(4);
        desc.SetSamples(8);

        AssertRelease(desc.View == Default);
        AssertRelease(desc.Type == EImageDim_2D);
        AssertRelease(desc.ArrayLayers == 4_layer);
        AssertRelease(desc.MaxLevel == 1_mipmap);
        AssertRelease(desc.Samples == 8_samples);

        desc.Validate();

        AssertRelease(desc.View == EImageView_2DArray);
        AssertRelease(desc.Type == EImageDim_2D);
        AssertRelease(desc.ArrayLayers == 4_layer);
        AssertRelease(desc.MaxLevel == 1_mipmap);
        AssertRelease(desc.Samples == 8_samples);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 8, 8 });
        desc.SetArrayLayers(4);
        desc.SetMaxMipmaps(16);
        desc.SetSamples(4);

        AssertRelease(desc.View == Default);
        AssertRelease(desc.Type == EImageDim_2D);
        AssertRelease(desc.ArrayLayers == 4_layer);
        AssertRelease(desc.MaxLevel == 16_mipmap);
        AssertRelease(desc.Samples == 4_samples);

        desc.Validate();

        AssertRelease(desc.View == EImageView_2DArray);
        AssertRelease(desc.Type == EImageDim_2D);
        AssertRelease(desc.ArrayLayers == 4_layer);
        AssertRelease(desc.MaxLevel == 1_mipmap);
        AssertRelease(desc.Samples == 4_samples);
    }
}
//----------------------------------------------------------------------------
NO_INLINE static void Test_ImageView1_() {
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 32, 32 });
        desc.SetArrayLayers(6);

        AssertRelease(desc.View == Default);

        desc.Validate();

        AssertRelease(desc.View == EImageView_2DArray);

        FImageViewDesc view;

        AssertRelease(view.View == Default);

        view.Validate(desc);

        AssertRelease(view.View == EImageView_2DArray);
        AssertRelease(view.Format == EPixelFormat::RGBA8_UNorm);
        AssertRelease(view.BaseLayer == 0_layer);
        AssertRelease(view.LayerCount == 6);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 32, 32 });
        desc.SetArrayLayers(6);
        desc.SetFlag(EImageFlags::CubeCompatible);

        AssertRelease(desc.View == Default);

        desc.Validate();

        AssertRelease(desc.View == EImageView_Cube);

        FImageViewDesc view;
        view.SetArrayLayers(2, 2);

        AssertRelease(view.View == Default);

        view.Validate(desc);

        AssertRelease(view.View == EImageView_2DArray);
        AssertRelease(view.Format == EPixelFormat::RGBA8_UNorm);
        AssertRelease(view.BaseLayer == 2_layer);
        AssertRelease(view.LayerCount == 2);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 32, 32, 32 });
        desc.SetFlag(EImageFlags::Array2DCompatible);

        AssertRelease(desc.View == Default);

        desc.Validate();

        AssertRelease(desc.View == EImageView_3D);

        FImageViewDesc view;
        view.SetType(EImageView_2D);

        view.Validate(desc);

        AssertRelease(view.View == EImageView_2D);
        AssertRelease(view.Format == EPixelFormat::RGBA8_UNorm);
        AssertRelease(view.BaseLayer == 0_layer);
        AssertRelease(view.LayerCount == 1);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 32, 32, 32 });
        desc.SetFlag(EImageFlags::Array2DCompatible);

        AssertRelease(desc.View == Default);

        desc.Validate();

        AssertRelease(desc.View == EImageView_3D);

        FImageViewDesc view;
        view.SetType(EImageView_2DArray);

        view.Validate(desc);

        AssertRelease(view.View == EImageView_2DArray);
        AssertRelease(view.Format == EPixelFormat::RGBA8_UNorm);
        AssertRelease(view.BaseLayer == 0_layer);
        AssertRelease(view.LayerCount == 32);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.SetDimension({ 32, 32, 32 });
        desc.SetFlag(EImageFlags::Array2DCompatible);

        AssertRelease(desc.View == Default);

        desc.Validate();

        AssertRelease(desc.View == EImageView_3D);

        FImageViewDesc view;
        view.SetType(EImageView_2DArray);
        view.SetArrayLayers(3, 6);

        view.Validate(desc);

        AssertRelease(view.View == EImageView_2DArray);
        AssertRelease(view.Format == EPixelFormat::RGBA8_UNorm);
        AssertRelease(view.BaseLayer == 3_layer);
        AssertRelease(view.LayerCount == 6);
    }
}
//----------------------------------------------------------------------------
NO_INLINE static void Test_ImageView2_() {
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::RGBA8_UNorm;
        desc.Type = EImageDim_2D;
        desc.Validate();

        FImageViewDesc view;
        view.Validate(desc);

        AssertRelease(view.AspectMask == EImageAspect::Color);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::Depth32f;
        desc.Type = EImageDim_2D;
        desc.Validate();

        FImageViewDesc view;
        view.Validate(desc);

        AssertRelease(view.AspectMask == EImageAspect::Depth);
    }
    {
        FImageDesc desc;
        desc.Format = EPixelFormat::Depth32F_Stencil8;
        desc.Type = EImageDim_2D;
        desc.Validate();

        FImageViewDesc view;
        view.Validate(desc);

        AssertRelease(view.AspectMask == EImageAspect::DepthStencil);
    }
}
//----------------------------------------------------------------------------
void UnitTest_ImageDesc() {
    Test_ImageDesc1_();
    Test_ImageDesc2_();
    Test_ImageDesc3_();

    Test_ImageView1_();
    Test_ImageView2_();

    LOG(RHI, Info, L"UnitTest_ImageDesc [PASSED]");
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE

#endif //!USE_PPE_RHIDEBUG
