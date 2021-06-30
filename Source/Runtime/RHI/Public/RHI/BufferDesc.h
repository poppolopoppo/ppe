#pragma once

#include "RHI_fwd.h"

#include "RHI/ResourceEnums.h"

#include "Container/Hash.h"

namespace PPE {
namespace RHI {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
struct FBufferDesc {
    u32 SizeInBytes{ 0 };
    EBufferUsage Usage{ Default };
    EQueueUsage Queues{ Default };

    FBufferDesc() = default;
    FBufferDesc(u32 sizeInBytes, EBufferUsage usage, EQueueUsage queues = Default)
    :   SizeInBytes(sizeInBytes)
    ,   Usage(usage)
    ,   Queues(queues)
    {}

    FBufferDesc& SetSize(u32 sz) { SizeInBytes = sz; return (*this); }
    FBufferDesc& SetUsage(EBufferUsage usage) { Usage = usage; return (*this); }
    FBufferDesc& SetQueues(EQueueUsage queues) { Queues = queues; return (*this); }

};
PPE_ASSUME_TYPE_AS_POD(FBufferDesc)
//----------------------------------------------------------------------------
struct FBufferViewDesc {
    EPixelFormat Format{ Default };
    u32 Offset{ UMax };
    u32 SizeInBytes{ UMax };

    FBufferViewDesc() = default;
    FBufferViewDesc(EPixelFormat format, u32 offset, u32 sizeInBytes)
    :   Format(format)
    ,   Offset(offset)
    ,   SizeInBytes(sizeInBytes)
    {}

    void Validate(const FBufferDesc& desc) {
        Assert(Offset < desc.SizeInBytes );
        Assert(Format != Default );

        Offset = Min(Offset, desc.SizeInBytes - 1);
        SizeInBytes	= Min(SizeInBytes, desc.SizeInBytes - Offset);
    }

    bool operator ==(const FBufferViewDesc& other) const {
        return (Format == other.Format && Offset == other.Offset && SizeInBytes == other.SizeInBytes);
    }
    bool operator !=(const FBufferViewDesc& other) const {
        return (not operator ==(other));
    }

    friend hash_t hash_value(const FBufferViewDesc& it) {
        return hash_tuple(it.Format, it.Offset, it.SizeInBytes);
    }
};
PPE_ASSUME_TYPE_AS_POD(FBufferViewDesc)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
