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
struct SharedConstantBufferKey {
    Graphics::BindName Name;
    Graphics::SCConstantBufferLayout Layout;
};
//----------------------------------------------------------------------------
size_t hash_value(const SharedConstantBufferKey& key);
//----------------------------------------------------------------------------
bool operator ==(const SharedConstantBufferKey& lhs, const SharedConstantBufferKey& rhs);
bool operator !=(const SharedConstantBufferKey& lhs, const SharedConstantBufferKey& rhs);
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
FWD_REFPTR(SharedConstantBuffer);
class SharedConstantBuffer : public Graphics::ConstantBuffer {
public:
    SharedConstantBuffer(const SharedConstantBufferKey& sharedKey);
    virtual ~SharedConstantBuffer();

    const SharedConstantBufferKey& SharedKey() const { return _sharedKey; }

    size_t HeaderHashValue() const { return _headerHashValue; } // <=> parameters
    size_t DataHashValue() const { return _dataHashValue; } // <=> buffer content

    bool SetData_OnlyIfChanged( Graphics::IDeviceAPIEncapsulator *device, 
                                size_t headerHashValue,
                                size_t dataHashValue,
                                const MemoryView<const u8>& rawData );

    bool Mergeable( const Graphics::BindName& name,
                    const Graphics::ConstantBufferLayout *layout ) const;

    SINGLETON_POOL_ALLOCATED_DECL(SharedConstantBuffer);

private:
    SharedConstantBufferKey _sharedKey;

    size_t _headerHashValue;
    size_t _dataHashValue;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
