#pragma once

#include "Engine.h"

#include "Core/Hash.h"
#include "Core/HashMap.h"
#include "Core/RefPtr.h"
#include "Core/ThreadResource.h"

#include "MaterialVariability.h"

namespace Core {
namespace Graphics {
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
//----------------------------------------------------------------------------
class EffectCompiler : public Meta::ThreadResource {
public:
    struct EffectKey {
        PCEffectDescriptor Descriptor;
        Graphics::PCVertexDeclaration VertexDeclaration;

        size_t HashValue() const { return hash_value(Descriptor, VertexDeclaration); }

        bool operator ==(const EffectKey& other) const { return Descriptor == other.Descriptor && VertexDeclaration == other.VertexDeclaration; }
        bool operator !=(const EffectKey& other) const { return !operator ==(other); }
    };

    EffectCompiler();
    ~EffectCompiler();

    VariabilitySeed Variability() const { return _variability; }

    Effect *GetOrCreateEffect(const EffectDescriptor *descriptor, const Graphics::VertexDeclaration *vertexDeclaration);

    MaterialEffect *CreateMaterialEffect(   const EffectDescriptor *descriptor,
                                            const Graphics::VertexDeclaration *vertexDeclaration,
                                            const Material *material);

    void RegenerateEffects();
    void Clear();

    void Start(Graphics::IDeviceAPIEncapsulator *device);
    void Shutdown(Graphics::IDeviceAPIEncapsulator *device);

private:
    Graphics::IDeviceAPIEncapsulator *_device;

    VariabilitySeed _variability;
    HASHMAP(Effect, EffectKey, PEffect) _effects;
};
//----------------------------------------------------------------------------
inline size_t hash_value(const EffectCompiler::EffectKey& key) {
    return key.HashValue();
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
