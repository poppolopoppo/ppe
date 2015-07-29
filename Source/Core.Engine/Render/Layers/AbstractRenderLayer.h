#pragma once

#include "Core.Engine/Engine.h"

#include "Core/IO/String.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/PointerWFlags.h"

#if defined(_DEBUG) || defined(WITH_CORE_GRAPHICS_DIAGNOSTICS)
#   define WITH_CORE_ENGINE_RENDERLAYER_DEBUGNAME
#endif

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class IDeviceAPIContext;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractRenderLayer);
class MaterialDatabase;
class RenderBatch;
class RenderTree;
struct VariabilitySeed;
//----------------------------------------------------------------------------
class AbstractRenderLayer : public RefCountable {
protected:
    AbstractRenderLayer(String&& name, bool enabled = true, bool exported = false, AbstractRenderLayer *next = nullptr);

public:
    virtual ~AbstractRenderLayer();

    AbstractRenderLayer(const AbstractRenderLayer& ) = delete;
    AbstractRenderLayer& operator =(const AbstractRenderLayer& ) = delete;

    const String& Name() const { return _name; }
    void SetName(String&& name);

    bool Enabled() const { return _nextWFlags.Flag0(); }
    void SetEnabled(bool value);

    bool Exported() const { return _nextWFlags.Flag1(); }

    AbstractRenderLayer *Next() const { return _nextWFlags.Get(); }
    void SetNext(AbstractRenderLayer *value);
    AbstractRenderLayer *NextLayer(size_t offset) const;

    RenderBatch *RenderBatchIFP();
    const RenderBatch *RenderBatchIFP() const;

    void Prepare(Graphics::IDeviceAPIEncapsulator *device, MaterialDatabase *materialDatabase, const RenderTree *renderTree, VariabilitySeed *seeds);
    void Render(Graphics::IDeviceAPIContext *context);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree);

protected:
    virtual RenderBatch *RenderBatchIFPImpl_() { return nullptr; }
    virtual const RenderBatch *RenderBatchIFPImpl_() const { return nullptr; }

    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, MaterialDatabase *materialDatabase, const RenderTree *renderTree, VariabilitySeed *seeds) = 0;
    virtual void RenderImpl_(Graphics::IDeviceAPIContext *context) = 0;
    virtual void DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const RenderTree *renderTree) = 0;

private:
    String _name;
    Meta::PointerWFlags<AbstractRenderLayer> _nextWFlags;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
