#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/MaterialVariability.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantBuffer.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(AbstractMaterialParameter);
FWD_REFPTR(Material);
struct MaterialContext;
FWD_REFPTR(Scene);
//----------------------------------------------------------------------------
class EffectConstantBuffer : public Graphics::ConstantBuffer {
public:
    EffectConstantBuffer(   const Graphics::BindName& name,
                            const Graphics::ConstantBufferLayout *layout);
    virtual ~EffectConstantBuffer();

    const Graphics::BindName& Name() const { return _name; }
    const MaterialVariabilitySeed& Variability() const { return _variability; }
    const VECTOR(Effect, PAbstractMaterialParameter)& Parameters() const { return _parameters; }

    void Prepare(Graphics::IDeviceAPIEncapsulator *device, const Material *material, const Scene *scene);
    void Eval(Graphics::IDeviceAPIEncapsulator *device, const MaterialContext& context);
    bool Match(const Graphics::BindName& name, const Graphics::ConstantBufferLayout& layout) const;

    SINGLETON_POOL_ALLOCATED_DECL(EffectConstantBuffer);

private:
    Graphics::BindName _name;
    MaterialVariabilitySeed _variability;
    VECTOR(Effect, PAbstractMaterialParameter) _parameters;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
