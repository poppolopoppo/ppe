#include "stdafx.h"

#include "Test_Includes.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Test_RayTracingDebugger1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IFrameGraph& fg = *app.RHI().FrameGraph();

    if (not (EnableShaderDebugging and fg.DeviceProperties().RayTracingNV)) {
        Unused(app);
        LOG(WindowTest, Warning, L"Test_RayTracingDebugger1_: skipped due to lack of debugger support (USE_PPE_RHIDEBUG={0})", USE_PPE_RHIDEBUG);
        return true;
    }

#if USE_PPE_RHIDEBUG
    FRayTracingPipelineDesc desc;
    desc.AddShader("Main"_rtshader, EShaderType::RayGen, EShaderLangFormat::VKSL_100 | EShaderLangFormat::EnableDebugTrace, "main", R"#(
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

	vec4 color = payload;
	imageStore( un_Output, ivec2(gl_LaunchIDNV), color );
}
)#"
ARGS_IF_RHIDEBUG("MainRayGenShader"));
    desc.AddShader("PrimaryMiss"_rtshader, EShaderType::RayMiss, EShaderLangFormat::VKSL_100 | EShaderLangFormat::EnableDebugTrace, "main", R"#(
#version 460 core
#extension GL_NV_ray_tracing : require
layout(location=0) rayPayloadInNV vec4  payload;

void main ()
{
	payload = vec4(0.0f);
}
)#"
ARGS_IF_RHIDEBUG("DefaultRayMissShader"));
    desc.AddShader("PrimaryHit"_rtshader, EShaderType::RayClosestHit, EShaderLangFormat::VKSL_100 | EShaderLangFormat::EnableDebugTrace, "main", R"#(
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
ARGS_IF_RHIDEBUG("RayClosestHit"));

    const TScopedResource<FRTPipelineID> ppln{ fg.ScopedResource(fg.CreatePipeline(desc ARGS_IF_RHIDEBUG("Test_RayTracingDebugger1"))) };
    LOG_CHECK(WindowTest, ppln.Valid());

    const uint2 viewSize{ 800, 600 };
    const uint2 debugCoord{ viewSize / 2_u32 };

    const TScopedResource<FImageID> dstImage{ fg.ScopedResource(fg.CreateImage(
        FImageDesc{}.SetDimension(viewSize).SetFormat(EPixelFormat::RGBA8_UNorm)
        .SetUsage(EImageUsage::Storage | EImageUsage::TransferSrc),
        Default ARGS_IF_RHIDEBUG("OutputImage"))) };
    LOG_CHECK(WindowTest, dstImage.Valid());

    const TMemoryView<const u32> indices{ 0, 1, 2 };
    const TMemoryView<const float3> vertices{
        { 0.25f, 0.25f, 0.0f },
        { 0.75f, 0.25f, 0.0f },
        { 0.50f, 0.75f, 0.0f } };

    FBuildRayTracingGeometry::FTriangles trianglesData;
    trianglesData.SetGeometryId("Triangle"_geometry)
        .SetIndexData(indices)
        .SetVertexData(vertices);

    FRayTracingGeometryDesc::FTriangles trianglesDesc;
    trianglesDesc.SetGeometryId("Triangle"_geometry)
        .SetIndices(checked_cast<u32>(indices.size()), EIndexFormat::UInt)
        .SetVertices<Meta::TDecay< decltype(vertices[0]) >>(checked_cast<u32>(vertices.size()))
        .SetFlag(ERayTracingGeometryFlags::Opaque);

    TScopedResource<FRTGeometryID> rtGeometry{ fg.ScopedResource(fg.CreateRayTracingGeometry(
        FRayTracingGeometryDesc({ trianglesDesc }),
        Default ARGS_IF_RHIDEBUG("RayTracingDebugger1_Geometry"))) };
    LOG_CHECK(WindowTest, rtGeometry.Valid());

    TScopedResource<FRTSceneID> rtScene{ fg.ScopedResource(fg.CreateRayTracingScene(
        FRayTracingSceneDesc(1),
        Default ARGS_IF_RHIDEBUG("RayTracingDebugger1_Scene"))) };
    LOG_CHECK(WindowTest, rtScene.Valid());

    TScopedResource<FRTShaderTableID> rtShaders{ fg.ScopedResource(
        fg.CreateRayTracingShaderTable(ARG0_IF_RHIDEBUG("RayTracingDebugger1_Shaders"))) };
    LOG_CHECK(WindowTest, rtShaders.Valid());

    PPipelineResources resources = NEW_REF(RHIPipeline, FPipelineResources);
    LOG_CHECK(WindowTest, fg.InitPipelineResources(resources.get(), ppln, "0"_descriptorset));

    bool dataIsCorrect = false;
    bool shaderOutputIsCorrect = false;

    const auto onShaderTraceReady = [&shaderOutputIsCorrect](FStringView taskName, FStringView shaderName, EShaderStages stages, TMemoryView<const FString> output) {
        shaderOutputIsCorrect = true;

        shaderOutputIsCorrect &= (stages == EShaderStages::AllRayTracing);
        shaderOutputIsCorrect &= (taskName == "DebuggableRayTracing");

        if (shaderName == "MainRayGenShader") {
            const FStringView ref0 = R"#(//> gl_LaunchIDNV: uint3 {400, 300, 0}
no source

//> uv: float2 {0.500626, 0.500835}
//  gl_LaunchIDNV: uint3 {400, 300, 0}
10. uv = vec2(gl_LaunchIDNV.xy) / vec2(gl_LaunchSizeNV.xy - 1);

//> origin: float3 {0.500626, 0.499165, -1.000000}
//  uv: float2 {0.500626, 0.500835}
11. origin = vec3(uv.x, 1.0f - uv.y, -1.0f);

//> traceNV(): void
//  origin: float3 {0.500626, 0.499165, -1.000000}
14. un_RtScene, /*rayFlags*/gl_RayFlagsNoneNV, /*cullMask*/0xFF,
15. 			 /*sbtRecordOffset*/0, /*sbtRecordStride*/1, /*missIndex*/0,
16. 			 /*origin*/origin, /*Tmin*/0.0f, /*direction*/direction, /*Tmax*/10.0f,
17. 			 /*payload*/0 );

//> color: float4 {0.249583, 0.252086, 0.498331, 1.000000}
19. color = payload;

//> imageStore(): void
//  gl_LaunchIDNV: uint3 {400, 300, 0}
//  color: float4 {0.249583, 0.252086, 0.498331, 1.000000}
20. 	imageStore( un_Output, ivec2(gl_LaunchIDNV), color );

)#";

            shaderOutputIsCorrect &= (output.size() == 1 and output[0] == ref0);
            LOG_CHECKVOID(WindowTest, shaderOutputIsCorrect);
        }
        else if (shaderName == "DefaultRayMissShader") {
            shaderOutputIsCorrect &= (output.empty());
            LOG_CHECKVOID(WindowTest, shaderOutputIsCorrect);
        }
        else if (shaderName == "RayClosestHit") {
            const FStringView ref1 = R"#(//> gl_LaunchIDNV: uint3 {400, 300, 0}
no source

//> barycentrics: float3 {0.249583, 0.252086, 0.498331}
9. barycentrics = vec3(1.0f - hitAttribs.x - hitAttribs.y, hitAttribs.x, hitAttribs.y);

//> payload: float4 {0.249583, 0.252086, 0.498331, 1.000000}
//  barycentrics: float3 {0.249583, 0.252086, 0.498331}
10. payload = vec4(barycentrics, 1.0);

)#";

            shaderOutputIsCorrect &= (output.size() == 1 and output[0] == ref1);
            LOG_CHECKVOID(WindowTest, shaderOutputIsCorrect);
        }
        else {
            shaderOutputIsCorrect = false;
            LOG_CHECKVOID(WindowTest, shaderOutputIsCorrect);
        }
    };

    fg.SetShaderDebugCallback(onShaderTraceReady);

    const auto onLoaded = [&dataIsCorrect, &debugCoord](const FImageView& imageData) {
        FRgba32f texel;
        imageData.Load(&texel, uint3(debugCoord, 0));

        const FRgba32f color{ 0.249583f, 0.252086f,  0.498331f, 1.0f };

        dataIsCorrect = DistanceSq(color, texel) < F_LargeEpsilon;
        LOG(WindowTest, Debug, L"Read({0}) -> {1} vs {2} == {3}", debugCoord, texel, color, dataIsCorrect);
        LOG_CHECKVOID(WindowTest, dataIsCorrect);
    };

    FCommandBufferBatch cmd{ fg.Begin(FCommandBufferDesc{}
        .SetName("Test_RayTracingDebugger1")
        .SetDebugFlags(EDebugFlags::Default)) };
    LOG_CHECK(WindowTest, !!cmd);

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

    PFrameTask tTrace = cmd->Task(FTraceRays{}
        .AddResources("0"_descriptorset, resources)
        .SetShaderTable(rtShaders)
        .SetGroupCount(viewSize.x, viewSize.y)
        .SetName("DebuggableRayTracing")
        .EnableShaderDebugTrace(uint3{ debugCoord, 0 })
        .DependsOn(tUpdateTable));

    PFrameTask tRead = cmd->Task(FReadImage{}
        .SetImage(dstImage, Default, viewSize)
        .SetCallback(onLoaded)
        .DependsOn(tTrace));
    Unused(tRead);

    LOG_CHECK(WindowTest, fg.Execute(cmd));
    LOG_CHECK(WindowTest, fg.WaitIdle());

    LOG_CHECK(WindowTest, dataIsCorrect);
    LOG_CHECK(WindowTest, shaderOutputIsCorrect);
#endif

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
