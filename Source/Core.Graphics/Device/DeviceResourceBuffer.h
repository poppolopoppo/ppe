#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"
#include "Core.Graphics/Device/DeviceResourceType.h"

#include "Core/Meta/BitField.h"

namespace Core {
    template <typename T>
    class MemoryView;
} //!namespace Core

namespace Core {
namespace Graphics {
class IDeviceAPIEncapsulator;
class DeviceResource;
FWD_REFPTR(DeviceAPIDependantResourceBuffer);

//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
enum class BufferMode : u32 {
    None            = 0,
    Read            = 1<<0,
    Write           = 1<<1,
    Discard         = 1<<2,
    DoNotWait       = 1<<3,
    ReadWrite       = Read|Write,
    WriteDiscard    = Write|Discard,
    WriteDoNotWait  = Write|DoNotWait,
};
//----------------------------------------------------------------------------
// http://amd-dev.wpengine.netdna-cdn.com/wordpress/media/2012/10/Ultimate%20Graphics%20Performance%20for%20DirectX%2010%20Hardware%20-%20Develop%202008.pdf
// https://developer.nvidia.com/sites/default/files/akamai/gamedev/files/gdc12/Efficient_Buffer_Management_McDonald.pdf
// - Forever : Immutable
// - Long lived : Default
// - Temporary : Dynamic
// - Constant : Staging
enum class BufferUsage : u32 {
    Default     = 0,    // The CPU updates the resource less than once per frame
    Immutable   ,       // The CPU does not update the resource
    Dynamic     ,       // The CPU updates the resource more than once per frame
    Staging     ,       // The CPU needs to read the resource
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceResourceBuffer {
public:
    DeviceResourceBuffer(size_t stride, size_t count, BufferMode mode, BufferUsage usage);
    ~DeviceResourceBuffer();

    DeviceResourceBuffer(const DeviceResourceBuffer& ) = delete;
    DeviceResourceBuffer& operator =(const DeviceResourceBuffer& ) = delete;

    bool Available() const { return nullptr != _deviceAPIDependantBuffer; }
    const PDeviceAPIDependantResourceBuffer& DeviceAPIDependantBuffer() const { return _deviceAPIDependantBuffer; }

    size_t Count() const { return _count; }
    size_t Stride() const { return bitstride_type::Get(_strideModeUsage); }
    size_t SizeInBytes() const { return Stride() * _count; }

    BufferMode Mode() const { return static_cast<BufferMode>(bitmode_type::Get(_strideModeUsage)); }
    BufferUsage Usage() const { return static_cast<BufferUsage>(bitusage_type::Get(_strideModeUsage)); }

    void Resize(size_t count);

    void Create(IDeviceAPIEncapsulator *device, const DeviceResource *resource, DeviceAPIDependantResourceBuffer *buffer);
    PDeviceAPIDependantResourceBuffer Destroy(IDeviceAPIEncapsulator *device, const DeviceResource *resource);

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

    void CopyFrom(IDeviceAPIEncapsulator *device, const DeviceResourceBuffer *psource);
    void CopySubPart(   IDeviceAPIEncapsulator *device, size_t dstOffset, 
                        const DeviceResourceBuffer *psource, size_t srcOffset,
                        size_t length );

private:
    typedef Meta::Bit<u32>::First<2>::type bitusage_type;
    typedef Meta::Bit<u32>::After<bitusage_type>::Field<2>::type bitmode_type;
    typedef Meta::Bit<u32>::After<bitmode_type>::Remain::type bitstride_type;

    u32 _count;
    u32 _strideModeUsage;
    PDeviceAPIDependantResourceBuffer _deviceAPIDependantBuffer;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
class DeviceAPIDependantResourceBuffer : public DeviceAPIDependantEntity {
public:
    DeviceAPIDependantResourceBuffer(   IDeviceAPIEncapsulator *device,
                                        const DeviceResource *resource,
                                        const DeviceResourceBuffer *buffer,
                                        const MemoryView<const u8>& optionalData);
    virtual ~DeviceAPIDependantResourceBuffer();

    DeviceResourceType ResourceType() const { return _resourceType; }

    size_t Count() const { return _count; }
    size_t Stride() const { return bitstride_type::Get(_strideModeUsage); }
    size_t SizeInBytes() const { return Stride() * _count; }

    BufferMode Mode() const { return static_cast<BufferMode>(bitmode_type::Get(_strideModeUsage)); }
    BufferUsage Usage() const { return static_cast<BufferUsage>(bitusage_type::Get(_strideModeUsage)); }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) = 0;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) = 0;

    virtual void CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantResourceBuffer *psource) = 0;

    virtual void CopySubPart(   IDeviceAPIEncapsulator *device, size_t dstOffset, 
                                const DeviceAPIDependantResourceBuffer *psource, size_t srcOffset, 
                                size_t length ) = 0;

    virtual size_t VideoMemorySizeInBytes() const { return SizeInBytes(); }

private:
    typedef Meta::Bit<u32>::First<2>::type bitusage_type;
    typedef Meta::Bit<u32>::After<bitusage_type>::Field<2>::type bitmode_type;
    typedef Meta::Bit<u32>::After<bitmode_type>::Remain::type bitstride_type;

    u32 _count;
    u32 _strideModeUsage;
    DeviceResourceType _resourceType;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
