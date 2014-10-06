#pragma once

#include "Engine.h"

#include "Core/RefPtr.h"
#include "Core/Vector.h"

#include "RenderCommand.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContextEncapsulator;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(MaterialEffect);
class RenderTree;
struct VariabilitySeed;
//----------------------------------------------------------------------------
class RenderBatch {
public:
    RenderBatch();
    ~RenderBatch();

    void Add(const RenderCommand *pcommand);
    void Remove(const RenderCommand *pcommand);

    void Prepare(   Graphics::IDeviceAPIEncapsulator *device,
                    const RenderTree *renderTree,
                    VariabilitySeed *seeds);
    void Render(Graphics::IDeviceAPIContextEncapsulator *context);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

private:
    VECTOR_THREAD_LOCAL(Shader, const RenderCommand *) _commands;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
