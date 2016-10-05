#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/Hash.h"
#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/ThreadResource.h"

#include "Core.Engine/Material/MaterialVariability.h"

namespace Core {
namespace Graphics {
class FBindName;
class IDeviceAPIEncapsulator;
FWD_REFPTR(VertexDeclaration);
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(Effect);
FWD_REFPTR(EffectDescriptor);
FWD_REFPTR(Material);
FWD_REFPTR(MaterialEffect);
class FSharedConstantBufferFactory;
//----------------------------------------------------------------------------
struct FEffectCompilerKey;
//----------------------------------------------------------------------------
class FEffectCompiler : public Meta::FThreadResource {
public:
    FEffectCompiler();
    ~FEffectCompiler();

    FVariabilitySeed Variability() const { return _variability; }

    FEffect *GetOrCreateEffect(  const FEffectDescriptor *descriptor, 
                                const Graphics::FVertexDeclaration *vertexDeclaration,
                                const TMemoryView<const Graphics::FBindName>& tags );

    FMaterialEffect *CreateMaterialEffect(   const FEffectDescriptor *descriptor,
                                            const Graphics::FVertexDeclaration *vertexDeclaration,
                                            const FMaterial *material);

    void RegenerateEffects();
    void Clear();

    void Start(Graphics::IDeviceAPIEncapsulator *device, FSharedConstantBufferFactory *sharedBufferFactory);
    void Shutdown(Graphics::IDeviceAPIEncapsulator *device, FSharedConstantBufferFactory *sharedBufferFactory);

private:
    Graphics::IDeviceAPIEncapsulator *_device;
    FSharedConstantBufferFactory *_sharedBufferFactory;

    FVariabilitySeed _variability;

    HASHMAP(FEffect, FEffectCompilerKey, PEffect) _effects;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
