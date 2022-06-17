#include "stdafx.h"

#include "Test_Includes.h"
#include "Test_Uniforms.h"

namespace PPE {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
bool Compiler_MeshShader1_(FWindowTestApp& app) {
    using namespace PPE::RHI;

    IRHIService& rhi = app.RHI();

    FMeshPipelineDesc ppln;
    ppln.AddShader(EShaderType::MeshTask, EShaderLangFormat::VKSL_110, "main", R"#(
#version 450
#extension GL_NV_mesh_shader : enable

layout(local_size_x = 32) in;

layout(binding=0) writeonly uniform image2D uni_image;
layout(binding=1) uniform block0 {
    uint uni_value;
};
shared vec4 mem[10];

taskNV out Task {
    vec2 dummy;
    vec2 submesh[3];
} mytask;

void main()
{
    uint iid = gl_LocalInvocationID.x;
    uint gid = gl_WorkGroupID.x;

    for (uint i = 0; i < 10; ++i) {
        mem[i] = vec4(i + uni_value);
    }
    imageStore(uni_image, ivec2(iid), mem[gid]);
    imageStore(uni_image, ivec2(iid), mem[gid+1]);

    memoryBarrierShared();
    barrier();

    mytask.dummy	  = vec2(30.0, 31.0);
    mytask.submesh[0] = vec2(32.0, 33.0);
    mytask.submesh[1] = vec2(34.0, 35.0);
    mytask.submesh[2] = mytask.submesh[gid%2];

    memoryBarrierShared();
    barrier();

    gl_TaskCountNV = 3;
}
)#"
ARGS_IF_RHIDEBUG("Compiler_MeshShader1_MT"));

    ppln.AddShader(EShaderType::Mesh, EShaderLangFormat::VKSL_110, "main", R"#(
#version 450
#extension GL_NV_mesh_shader : enable

#define MAX_VER  81
#define MAX_PRIM 32

layout(local_size_x_id = 0) in;

layout(max_vertices=MAX_VER) out;
layout(max_primitives=MAX_PRIM) out;
layout(triangles) out;

taskNV in taskBlock {
    float gid1[2];
    vec4 gid2;
} mytask;

layout(binding=2, std430) buffer bufferBlock {
    float gid3[2];
    vec4 gid4;
} mybuf;

layout(location=0) out outBlock {
    float gid5;
    vec4 gid6;
} myblk[];

void main()
{
    uint iid = gl_LocalInvocationID.x;

    myblk[iid].gid5 = mytask.gid1[1] + mybuf.gid3[1];
    myblk[iid].gid6 = mytask.gid2 + mybuf.gid4;
}
)#"
ARGS_IF_RHIDEBUG("Compiler_MeshShader1_MS"));

    const SPipelineCompiler compiler = rhi.Compiler(EShaderLangFormat::VKSL_110);
    LOG_CHECK(WindowTest, !!compiler);

    LOG_CHECK(WindowTest, compiler->Compile(ppln, EShaderLangFormat::SPIRV_110));

    LOG_CHECK(WindowTest, ppln.Topology == EPrimitiveTopology::TriangleList);

    LOG_CHECK(WindowTest, ppln.MaxIndices == 32*3);
    LOG_CHECK(WindowTest, ppln.MaxVertices == 81);

    LOG_CHECK(WindowTest, ppln.DefaultTaskGroupSize == uint3{ 32,1,1 });
    LOG_CHECK(WindowTest, ppln.DefaultMeshGroupSize == uint3::One);

    LOG_CHECK(WindowTest, ppln.MeshSizeSpecialization == uint3{ 0,UMax,UMax });
    LOG_CHECK(WindowTest, ppln.TaskSizeSpecialization == uint3{ ~0u });

    return true;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace PPE
