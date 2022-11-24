// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_RayTracing1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FRayTracingPipelineDesc ppln;
    ppln.AddShader("main"_rtshader, EShaderType::RayGen, EShaderLangFormat::VKSL_110, "main", R"#(
#version 460
#extension GL_NV_ray_tracing : enable
layout(binding = 0, set = 0) uniform accelerationStructureNV accNV;
layout(location = 0) rayPayloadNV vec4 payload;

void main()
{
	uint lx = gl_LaunchIDNV.x;
	uint ly = gl_LaunchIDNV.y;
	uint sx = gl_LaunchSizeNV.x;
	uint sy = gl_LaunchSizeNV.y;
	traceNV(accNV, lx, ly, sx, sy, 0u, vec3(0.0f), 0.5f, vec3(1.0f), 0.75f, 1);
}
)#"
ARGS_IF_RHIDEBUG("Compiler_RayTracing1_RG"));

    ppln.AddShader("primaryAnyHit"_rtshader, EShaderType::RayAnyHit, EShaderLangFormat::VKSL_110, "main", R"#(
#version 460
#extension GL_NV_ray_tracing : enable
layout(location = 1) rayPayloadInNV vec4 incomingPayload;
void main()
{
	uvec3 v0 = gl_LaunchIDNV;
	uvec3 v1 = gl_LaunchSizeNV;
	int v2 = gl_PrimitiveID;
	int v3 = gl_InstanceID;
	int v4 = gl_InstanceCustomIndexNV;
	vec3 v5 = gl_WorldRayOriginNV;
	vec3 v6 = gl_WorldRayDirectionNV;
	vec3 v7 = gl_ObjectRayOriginNV;
	vec3 v8 = gl_ObjectRayDirectionNV;
	float v9 = gl_RayTminNV;
	float v10 = gl_RayTmaxNV;
	float v11 = gl_HitTNV;
	uint v12 = gl_HitKindNV;
	mat4x3 v13 = gl_ObjectToWorldNV;
	mat4x3 v14 = gl_WorldToObjectNV;
	incomingPayload = vec4(0.5f);
	if (v2 == 1)
		ignoreIntersectionNV();
	else
		terminateRayNV();
}
)#"
ARGS_IF_RHIDEBUG("Compiler_RayTracing1_RAH"));

	ppln.AddShader("callable"_rtshader, EShaderType::RayCallable, EShaderLangFormat::VKSL_110, "main", R"#(
#version 460
#extension GL_NV_ray_tracing : enable
layout(location = 0) callableDataNV vec4 data0;
layout(location = 1) callableDataInNV dataBlock {
	uint data1;
};
void main()
{
	uvec3 id = gl_LaunchIDNV;
	uvec3 size = gl_LaunchSizeNV;
	data1 = 256U;
	executeCallableNV(2,1);
}
)#"
ARGS_IF_RHIDEBUG("Compiler_RayTracing1_RC"));

	ppln.AddShader("primaryClosestHit"_rtshader, EShaderType::RayClosestHit, EShaderLangFormat::VKSL_110, "main", R"#(
#version 460
#extension GL_NV_ray_tracing : enable
layout(binding = 0, set = 0) uniform accelerationStructureNV accNV;
layout(location = 0) rayPayloadNV vec4 localPayload;
layout(location = 1) rayPayloadInNV vec4 incomingPayload;
void main()
{
	uvec3 v0 = gl_LaunchIDNV;
	uvec3 v1 = gl_LaunchSizeNV;
	int v2 = gl_PrimitiveID;
	int v3 = gl_InstanceID;
	int v4 = gl_InstanceCustomIndexNV;
	vec3 v5 = gl_WorldRayOriginNV;
	vec3 v6 = gl_WorldRayDirectionNV;
	vec3 v7 = gl_ObjectRayOriginNV;
	vec3 v8 = gl_ObjectRayDirectionNV;
	float v9 = gl_RayTminNV;
	float v10 = gl_RayTmaxNV;
	float v11 = gl_HitTNV;
	uint v12 = gl_HitKindNV;
	mat4x3 v13 = gl_ObjectToWorldNV;
	mat4x3 v14 = gl_WorldToObjectNV;
	traceNV(accNV, 0u, 1u, 2u, 3u, 0u, vec3(0.5f), 0.5f, vec3(1.0f), 0.75f, 1);
}
)#"
ARGS_IF_RHIDEBUG("Compiler_RayTracing1_RCH"));

	ppln.AddShader("primaryMiss"_rtshader, EShaderType::RayMiss, EShaderLangFormat::VKSL_110, "main", R"#(
#version 460
#extension GL_NV_ray_tracing : enable
layout(binding = 0, set = 0) uniform accelerationStructureNV accNV;
layout(location = 0) rayPayloadNV vec4 localPayload;
layout(location = 1) rayPayloadInNV vec4 incomingPayload;
void main()
{
	uvec3 v0 = gl_LaunchIDNV;
	uvec3 v1 = gl_LaunchSizeNV;
	vec3 v2 = gl_WorldRayOriginNV;
	vec3 v3 = gl_WorldRayDirectionNV;
	vec3 v4 = gl_ObjectRayOriginNV;
	vec3 v5 = gl_ObjectRayDirectionNV;
	float v6 = gl_RayTminNV;
	float v7 = gl_RayTmaxNV;
	traceNV(accNV, 0u, 1u, 2u, 3u, 0u, vec3(0.5f), 0.5f, vec3(1.0f), 0.75f, 1);
}
)#"
ARGS_IF_RHIDEBUG("Compiler_RayTracing1_RM"));

	ppln.AddShader("intersection"_rtshader, EShaderType::RayIntersection, EShaderLangFormat::VKSL_110, "main", R"#(
#version 460
#extension GL_NV_ray_tracing : enable
hitAttributeNV vec4 iAttr;
void main()
{
	uvec3 v0 = gl_LaunchIDNV;
	uvec3 v1 = gl_LaunchSizeNV;
	int v2 = gl_PrimitiveID;
	int v3 = gl_InstanceID;
	int v4 = gl_InstanceCustomIndexNV;
	vec3 v5 = gl_WorldRayOriginNV;
	vec3 v6 = gl_WorldRayDirectionNV;
	vec3 v7 = gl_ObjectRayOriginNV;
	vec3 v8 = gl_ObjectRayDirectionNV;
	float v9 = gl_RayTminNV;
	float v10 = gl_RayTmaxNV;
	mat4x3 v11 = gl_ObjectToWorldNV;
	mat4x3 v12 = gl_WorldToObjectNV;
	iAttr = vec4(0.5f,0.5f,0.0f,1.0f);
	reportIntersectionNV(0.5, 1U);
}
)#"
ARGS_IF_RHIDEBUG("Compiler_RayTracing1_RI"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::VKSL_110);
    LOG_CHECK(WindowTest, !!compiler);

    LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_110));

	const FDescriptorSet* const ds = ppln.DescriptorSet("0"_descriptorset);
	LOG_CHECK(WindowTest, !!ds);

    LOG_CHECK(WindowTest, TestRaytracingScene(*ds, "accNV"_uniform, 0, EShaderStages::RayGen | EShaderStages::RayClosestHit | EShaderStages::RayMiss));

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
