#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Engine/Material/MaterialParameter_fwd.h"
#include "Core.Engine/Material/MaterialVariability.h"

#include "Core.Graphics/Device/BindName.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Container/RawStorage.h"
#include "Core/Container/Vector.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
}

namespace Engine {
class MaterialDatabase;
FWD_REFPTR(SharedConstantBuffer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(EffectConstantBuffer);
class EffectConstantBuffer : public RefCountable {
public:
    explicit EffectConstantBuffer(SharedConstantBuffer *sharedBuffer);
    virtual ~EffectConstantBuffer();

    const SharedConstantBuffer *SharedBuffer() const { return _sharedBuffer.get(); }

    const MaterialVariabilitySeed& Variability() const { return _variability; }
    const VECTOR(Effect, PMaterialParameter)& Parameters() const { return _parameters; }

    void Prepare(const MaterialParameterMutableContext& context);
    void Eval(const MaterialParameterContext& context, const VariabilitySeeds& seeds);
    void SetDataIFN(Graphics::IDeviceAPIEncapsulator *device) const;
    void Clear();

    SINGLETON_POOL_ALLOCATED_DECL(EffectConstantBuffer);

private:
    PSharedConstantBuffer _sharedBuffer;

    size_t _headerHashValue;
    MaterialVariabilitySeed _variability;
    VECTOR(Effect, PMaterialParameter) _parameters;

    size_t _dataHashValue;
    RAWSTORAGE_ALIGNED(Effect, u8, 16) _rawData;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
