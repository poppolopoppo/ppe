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
class FMaterialDatabase;
FWD_REFPTR(SharedConstantBuffer);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(EffectConstantBuffer);
class FEffectConstantBuffer : public FRefCountable {
public:
    explicit FEffectConstantBuffer(FSharedConstantBuffer *sharedBuffer);
    virtual ~FEffectConstantBuffer();

    const FSharedConstantBuffer *SharedBuffer() const { return _sharedBuffer.get(); }

    const FMaterialVariabilitySeed& Variability() const { return _variability; }
    const VECTOR(FEffect, PMaterialParameter)& Parameters() const { return _parameters; }

    void Prepare(const FMaterialParameterMutableContext& context);
    void Eval(const FMaterialParameterContext& context, const VariabilitySeeds& seeds);
    void SetDataIFN(Graphics::IDeviceAPIEncapsulator *device) const;
    void Clear();

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    PSharedConstantBuffer _sharedBuffer;

    size_t _headerHashValue;
    FMaterialVariabilitySeed _variability;
    VECTOR(FEffect, PMaterialParameter) _parameters;

    size_t _dataHashValue;
    RAWSTORAGE_ALIGNED(FEffect, u8, 16) _rawData;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
