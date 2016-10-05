#pragma once

#include "Core.Engine/Engine.h"

#include "Core.Graphics/Device/BindName.h"
#include "Core.Graphics/Device/Shader/ConstantBuffer.h"

#include "Core/Allocator/PoolAllocator.h"
#include "Core/Memory/MemoryView.h"

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
}

namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FSharedConstantBufferKey {
    Graphics::FBindName FName;
    Graphics::SCConstantBufferLayout Layout;
};
//----------------------------------------------------------------------------
hash_t hash_value(const FSharedConstantBufferKey& key);
//----------------------------------------------------------------------------
bool operator ==(const FSharedConstantBufferKey& lhs, const FSharedConstantBufferKey& rhs);
bool operator !=(const FSharedConstantBufferKey& lhs, const FSharedConstantBufferKey& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(SharedConstantBuffer);
class FSharedConstantBuffer : public Graphics::FConstantBuffer {
public:
    FSharedConstantBuffer(const FSharedConstantBufferKey& sharedKey);
    virtual ~FSharedConstantBuffer();

    const FSharedConstantBufferKey& SharedKey() const { return _sharedKey; }

    size_t HeaderHashValue() const { return _headerHashValue; } // <=> parameters
    size_t DataHashValue() const { return _dataHashValue; } // <=> buffer content

    bool SetData_OnlyIfChanged( Graphics::IDeviceAPIEncapsulator *device, 
                                size_t headerHashValue,
                                size_t dataHashValue,
                                const TMemoryView<const u8>& rawData );

    bool Mergeable( const Graphics::FBindName& name,
                    const Graphics::FConstantBufferLayout *layout ) const;

    SINGLETON_POOL_ALLOCATED_DECL();

private:
    FSharedConstantBufferKey _sharedKey;

    size_t _headerHashValue;
    size_t _dataHashValue;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
