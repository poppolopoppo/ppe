
#include "stdafx.h"

#include "Vulkan/RayTracing/VulkanRayTracingGeometry.h"

#include "Vulkan/Common/VulkanEnums.h"
#include "Vulkan/Instance/VulkanDevice.h"
#include "Vulkan/Instance/VulkanResourceManager.h"
#include "Vulkan/Memory/VulkanMemoryObject.h"

#include "Diagnostic/Logger.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
static void CopyAndSortGeometry_(
    VECTOR(RHIRayTracing, FVulkanRayTracingGeometry::FTriangles)* outTriangles,
    VECTOR(RHIRayTracing, FVulkanRayTracingGeometry::FAabbs)* outAabbs,
    const FRayTracingGeometryDesc& desc ) {
    Assert(outTriangles);
    Assert(outAabbs);

    outTriangles->reserve(desc.Triangles.size());
    outTriangles->assign(desc.Triangles.Map(
        [](const auto& src) {
            FVulkanRayTracingGeometry::FTriangles dst;

            Assert(src.VertexCount > 0);
            Assert(src.GeometryId.Valid());

            dst.GeometryId = src.GeometryId;
            dst.VertexSize = checked_cast<u16>(EVertexFormat_SizeOf(src.VertexFormat));
            dst.IndexSize = checked_cast<u16>(EIndexFormat_SizeOf(src.IndexFormat));
            dst.MaxVertexCount = src.VertexCount;
            dst.MaxIndexCount = src.IndexCount;
            dst.VertexFormat = src.VertexFormat;
            dst.IndexFormat = src.IndexFormat;
            dst.Flags = src.Flags;

            return dst;
        }));

    outAabbs->reserve(desc.Aabbs.size());
    outAabbs->assign(desc.Aabbs.Map(
        [](const auto& src) {
            FVulkanRayTracingGeometry::FAabbs dst;

            Assert(src.AabbCount > 0);
            Assert(src.GeometryId.Valid());

            dst.GeometryId = src.GeometryId;
            dst.MaxAabbCount = src.AabbCount;
            dst.Flags = src.Flags;

            return dst;
        }));

    std::sort(outTriangles->begin(), outTriangles->end());
    std::sort(outAabbs->begin(), outAabbs->end());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
#if USE_PPE_RHIDEBUG
FVulkanRayTracingGeometry::~FVulkanRayTracingGeometry() {
    Assert_NoAssume(VK_NULL_HANDLE == _data.LockExclusive()->BottomLevelAS);
}
#endif
//----------------------------------------------------------------------------
// from specs: "Acceleration structure creation uses the count and type information from the geometries"
bool FVulkanRayTracingGeometry::Construct(
    FVulkanResourceManager& resources,
    const FRayTracingGeometryDesc& desc,
    FRawMemoryID memoryId,
    FVulkanMemoryObject& memoryObj
    ARGS_IF_RHIDEBUG(FConstChar debugName) ) {
    const auto exclusiveData = _data.LockExclusive();
    Assert(memoryId.Valid());
    Assert(VK_NULL_HANDLE == exclusiveData->BottomLevelAS);
    Assert(not exclusiveData->MemoryId.Valid());
    Assert(not desc.Triangles.empty() || not desc.Aabbs.empty());

    const FVulkanDevice& device = resources.Device();
    AssertRelease((desc.Triangles.size() + desc.Aabbs.size()) <= device.Capabilities().RayTracingPropertiesNV.maxGeometryCount);

    CopyAndSortGeometry_(&exclusiveData->Triangles, &exclusiveData->Aabbs, desc);

    VECTOR(RHIRayTracing, VkGeometryNV) geometries;
    geometries.resize(exclusiveData->Triangles.size() + exclusiveData->Aabbs.size());

    forrange(i, 0, exclusiveData->Triangles.size()) {
        const auto& src = exclusiveData->Triangles[i];
        auto& dst = geometries[i];

        dst = {};
        dst.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
        dst.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
        dst.flags = VkCast(src.Flags);
        dst.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
        dst.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
        dst.geometry.triangles.vertexCount = src.MaxVertexCount;
        dst.geometry.triangles.vertexFormat = VkCast(src.VertexFormat);

        if (src.MaxIndexCount > 0) {
            dst.geometry.triangles.indexCount = src.MaxIndexCount;
            dst.geometry.triangles.indexType = VkCast(src.IndexFormat);
        }
        else {
            Assert(src.IndexFormat == EIndexFormat::Unknown);
            dst.geometry.triangles.indexType = VK_INDEX_TYPE_NONE_NV;
        }
    }

    forrange(i, 0, exclusiveData->Aabbs.size()) {
        const auto& src = exclusiveData->Aabbs[i];
        auto& dst = geometries[i + exclusiveData->Triangles.size()];

        dst = {};
        dst.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
        dst.geometryType = VK_GEOMETRY_TYPE_AABBS_NV;
        dst.flags = VkCast(src.Flags);
        dst.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
        dst.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
        dst.geometry.aabbs.numAABBs = src.MaxAabbCount;
    }

    VkAccelerationStructureCreateInfoNV create{};
    create.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
    create.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
    create.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
    create.info.geometryCount = checked_cast<u32>(geometries.size());
    create.info.pGeometries = geometries.data();
    create.info.flags = VkCast(desc.Flags);

    VK_CHECK( device.vkCreateAccelerationStructureNV(
        device.vkDevice(),
        &create, device.vkAllocator(),
        &exclusiveData->BottomLevelAS ) );

    LOG_CHECK(RHI, memoryObj.AllocateAccelStruct(
        resources.MemoryManager(),
        exclusiveData->BottomLevelAS ) );

    VK_CHECK( device.vkGetAccelerationStructureHandleNV(
        device.vkDevice(),
        exclusiveData->BottomLevelAS,
        sizeof(exclusiveData->BLAS.Value),
        &exclusiveData->BLAS.Value ));

    exclusiveData->MemoryId = FMemoryID{ memoryId };
    exclusiveData->Flags = desc.Flags;

#if USE_PPE_RHIDEBUG
    if (debugName) {
        _debugName = debugName;
        device.SetObjectName(exclusiveData->BottomLevelAS, _debugName, VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV);
    }
#endif

    return true;
}
//----------------------------------------------------------------------------
void FVulkanRayTracingGeometry::TearDown(FVulkanResourceManager& resources) {
    const auto exclusiveData = _data.LockExclusive();

    if (exclusiveData->BottomLevelAS != VK_NULL_HANDLE) {
        const FVulkanDevice& device = resources.Device();
        device.vkDestroyAccelerationStructureNV(
            device.vkDevice(),
            exclusiveData->BottomLevelAS,
            device.vkAllocator() );
    }

    if (exclusiveData->MemoryId.Valid()) {
        resources.ReleaseResource(exclusiveData->MemoryId.Release());
    }

    exclusiveData->BLAS = Zero;
    exclusiveData->BottomLevelAS = VK_NULL_HANDLE;
    exclusiveData->Flags = Default;

    exclusiveData->Triangles.clear_ReleaseMemory();
    exclusiveData->Aabbs.clear_ReleaseMemory();

    ONLY_IF_RHIDEBUG(_debugName.Clear());
}
//----------------------------------------------------------------------------
size_t FVulkanRayTracingGeometry::GeometryIndex(const FGeometryID& geometryId) const {
    Assert(geometryId.Valid());

    const auto sharedData = _data.LockShared();

    const auto it = std::lower_bound(
        sharedData->Triangles.begin(),
        sharedData->Triangles.end(),
        geometryId,
        Meta::TLess{} );
    if (sharedData->Triangles.end() != it && it->GeometryId == geometryId)
        return std::distance(sharedData->Triangles.begin(), it);

    const auto jt = std::lower_bound(
        sharedData->Aabbs.begin(),
        sharedData->Aabbs.end(),
        geometryId,
        Meta::TLess{} );
    if (sharedData->Aabbs.end() != jt && jt->GeometryId == geometryId)
        return (sharedData->Triangles.size() +
            std::distance(sharedData->Aabbs.begin(), jt) );

    return INDEX_NONE;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
