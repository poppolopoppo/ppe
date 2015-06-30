#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core/Container/Hash.h"

#include "Core.Graphics/Device/DeviceResourceType.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct DeviceSharedEntityKey {
    DeviceResourceType Type;
    size_t HashValue;

    bool operator ==(const DeviceSharedEntityKey& other) const {
        return  other.Type == Type && 
                other.HashValue == HashValue;
    }

    bool operator < (const DeviceSharedEntityKey& other) const {
        return (other.Type == Type)
            ? HashValue < other.HashValue
            : size_t(Type) < size_t(other.Type);
    }

    bool operator !=(const DeviceSharedEntityKey& other) const { return !operator ==(other); }
    bool operator >=(const DeviceSharedEntityKey& other) const { return !operator < (other); }
};
//----------------------------------------------------------------------------
FORCE_INLINE size_t hash_value(const DeviceSharedEntityKey& key) {
    return Core::hash_value(size_t(key.Type), key.HashValue);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
