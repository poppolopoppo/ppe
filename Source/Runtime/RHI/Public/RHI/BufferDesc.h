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

    FBufferDesc() = default;
    FBufferDesc(size_t sizeInBytes, EBufferUsage usage, EQueueUsage queues = Default)
    :   SizeInBytes(sizeInBytes)
    ,   Usage(usage)
    ,   Queues(queues)
    {}

    FBufferDesc& SetSize(size_t sz) { SizeInBytes = sz; return (*this); }
    FBufferDesc& SetUsage(EBufferUsage usage) { Usage = usage; return (*this); }
    FBufferDesc& SetQueues(EQueueUsage queues) { Queues = queues; return (*this); }

    NODISCARD bool operator ==(const FBufferDesc& other) const NOEXCEPT {
        return (SizeInBytes == other.SizeInBytes && Usage == other.Usage && Queues == other.Queues);
    }
    NODISCARD bool operator !=(const FBufferDesc& other) const NOEXCEPT {
        return (not operator ==(other));
    }
};
PPE_ASSUME_TYPE_AS_POD(FBufferDesc)
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

    NODISCARD bool operator ==(const FBufferViewDesc& other) const NOEXCEPT {
        return (Format == other.Format && Offset == other.Offset && SizeInBytes == other.SizeInBytes);
    }
    NODISCARD bool operator !=(const FBufferViewDesc& other) const NOEXCEPT {
        return (not operator ==(other));
    }

    NODISCARD friend hash_t hash_value(const FBufferViewDesc& it) NOEXCEPT {
        return hash_tuple(it.Format, it.Offset, it.SizeInBytes);
    }
};
PPE_ASSUME_TYPE_AS_POD(FBufferViewDesc)
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace RHI
} //!namespace PPE
