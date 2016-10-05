#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResourceType.h"

#include "Core/Meta/BitField.h"

namespace Core {
    template <typename T>
    class TMemoryView;
} //!namespace Core

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class FDeviceResource;
FWD_REFPTR(DeviceAPIDependantResourceBuffer);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class EBufferMode : u32 {
    None            = 0,
    Read            = 1<<0,
    Write           = 1<<1,
    Discard         = 1<<2,
    DoNotWait       = 1<<3,
    ReadWrite       = Read|Write,
    WriteDiscard    = Write|Discard,
    WriteDoNotWait  = Write|DoNotWait,
};
ENUM_FLAGS(EBufferMode)
//----------------------------------------------------------------------------
// http://amd-dev.wpengine.netdna-cdn.com/wordpress/media/2012/10/Ultimate%20Graphics%20Performance%20for%20DirectX%2010%20Hardware%20-%20Develop%202008.pdf
// https://developer.nvidia.com/sites/default/files/akamai/gamedev/files/gdc12/Efficient_Buffer_Management_McDonald.pdf
// - Forever : Immutable
// - Long lived : Default
// - Temporary : Dynamic
// - Constant : Staging
enum class EBufferUsage : u32 {
    Default     = 0,    // The CPU updates the resource less than once per frame
    Immutable   ,       // The CPU does not update the resource
    Dynamic     ,       // The CPU updates the resource more than once per frame
    Staging     ,       // The CPU needs to read the resource
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceResourceBuffer {
public:
    FDeviceResourceBuffer(size_t stride, size_t count, EBufferMode mode, EBufferUsage usage);
    ~FDeviceResourceBuffer();

    FDeviceResourceBuffer(const FDeviceResourceBuffer& ) = delete;
    FDeviceResourceBuffer& operator =(const FDeviceResourceBuffer& ) = delete;

    bool Available() const { return nullptr != _deviceAPIDependantBuffer; }
    const PDeviceAPIDependantResourceBuffer& DeviceAPIDependantBuffer() const { return _deviceAPIDependantBuffer; }

    size_t Count() const { return _count; }
    size_t Stride() const { return bitstride_type::Get(_strideModeUsage); }
    size_t SizeInBytes() const { return Stride() * _count; }

    EBufferMode Mode() const { return static_cast<EBufferMode>(bitmode_type::Get(_strideModeUsage)); }
    EBufferUsage Usage() const { return static_cast<EBufferUsage>(bitusage_type::Get(_strideModeUsage)); }

    void Resize(size_t count);

    void Create(IDeviceAPIEncapsulator *device, const FDeviceResource *resource, FDeviceAPIDependantResourceBuffer *buffer);
    PDeviceAPIDependantResourceBuffer Destroy(IDeviceAPIEncapsulator *device, const FDeviceResource *resource);

    void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count);
    void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count);

    template <typename T>
    void GetData(IDeviceAPIEncapsulator *device, size_t offset, T *const dst, size_t count) {
        GetData(device, offset, dst, sizeof(T), count);
    }

    template <typename T>
    void SetData(IDeviceAPIEncapsulator *device, size_t offset, const T *src, size_t count) {
        SetData(device, offset, src, sizeof(T), count);
    }

    void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceResourceBuffer *psource);
    void CopySubPart(   IDeviceAPIEncapsulator *device, size_t dstOffset,
                        const FDeviceResourceBuffer *psource, size_t srcOffset,
                        size_t length );

    size_t HashValue() const;
    bool Match(const FDeviceAPIDependantResourceBuffer& entity) const;

private:
    typedef Meta::TBit<u32>::TFirst<2>::type bitusage_type;
    typedef Meta::TBit<u32>::TAfter<bitusage_type>::TField<4>::type bitmode_type;
    typedef Meta::TBit<u32>::TAfter<bitmode_type>::FRemain::type bitstride_type;

    u32 _count;
    u32 _strideModeUsage;
    PDeviceAPIDependantResourceBuffer _deviceAPIDependantBuffer;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class FDeviceAPIDependantResourceBuffer : public FDeviceAPIDependantEntity {
public:
    FDeviceAPIDependantResourceBuffer(   IDeviceAPIEncapsulator *device,
                                        const FDeviceResource *resource,
                                        const FDeviceResourceBuffer *buffer,
                                        const TMemoryView<const u8>& optionalData);
    virtual ~FDeviceAPIDependantResourceBuffer();

    EDeviceResourceType ResourceType() const { return _resourceType; }

    size_t Count() const { return _count; }
    size_t Stride() const { return bitstride_type::Get(_strideModeUsage); }
    size_t SizeInBytes() const { return Stride() * _count; }

    EBufferMode Mode() const { return static_cast<EBufferMode>(bitmode_type::Get(_strideModeUsage)); }
    EBufferUsage Usage() const { return static_cast<EBufferUsage>(bitusage_type::Get(_strideModeUsage)); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) = 0;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) = 0;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const FDeviceAPIDependantResourceBuffer *psource) = 0;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device, size_t dstOffset,
                                const FDeviceAPIDependantResourceBuffer *psource, size_t srcOffset,
                                size_t length ) = 0;

    virtual size_t VideoMemorySizeInBytes() const { return SizeInBytes(); }

private:
    typedef Meta::TBit<u32>::TFirst<2>::type bitusage_type;
    typedef Meta::TBit<u32>::TAfter<bitusage_type>::TField<2>::type bitmode_type;
    typedef Meta::TBit<u32>::TAfter<bitmode_type>::FRemain::type bitstride_type;

    u32 _count;
    u32 _strideModeUsage;
    EDeviceResourceType _resourceType;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
