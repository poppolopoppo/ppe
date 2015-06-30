#pragma once

#include "Core.Engine/Engine.h"

#include "Core/Container/Hash.h"
#include "Core/Container/HashMap.h"
#include "Core/Memory/RefPtr.h"
#include "Core/Meta/ThreadResource.h"

#include "Core.Engine/Material/MaterialVariability.h"

namespace Core {
namespace Graphics {
class BindName;
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
class SharedConstantBufferFactory;
//----------------------------------------------------------------------------
struct EffectCompilerKey;
//----------------------------------------------------------------------------
class EffectCompiler : public Meta::ThreadResource {
public:
    EffectCompiler();
    ~EffectCompiler();

    VariabilitySeed Variability() const { return _variability; }

    Effect *GetOrCreateEffect(  const EffectDescriptor *descriptor, 
                                const Graphics::VertexDeclaration *vertexDeclaration,
                                const MemoryView<const Graphics::BindName>& tags );

    MaterialEffect *CreateMaterialEffect(   const EffectDescriptor *descriptor,
                                            const Graphics::VertexDeclaration *vertexDeclaration,
                                            const Material *material);

    void RegenerateEffects();
    void Clear();

    void Start(Graphics::IDeviceAPIEncapsulator *device, SharedConstantBufferFactory *sharedBufferFactory);
    void Shutdown(Graphics::IDeviceAPIEncapsulator *device, SharedConstantBufferFactory *sharedBufferFactory);

private:
    Graphics::IDeviceAPIEncapsulator *_device;
    SharedConstantBufferFactory *_sharedBufferFactory;

    VariabilitySeed _variability;

    HASHMAP(Effect, EffectCompilerKey, PEffect) _effects;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
