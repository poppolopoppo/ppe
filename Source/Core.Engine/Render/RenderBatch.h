#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContextEncapsulator;
}

namespace Engine {
struct RenderCommandCriteria;
struct RenderCommandParams;
struct RenderCommandRegistration;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MaterialDatabase);
FWD_REFPTR(MaterialEffect);
class RenderTree;
struct VariabilitySeed;
//----------------------------------------------------------------------------
class RenderBatch {
public:
    RenderBatch();
    ~RenderBatch();

    void Add(   RenderCommandRegistration *registration, 
                const RenderCommandCriteria& criteria, 
                const RenderCommandParams& params );
    RenderCommandCriteria Remove(const RenderCommandRegistration *registration);

    void Prepare(   Graphics::IDeviceAPIEncapsulator *device,
                    MaterialDatabase *materialDatabase,
                    const RenderTree *renderTree,
                    VariabilitySeed *seeds);
    void Render(Graphics::IDeviceAPIContextEncapsulator *context);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

private:
    VECTOR_THREAD_LOCAL(Shader, RenderCommandCriteria) _criterias;
    VECTOR_THREAD_LOCAL(Shader, RenderCommandParams) _params;

    bool _needSort;

    VECTOR_THREAD_LOCAL(Shader, const RenderCommandRegistration *) _registrations;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
