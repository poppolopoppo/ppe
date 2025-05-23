﻿// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Drawing_TraceRays2_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    if (not (app.RHI().Features() & ERHIFeature::RayTracing)) {
        PPE_LOG(WindowTest, Warning, "Drawing_TraceRays2_: skipped due to lack of ray tracing support");
        return true;
    }

    IFrameGraph& fg = *app.RHI().FrameGraph();

    FRayTracingPipelineDesc desc;
    desc.AddShader("Main"_rtshader, EShaderType::RayGen, EShaderLangFormat::VKSL_100, "main", R"#(
#version 460 core
#extension GL_NV_ray_tracing : require
layout(set=0, binding=0) uniform accelerationStructureNV  un_RtScene;
layout(set=0, binding=1, rgba8) writeonly uniform image2D  un_Output;
layout(location=0) rayPayloadNV vec4  payload;

void main ()
{
	const vec2 uv = vec2(gl_LaunchIDNV.xy) / vec2(gl_LaunchSizeNV.xy - 1);

	const vec3 origin = vec3(uv.x, 1.0f - uv.y, -1.0f);
	const vec3 direction = vec3(0.0f, 0.0f, 1.0f);

	traceNV( /*topLevel*/un_RtScene, /*rayFlags*/gl_RayFlagsNoneNV, /*cullMask*/0xFF,
			 /*sbtRecordOffset*/0, /*sbtRecordStride*/1, /*missIndex*/0,
			 /*origin*/origin, /*Tmin*/0.0f, /*direction*/direction, /*Tmax*/10.0f,
			 /*payload*/0 );

	imageStore( un_Output, ivec2(gl_LaunchIDNV), payload );
}
)#"
ARGS_IF_RHIDEBUG("Drawing_TraceRays2_RayGen"));
    desc.AddShader("PrimaryMiss"_rtshader, EShaderType::RayMiss, EShaderLangFormat::VKSL_100, "main", R"#(
#version 460 core
#extension GL_NV_ray_tracing : require
layout(location=0) rayPayloadInNV vec4  payload;

void main ()
{
	payload = vec4(0.0f);
}
)#"
ARGS_IF_RHIDEBUG("Drawing_TraceRays2_RayMiss"));
    desc.AddShader("PrimaryHit"_rtshader, EShaderType::RayClosestHit, EShaderLangFormat::VKSL_100, "main", R"#(
#version 460 core
#extension GL_NV_ray_tracing : require
layout(location=0) rayPayloadInNV vec4  payload;
				   hitAttributeNV vec2  hitAttribs;

void main ()
{
	const vec3 barycentrics = vec3(1.0f - hitAttribs.x - hitAttribs.y, hitAttribs.x, hitAttribs.y);
	payload = vec4(barycentrics, 1.0);
}
)#"
ARGS_IF_RHIDEBUG("Drawing_TraceRays2_RayClosestHit"));

    const TAutoResource<FRTPipelineID> ppln{ fg.ScopedResource(fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Drawing_TraceRays2"))) };
    PPE_LOG_CHECK(WindowTest, ppln.Valid());

    const u32 indices[] = { 0, 1, 2 };
    const float3 vertices[] = {
        { 0.25f, 0.25f, 0.0f },
        { 0.75f, 0.25f, 0.0f },
        { 0.50f, 0.75f, 0.0f } };

    FBuildRayTracingGeometry::FTriangles trianglesData;
    trianglesData.SetGeometryId("Triangle"_geometry)
        .SetIndexData(indices)
        .SetVertexData(vertices);

    FRayTracingGeometryDesc::FTriangles trianglesDesc;
    trianglesDesc.SetGeometryId("Triangle"_geometry)
        .SetIndices(lengthof(indices), EIndexFormat::UInt)
        .SetVertices<float3>(lengthof(vertices))
        .SetFlag(ERayTracingGeometryFlags::Opaque);

    TAutoResource<FRTGeometryID> rtGeometry{ fg.ScopedResource(fg.CreateRayTracingGeometry(
        FRayTracingGeometryDesc({ trianglesDesc }),
        Default ARGS_IF_RHIDEBUG("TraceRays2_Geometry"))) };
    PPE_LOG_CHECK(WindowTest, rtGeometry.Valid());

    TAutoResource<FRTSceneID> rtScene{ fg.ScopedResource(fg.CreateRayTracingScene(
        FRayTracingSceneDesc(1),
        Default ARGS_IF_RHIDEBUG("TraceRays2_Scene"))) };
    PPE_LOG_CHECK(WindowTest, rtScene.Valid());

    TAutoResource<FRTShaderTableID> rtShaders{ fg.ScopedResource(
        fg.CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG("TraceRays2_Shaders"))) };
    PPE_LOG_CHECK(WindowTest, rtShaders.Valid());

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    PPE_LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), ppln, "0"_descriptorset));

    bool dataIsCorrect = false;
    const auto onLoaded = [&dataIsCorrect](const FImageView& imageData) {
        const auto testTexel = [&imageData](float x, float y, const FRgba32f& color) -> bool {
            const u32 ix = FPlatformMaths::RoundToUnsigned((x + 1.0f) * 0.5f * float(imageData.Dimensions().x) + 0.5f);
            const u32 iy = FPlatformMaths::RoundToUnsigned((y + 1.0f) * 0.5f * float(imageData.Dimensions().y) + 0.5f);

            FRgba32f texel;
            imageData.Load(&texel, uint3(ix, iy, 0));

            const bool isEqual = DistanceSq(color, texel) < LargeEpsilon;
            PPE_LOG(WindowTest, Debug, "Read({0}) -> {1} vs {2} == {3}", float2(x, y), texel, color, isEqual);
            PPE_LOG_CHECK(WindowTest, isEqual);
            Assert(isEqual);
            return true;
        };

        dataIsCorrect = true;
        dataIsCorrect &= testTexel( 0.00f, -0.49f, FLinearColor{ 0.0f, 0.0f, 1.0f, 1.0f });
        dataIsCorrect &= testTexel( 0.49f,  0.49f, FLinearColor{ 0.0f, 1.0f, 0.0f, 1.0f });
        dataIsCorrect &= testTexel(-0.49f,  0.49f, FLinearColor{ 1.0f, 0.0f, 0.0f, 1.0f });
        dataIsCorrect &= testTexel( 0.00f, -0.51f, Zero);
        dataIsCorrect &= testTexel( 0.51f,  0.51f, Zero);
        dataIsCorrect &= testTexel(-0.51f,  0.51f, Zero);
        dataIsCorrect &= testTexel( 0.00f,  0.51f, Zero);
        dataIsCorrect &= testTexel( 0.51f, -0.51f, Zero);
        dataIsCorrect &= testTexel(-0.51f, -0.51f, Zero);
    };

    const uint2 viewSize{ 800, 600 };

    const TAutoResource<FImageID> dstImage{ fg.ScopedResource(fg.CreateImage(
        FImageDesc{}.SetDimension(viewSize).SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Storage | EImageUsage_BlitTransferSrc),
        Default ARGS_IF_RHIDEBUG("OutputImage"))) };
    PPE_LOG_CHECK(WindowTest, dstImage.Valid());

    // frame 1
    {
        FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
            .SetName("Drawing_TraceRays2-Frame1")
            .SetDebugFlags(EDebugFlags_Default)) };
        PPE_LOG_CHECK(WindowTest, !!cmd);

        resources->BindImage("un_Output"_uniform, dstImage);
        resources->BindRayTracingScene("un_RtScene"_uniform, rtScene);

        FBuildRayTracingScene::FInstance rtInstance;
        rtInstance.SetInstanceId("0"_instance);
        rtInstance.SetGeometryId(rtGeometry);

        PFrameTask tBuildGeom = cmd->Task(FBuildRayTracingGeometry{}.SetTarget(rtGeometry).Add(trianglesData));
        PFrameTask tBuildScene = cmd->Task(FBuildRayTracingScene{}.SetTarget(rtScene).Add(rtInstance).DependsOn(tBuildGeom));

        PFrameTask tUpdateTable = cmd->Task(FUpdateRayTracingShaderTable{}
            .SetTarget(rtShaders)
            .SetPipeline(ppln)
            .SetScene(rtScene)
            .SetRayGenShader("Main"_rtshader)
            .AddMissShader("PrimaryMiss"_rtshader, 0)
            .AddHitShader("0"_instance, "Triangle"_geometry, 0, "PrimaryHit"_rtshader)
            .DependsOn(tBuildScene));
        Unused(tUpdateTable);

        PPE_LOG_CHECK(WindowTest, fg.Execute(cmd));
        PPE_LOG_CHECK(WindowTest, fg.Flush());
    }

    // frame 2
    {
        FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
            .SetName("Drawing_TraceRays2-Frame2")
            .SetDebugFlags(EDebugFlags_Default)) };
        PPE_LOG_CHECK(WindowTest, !!cmd);

        PFrameTask tBuildGeom = cmd->Task(FBuildRayTracingGeometry{}.SetTarget(rtGeometry).Add(trianglesData));

        PFrameTask tTrace = cmd->Task(FTraceRays{}
            .AddResources("0"_descriptorset, resources)
            .SetShaderTable(rtShaders)
            .SetGroupCount(viewSize.x, viewSize.y)
            .DependsOn(tBuildGeom));
        Unused(tTrace);

        PPE_LOG_CHECK(WindowTest, fg.Execute(cmd));
        PPE_LOG_CHECK(WindowTest, fg.Flush());
    }

    // frame 3
    {
        FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
            .SetName("Drawing_TraceRays2-Frame3")
            .SetDebugFlags(EDebugFlags_Default)) };
        PPE_LOG_CHECK(WindowTest, !!cmd);

        PFrameTask tBuildGeom = cmd->Task(FBuildRayTracingGeometry{}.SetTarget(rtGeometry).Add(trianglesData));

        PFrameTask tTrace = cmd->Task(FTraceRays{}
            .AddResources("0"_descriptorset, resources)
            .SetShaderTable(rtShaders)
            .SetGroupCount(viewSize.x, viewSize.y)
            .DependsOn(tBuildGeom));

        PFrameTask tRead = cmd->Task(FReadImage{}
            .SetImage(dstImage, int2::Zero, viewSize)
            .SetCallback(onLoaded)
            .DependsOn(tTrace));
        Unused(tRead);

        PPE_LOG_CHECK(WindowTest, fg.Execute(cmd));
        PPE_LOG_CHECK(WindowTest, fg.Flush());
    }

    PPE_LOG_CHECK(WindowTest, fg.WaitIdle());

    PPE_LOG_CHECK(WindowTest, dataIsCorrect);

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------->
} //!namespace PPE
