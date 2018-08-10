#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Device/BindName.h"

#include "Core/Container/AssociativeVector.h"
#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/WeakPtr.h"
#include "Core/Meta/PointerWFlags.h"

#include "Core.Engine/Material/MaterialParameter_fwd.h"

namespace Core {
class FFilename;

namespace Graphics {
class FBindName;
class IDeviceAPIContext;
class IDeviceAPIEncapsulator;
FWD_REFPTR(SamplerState);
FWD_WEAKPTR(Texture);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Effect);
FWD_REFPTR(EffectConstantBuffer);
FWD_REFPTR(Material);
class FMaterialDatabase;
FWD_REFPTR(RenderSurfaceLock);
FWD_REFPTR(Scene);
struct FVariabilitySeed;
//----------------------------------------------------------------------------
class FMaterialEffect : public FRefCountable {
public:
    struct FTextureSlot {
        FTextureSlot(const Graphics::FBindName& name,
                    const Graphics::FSamplerState *sampler,
                    bool useSRGB,
                    bool isCubeMap,
                    bool isVirtualTexture = false);

        Graphics::FBindName FName;
        const Graphics::FSamplerState *Sampler;

        bool UseSRGB;
        bool IsCubeMap;
        bool IsVirtualTexture;
    };

    struct FTextureBinding {
        PPE::FFilename FFilename;
        Graphics::WCTexture FTexture;
        PRenderSurfaceLock SurfaceLock;
    };

    FMaterialEffect(const Engine::FEffect *effect, const Engine::FMaterial *material);
    ~FMaterialEffect();

    const PCEffect& FEffect() const { return _effect; }
    const PCMaterial& FMaterial() const { return _material; }

    const VECTOR(FEffect, PEffectConstantBuffer)& Constants() const { return _constants; }
    const VECTOR(FEffect, FTextureSlot)& TextureSlots() const { return _textureSlots; }
    const VECTOR(FEffect, FTextureBinding)& TextureBindings() const { return _textureBindings; }
    const ASSOCIATIVE_VECTOR(FEffect, Graphics::FBindName, PMaterialParameter)& Parameters() const { return _parameters; }

    void BindParameter(const Graphics::FBindName& name, IMaterialParameter *parameter);

    /* (1) link parameters and textures from database and material */
    void Create(Graphics::IDeviceAPIEncapsulator *device, FMaterialDatabase *materialDatabase, const FScene *scene);
    /* (4) destroy binded resources, ready to call Create() again */
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    /* (2) eval linked parameters and update constant buffers IFN, retrieves texture resources */
    void Prepare(Graphics::IDeviceAPIEncapsulator *device, const FScene *scene, const FVariabilitySeed *seeds);
    /* (3) setup constant buffers and textures (with sampler states) on the device */
    void Set(Graphics::IDeviceAPIContext *deviceContext);

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    PCEffect _effect;
    PCMaterial _material;

    VECTOR(FEffect, PEffectConstantBuffer) _constants;
    VECTOR(FEffect, FTextureSlot) _textureSlots;
    VECTOR(FEffect, FTextureBinding) _textureBindings;
    ASSOCIATIVE_VECTOR(FEffect, Graphics::FBindName, PMaterialParameter) _parameters;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
