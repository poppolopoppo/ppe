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
class FMaterialDatabase;
class FRenderBatch;
class FRenderTree;
struct FVariabilitySeed;
//----------------------------------------------------------------------------
class FAbstractRenderLayer : public FRefCountable {
protected:
    FAbstractRenderLayer(FString&& name, bool enabled = true, bool exported = false, FAbstractRenderLayer *next = nullptr);

public:
    virtual ~FAbstractRenderLayer();

    FAbstractRenderLayer(const FAbstractRenderLayer& ) = delete;
    FAbstractRenderLayer& operator =(const FAbstractRenderLayer& ) = delete;

    const FString& FName() const { return _name; }
    void SetName(FString&& name);

    bool Enabled() const { return _nextWFlags.Flag0(); }
    void SetEnabled(bool value);

    bool Exported() const { return _nextWFlags.Flag1(); }

    FAbstractRenderLayer *Next() const { return _nextWFlags.Get(); }
    void SetNext(FAbstractRenderLayer *value);
    FAbstractRenderLayer *NextLayer(size_t offset) const;

    FRenderBatch *RenderBatchIFP();
    const FRenderBatch *RenderBatchIFP() const;

    void Prepare(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, const FRenderTree *renderTree, FVariabilitySeed *seeds);
    void Render(Graphics::IDeviceAPIContext *context);
    void Destroy(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree *renderTree);

protected:
    virtual FRenderBatch *RenderBatchIFPImpl_() { return nullptr; }
    virtual const FRenderBatch *RenderBatchIFPImpl_() const { return nullptr; }

    virtual void PrepareImpl_(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, const FRenderTree *renderTree, FVariabilitySeed *seeds) = 0;
    virtual void RenderImpl_(Graphics::IDeviceAPIContext *context) = 0;
    virtual void DestroyImpl_(Graphics::IDeviceAPIEncapsulator *device, const FRenderTree *renderTree) = 0;

private:
    FString _name;
    Meta::TPointerWFlags<FAbstractRenderLayer> _nextWFlags;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
