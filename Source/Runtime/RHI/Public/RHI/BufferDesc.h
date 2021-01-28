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
    size_t SizeInBytes{ 0 };
    EBufferUsage Usage{ Default };
    EQueueUsage Queues{ Default };
    bool External{ false };

    FBufferDesc() = default;
    FBufferDesc(size_t sizeInBytes, EBufferUsage usage, EQueueUsage queues = Default)
    :   SizeInBytes(sizeInBytes)
    ,   Usage(usage)
    ,   Queues(queues)
    {}

    FBufferDesc& SetSize(size_t sz) { SizeInBytes = sz; return (*this); }
    FBufferDesc& SetUsage(EBufferUsage usage) { Usage = usage; return (*this); }
    FBufferDesc& SetQueues(EQueueUsage queues) { Queues = queues; return (*this); }

};
//----------------------------------------------------------------------------
struct FBufferViewDesc {
    EPixelFormat Format{ Default };
    size_t Offset{ UMax };
    size_t SizeInBytes{ UMax };

    FBufferViewDesc() = default;
    FBufferViewDesc(EPixelFormat format, size_t offset, size_t sizeInBytes)
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
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
