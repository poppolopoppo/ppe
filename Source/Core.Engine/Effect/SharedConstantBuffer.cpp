#include "stdafx.h"

#include "SharedConstantBuffer.h"

#include "Core.Graphics/Device/DeviceAPI.h"
#include "Core.Graphics/Device/Shader/ConstantBufferLayout.h"
#include "Core.Graphics/Device/Shader/ConstantField.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Container/Hash.h"

namespace Core {
namespace Engine {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
size_t hash_value(const SharedConstantBufferKey& key) {
    return Core::hash_value(key.Name, *key.Layout);
}
//----------------------------------------------------------------------------
bool operator ==(const SharedConstantBufferKey& lhs, const SharedConstantBufferKey& rhs) {
    return lhs.Name == rhs.Name && lhs.Layout->Equals(*rhs.Layout);
}
//----------------------------------------------------------------------------
bool operator !=(const SharedConstantBufferKey& lhs, const SharedConstantBufferKey& rhs) {
    return !operator ==(lhs, rhs);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Engine, SharedConstantBuffer, )
//----------------------------------------------------------------------------
SharedConstantBuffer::SharedConstantBuffer(const SharedConstantBufferKey& sharedKey)
:   Graphics::ConstantBuffer(sharedKey.Layout)
,   _sharedKey(sharedKey)
,   _headerHashValue(0)
,   _dataHashValue(0) {
    Assert(!_sharedKey.Name.empty());
    Assert(_sharedKey.Layout);
}
//----------------------------------------------------------------------------
SharedConstantBuffer::~SharedConstantBuffer() {}
//----------------------------------------------------------------------------
bool SharedConstantBuffer::SetData_OnlyIfChanged( 
    Graphics::IDeviceAPIEncapsulator *device, 
    size_t headerHashValue,
    size_t dataHashValue,
    const MemoryView<const u8>& rawData ) {
    Assert(headerHashValue);
    Assert(dataHashValue);
    Assert(!rawData.empty());

    _headerHashValue = headerHashValue;
    if (dataHashValue == _dataHashValue)
        return false;

    ConstantBuffer::SetData(device, rawData);
    _dataHashValue = dataHashValue;

    return true;
}
//----------------------------------------------------------------------------
bool SharedConstantBuffer::Mergeable( 
    const Graphics::BindName& name,
    const Graphics::ConstantBufferLayout *layout ) const {
    Assert(!name.empty());
    Assert(layout);

    return  _sharedKey.Name == name &&
            _sharedKey.Layout->Equals(*layout);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Engine
} //!namespace Core
