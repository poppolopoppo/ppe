#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContext;
}

namespace Engine {
struct FRenderCommandCriteria;
struct FRenderCommandParams;
struct FRenderCommandRegistration;
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FMaterialDatabase;
FWD_REFPTR(MaterialEffect);
class FRenderTree;
struct FVariabilitySeed;
//----------------------------------------------------------------------------
class FRenderBatch {
public:
    FRenderBatch();
    ~FRenderBatch();

    void Add(   FRenderCommandRegistration *registration, 
                const FRenderCommandCriteria& criteria, 
                const FRenderCommandParams& params );
    FRenderCommandCriteria Remove(const FRenderCommandRegistration *registration);

    void Prepare(   Graphics::IDeviceAPIEncapsulator *device,
                    FMaterialDatabase *materialDatabase,
                    const FRenderTree *renderTree,
                    FVariabilitySeed *seeds);
    void Render(Graphics::IDeviceAPIContext *context);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

private:
    VECTOR_THREAD_LOCAL(Shader, FRenderCommandCriteria) _criterias;
    VECTOR_THREAD_LOCAL(Shader, FRenderCommandParams) _params;

    bool _needSort;

    VECTOR_THREAD_LOCAL(Shader, const FRenderCommandRegistration *) _registrations;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
