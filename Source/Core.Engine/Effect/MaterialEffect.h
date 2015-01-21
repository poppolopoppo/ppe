#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Device/BindName.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Pair.h"
#include "Core/Container/Vector.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Memory/WeakPtr.h"
#include "Core/Meta/PointerWFlags.h"

namespace Core {
class Filename;

namespace Graphics {
class BindName;
class IDeviceAPIContextEncapsulator;
class IDeviceAPIEncapsulator;
FWD_REFPTR(SamplerState);
FWD_WEAKPTR(Texture);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractMaterialParameter);
FWD_REFPTR(Effect);
FWD_REFPTR(EffectConstantBuffer);
FWD_REFPTR(Material);
struct MaterialContext;
FWD_REFPTR(RenderSurfaceLock);
FWD_REFPTR(Scene);
struct VariabilitySeed;
//----------------------------------------------------------------------------
class MaterialEffect : public RefCountable {
public:
    struct TextureSlot {
        TextureSlot(const Graphics::BindName& name,
                    const Graphics::SamplerState *sampler,
                    bool useSRGB,
                    bool isCubeMap,
                    bool isVirtualTexture = false);

        Graphics::BindName Name;
        const Graphics::SamplerState *Sampler;

        bool UseSRGB;
        bool IsCubeMap;
        bool IsVirtualTexture;
    };

    struct TextureBinding {
        Core::Filename Filename;
        Graphics::WCTexture Texture;
        PRenderSurfaceLock SurfaceLock;
    };

    MaterialEffect(const Engine::Effect *effect, const Engine::Material *material);
    ~MaterialEffect();

    const PCEffect& Effect() const { return _effect; }
    const PCMaterial& Material() const { return _material; }

    const VECTOR(Effect, PEffectConstantBuffer)& Constants() const { return _constants; }
    const VECTOR(Effect, TextureSlot)& TextureSlots() const { return _textureSlots; }
    const VECTOR(Effect, TextureBinding)& TextureBindings() const { return _textureBindings; }

    /* (1) link parameters and textures from database and material */
    void Create(Graphics::IDeviceAPIEncapsulator *device, const Scene *scene);
    /* (4) destroy binded resources, ready to call Create() again */
    void Destroy(Graphics::IDeviceAPIEncapsulator *device);

    /* (2) eval linked parameters and update constant buffers IFN, retrieves texture resources */
    void Prepare(Graphics::IDeviceAPIEncapsulator *device, const Scene *scene, const VariabilitySeed *seeds);
    /* (3) setup constant buffers and textures (with sampler states) on the device */
    void Set(Graphics::IDeviceAPIContextEncapsulator *deviceContext);

    SINGLETON_POOL_ALLOCATED_DECL(MaterialEffect);

private:
    PCEffect _effect;
    PCMaterial _material;

    VECTOR(Effect, PEffectConstantBuffer) _constants;
    VECTOR(Effect, TextureSlot) _textureSlots;
    VECTOR(Effect, TextureBinding) _textureBindings;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
