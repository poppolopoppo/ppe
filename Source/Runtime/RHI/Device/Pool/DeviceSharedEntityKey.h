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
struct FDeviceSharedEntityKey {
    
    size_t Data;

#ifdef ARCH_X64
    typedef Meta::TBit<size_t>::TFirst<60>::type field_hashValue;
    typedef Meta::TBit<size_t>::TAfter<field_hashValue>::FRemain::type field_resourceType;
#else
    typedef Meta::TBit<size_t>::TFirst<28>::type field_hashValue;
    typedef Meta::TBit<size_t>::TAfter<field_hashValue>::FRemain::type field_resourceType;
#endif

    EDeviceResourceType ResourceType() const { return EDeviceResourceType(field_resourceType::Get(Data)); }
    size_t HashValue() const { return field_hashValue::Get(Data); }

    bool operator ==(const FDeviceSharedEntityKey& other) const { return Data == other.Data; }
    bool operator !=(const FDeviceSharedEntityKey& other) const { return !operator ==(other); }

    bool operator < (const FDeviceSharedEntityKey& other) const { return Data <  other.Data; }
    bool operator >=(const FDeviceSharedEntityKey& other) const { return !operator < (other); }

    static FDeviceSharedEntityKey Invalid() { return FDeviceSharedEntityKey{size_t(-1)}; }

    static FDeviceSharedEntityKey Make(EDeviceResourceType type, size_t hashValue) {
        FDeviceSharedEntityKey result;
        field_resourceType::InplaceSet(result.Data, size_t(type));
        field_hashValue::InplaceSet(result.Data, ((hashValue >> field_resourceType::Count) ^ hashValue) & field_hashValue::Mask);
        return result;
    }
};
//----------------------------------------------------------------------------
FORCE_INLINE hash_t hash_value(const FDeviceSharedEntityKey& key) {
    return key.Data;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
