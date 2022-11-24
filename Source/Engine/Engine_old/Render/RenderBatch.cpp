// PPE - PoPpOlOpOPpo Engine. All Rights Reserved.

#include "RenderBatch.h"

#include "RenderCommand.h"
#include "RenderTree.h"

#include "Effect/Effect.h"
#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialVariability.h"
#include "Scene/Scene.h"
#include "Texture/TextureCache.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/DeviceDiagnostics.h"
#include "Core.Graphics/Device/Geometry/IndexBuffer.h"
#include "Core.Graphics/Device/Geometry/PrimitiveType.h"
#include "Core.Graphics/Device/Geometry/VertexBuffer.h"

#include "Core/Memory/UniqueView.h"

#include <algorithm>
#include <functional>

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
namespace {
//----------------------------------------------------------------------------
template <typename T, typename _Allocator>
static void Reindex_(TVector<T, _Allocator>& v, const TMemoryView<const u32>& indices) {
    Assert(v.size() == indices.size());
    using std::swap;

    const size_t count = indices.size();
    T *const pitem = &v[0];
    const u32 *const pindex = indices.Pointer();

    STACKLOCAL_POD_ARRAY(T, tmp, count);
    memcpy(tmp.Pointer(), pitem, tmp.SizeInBytes());

    forrange(i, 0, count)
        if (i != pindex[i])
            pitem[i] = tmp[pindex[i]];
}
//----------------------------------------------------------------------------
static void SortBatch_(
    VECTOR_THREAD_LOCAL(Shader, FRenderCommandCriteria)& criterias,
    VECTOR_THREAD_LOCAL(Shader, FRenderCommandParams)& params,
    VECTOR_THREAD_LOCAL(Shader, const FRenderCommandRegistration *)& registrations
    ) {
    Assert(criterias.size() == params.size());
    Assert(params.size() == registrations.size());

    const u32 count = checked_cast<u32>(criterias.size());
    Assert(count > 0);

    STACKLOCAL_POD_ARRAY(u32, reindexation, count);
    forrange(i, 0, count)
        reindexation[i] = i;

    std::sort(reindexation.begin(), reindexation.end(), [&](u32 lhs, u32 rhs) {
        return criterias[lhs] < criterias[rhs];
    });

    Reindex_(criterias, reindexation.Cast<const u32>());
    Reindex_(params, reindexation.Cast<const u32>());
    Reindex_(registrations, reindexation.Cast<const u32>());
}
//----------------------------------------------------------------------------
} //!namespace
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FRenderBatch::FRenderBatch() 
:   _needSort(false) {}
//----------------------------------------------------------------------------
FRenderBatch::~FRenderBatch() {
    Assert(_criterias.empty());
    Assert(_params.empty());
    Assert(_registrations.empty());
}
//----------------------------------------------------------------------------
void FRenderBatch::Add(
    FRenderCommandRegistration *registration,
    const FRenderCommandCriteria& criteria,
    const FRenderCommandParams& params ) {
    Assert(registration);
    Assert(nullptr == registration->Batch);
    Assert(criteria.MaterialEffect());
    Assert(criteria.Indices);
    Assert(criteria.Vertices);
    Assert(criteria.Effect == criteria.MaterialEffect()->Effect());
    Assert(!criteria.Ready());

    registration->Batch = this;

    _registrations.push_back(registration);
    _criterias.push_back(criteria);
    _params.push_back(params);

    _needSort = true;
}
//----------------------------------------------------------------------------
FRenderCommandCriteria FRenderBatch::Remove(const FRenderCommandRegistration *registration) {
    Assert(registration);
    Assert(this == registration->Batch);

    const auto it = std::find(_registrations.begin(), _registrations.end(), registration);
    Assert(_registrations.end() != it);

    const size_t index = std::distance(_registrations.begin(), it);

    const FRenderCommandCriteria criteria = _criterias[index];

    _registrations.erase(it);
    _criterias.erase(_criterias.begin() + index);
    _params.erase(_params.begin() + index);

    const_cast<FRenderCommandRegistration *>(registration)->Batch = nullptr;

    return criteria;
}
//----------------------------------------------------------------------------
void FRenderBatch::Prepare(
    Graphics::IDeviceAPIEncapsulator *device,
    FMaterialDatabase *materialDatabase,
    const FRenderTree *renderTree,
    FVariabilitySeed *seeds) {
    if (_criterias.empty()) {
        Assert(!_needSort);
        Assert(_params.empty());
        Assert(_registrations.empty());
        return;
    }

    if (_needSort) {
        SortBatch_(_criterias, _params, _registrations);
        _needSort = false;
    }

    const FScene *scene = renderTree->Scene();
    const size_t count = _criterias.size();

    const FRenderCommandCriteria *pred = nullptr;
    forrange(i, 0, count) {
        seeds[size_t(EMaterialVariability::Batch)].Next();

        FRenderCommandCriteria& criteria = _criterias[i];

        if (!pred || pred->MaterialEffect() != criteria.MaterialEffect()) {
            if (!criteria.Ready()) {
                criteria.MaterialEffect()->Create(device, materialDatabase, renderTree->Scene());
                criteria.SetReady();
            }

            seeds[size_t(EMaterialVariability::FMaterial)].Next();
            criteria.MaterialEffect()->Prepare(device, scene, seeds);
        }

        pred = &criteria;
    }
}
//----------------------------------------------------------------------------
void FRenderBatch::Render(Graphics::IDeviceAPIContext *context) {
    const size_t count = _criterias.size();

    const FRenderCommandCriteria *pred = nullptr;
    forrange(i, 0, count) {
        const FRenderCommandCriteria& criteria = _criterias[i];
        Assert(criteria.Ready());

        GRAPHICS_DIAGNOSTICS_BEGINEVENT(context->Diagnostics(), criteria.MaterialEffect()->Material()->Name().c_str());

        if (!pred || pred->Effect != criteria.Effect) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(context->Diagnostics(), criteria.Effect->ResourceName());
            criteria.Effect->Set(context);
        }

        if (!pred || pred->Vertices != criteria.Vertices) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(context->Diagnostics(), criteria.Vertices->ResourceName());
            context->SetVertexBuffer(criteria.Vertices);
        }

        if (!pred || pred->Indices != criteria.Indices) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(context->Diagnostics(), criteria.Indices->ResourceName());
            context->SetIndexBuffer(criteria.Indices);
        }

        if (!pred || pred->MaterialEffect() != criteria.MaterialEffect()) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(context->Diagnostics(), criteria.MaterialEffect()->Material()->Name().c_str());
            criteria.MaterialEffect()->Set(context);
        }

        const FRenderCommandParams& params = _params[i];

        context->DrawIndexedPrimitives( params.PrimitiveType(),
                                        params.BaseVertex,
                                        params.StartIndex,
                                        params.PrimitiveCount() );

        GRAPHICS_DIAGNOSTICS_ENDEVENT(context->Diagnostics());

        pred = &criteria;
    }
}
//----------------------------------------------------------------------------
void FRenderBatch::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    Assert(_criterias.empty());
    Assert(_params.empty());
    Assert(_registrations.empty());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
