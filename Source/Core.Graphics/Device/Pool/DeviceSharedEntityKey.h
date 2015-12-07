#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceResourceType.h"

#include "Core/Container/Hash.h"
#include "Core/Meta/BitField.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct DeviceSharedEntityKey {
    
    size_t Data;

#ifdef ARCH_X64
    typedef Meta::Bit<size_t>::First<60>::type field_hashValue;
    typedef Meta::Bit<size_t>::After<field_hashValue>::Remain::type field_resourceType;
#else
    typedef Meta::Bit<size_t>::First<28>::type field_hashValue;
    typedef Meta::Bit<size_t>::After<field_hashValue>::Remain::type field_resourceType;
#endif

    DeviceResourceType ResourceType() const { return DeviceResourceType(field_resourceType::Get(Data)); }
    size_t HashValue() const { return field_hashValue::Get(Data); }

    bool operator ==(const DeviceSharedEntityKey& other) const { return Data == other.Data; }
    bool operator !=(const DeviceSharedEntityKey& other) const { return !operator ==(other); }

    bool operator < (const DeviceSharedEntityKey& other) const { return Data <  other.Data; }
    bool operator >=(const DeviceSharedEntityKey& other) const { return !operator < (other); }

    static DeviceSharedEntityKey Invalid() { return DeviceSharedEntityKey{size_t(-1)}; }

    static DeviceSharedEntityKey Make(DeviceResourceType type, size_t hashValue) {
        DeviceSharedEntityKey result;
        field_resourceType::InplaceSet(result.Data, size_t(type));
        field_hashValue::InplaceSet(result.Data, ((hashValue >> field_resourceType::Count) ^ hashValue) & field_hashValue::Mask);
        return result;
    }
};
//----------------------------------------------------------------------------
FORCE_INLINE hash_t hash_value(const DeviceSharedEntityKey& key) {
    return key.Data;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
