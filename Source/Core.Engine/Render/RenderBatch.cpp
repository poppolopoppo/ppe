#include "stdafx.h"

#include "RenderBatch.h"

#include "RenderCommand.h"
#include "RenderTree.h"

#include "Effect/Effect.h"
#include "Effect/MaterialEffect.h"
#include "Material/Material.h"
#include "Material/MaterialVariability.h"
#include "Scene/Scene.h"
#include "Texture/TextureCache.h"

#include "Core.Graphics/Device/DeviceAPIEncapsulator.h"
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
static void Reindex_(Vector<T, _Allocator>& v, const MemoryView<const u32>& indices) {
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
    VECTOR_THREAD_LOCAL(Shader, RenderCommandCriteria)& criterias,
    VECTOR_THREAD_LOCAL(Shader, RenderCommandParams)& params,
    VECTOR_THREAD_LOCAL(Shader, const RenderCommandRegistration *)& registrations
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
RenderBatch::RenderBatch() 
:   _needSort(false) {}
//----------------------------------------------------------------------------
RenderBatch::~RenderBatch() {
    Assert(_criterias.empty());
    Assert(_params.empty());
    Assert(_registrations.empty());
}
//----------------------------------------------------------------------------
void RenderBatch::Add(
    RenderCommandRegistration *registration,
    const RenderCommandCriteria& criteria,
    const RenderCommandParams& params ) {
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
RenderCommandCriteria RenderBatch::Remove(const RenderCommandRegistration *registration) {
    Assert(registration);
    Assert(this == registration->Batch);

    const auto it = std::find(_registrations.begin(), _registrations.end(), registration);
    Assert(_registrations.end() != it);

    const size_t index = std::distance(_registrations.begin(), it);

    const RenderCommandCriteria criteria = _criterias[index];

    _registrations.erase(it);
    _criterias.erase(_criterias.begin() + index);
    _params.erase(_params.begin() + index);

    const_cast<RenderCommandRegistration *>(registration)->Batch = nullptr;

    return criteria;
}
//----------------------------------------------------------------------------
void RenderBatch::Prepare(
    Graphics::IDeviceAPIEncapsulator *device,
    MaterialDatabase *materialDatabase,
    const RenderTree *renderTree,
    VariabilitySeed *seeds) {
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

    const Scene *scene = renderTree->Scene();
    const size_t count = _criterias.size();

    const RenderCommandCriteria *pred = nullptr;
    forrange(i, 0, count) {
        seeds[size_t(MaterialVariability::Batch)].Next();

        RenderCommandCriteria& criteria = _criterias[i];

        if (!pred || pred->MaterialEffect() != criteria.MaterialEffect()) {
            if (!criteria.Ready()) {
                criteria.MaterialEffect()->Create(device, materialDatabase, renderTree->Scene());
                criteria.SetReady();
            }

            seeds[size_t(MaterialVariability::Material)].Next();
            criteria.MaterialEffect()->Prepare(device, scene, seeds);
        }

        pred = &criteria;
    }
}
//----------------------------------------------------------------------------
void RenderBatch::Render(Graphics::IDeviceAPIContextEncapsulator *context) {
    const Graphics::AbstractDeviceAPIEncapsulator *encapsulator = context->Encapsulator();

    const size_t count = _criterias.size();

    const RenderCommandCriteria *pred = nullptr;
    forrange(i, 0, count) {
        const RenderCommandCriteria& criteria = _criterias[i];
        Assert(criteria.Ready());

        GRAPHICS_DIAGNOSTICS_BEGINEVENT(encapsulator, criteria.MaterialEffect()->Material()->Name().cstr());

        if (!pred || pred->Effect != criteria.Effect) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(encapsulator, criteria.Effect->ResourceName());
            criteria.Effect->Set(context);
        }

        if (!pred || pred->Vertices != criteria.Vertices) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(encapsulator, criteria.Vertices->ResourceName());
            context->SetVertexBuffer(criteria.Vertices);
        }

        if (!pred || pred->Indices != criteria.Indices) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(encapsulator, criteria.Indices->ResourceName());
            context->SetIndexBuffer(criteria.Indices);
        }

        if (!pred || pred->MaterialEffect() != criteria.MaterialEffect()) {
            GRAPHICS_DIAGNOSTICS_SETMARKER(encapsulator, criteria.MaterialEffect()->Material()->Name().cstr());
            criteria.MaterialEffect()->Set(context);
        }

        const RenderCommandParams& params = _params[i];

        context->DrawIndexedPrimitives( params.PrimitiveType(),
                                        params.BaseVertex,
                                        params.StartIndex,
                                        params.PrimitiveCount() );

        GRAPHICS_DIAGNOSTICS_ENDEVENT(encapsulator);

        pred = &criteria;
    }
}
//----------------------------------------------------------------------------
void RenderBatch::Destroy(Graphics::IDeviceAPIEncapsulator *device) {
    Assert(_criterias.empty());
    Assert(_params.empty());
    Assert(_registrations.empty());
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
