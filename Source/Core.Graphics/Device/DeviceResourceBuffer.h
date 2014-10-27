#pragma once

#include "Core.Graphics/Graphics.h"

#include "Core.Graphics/Device/DeviceAPIDependantEntity.h"

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
    None        = 0,
    Read        = 1,
    Write       = 2,
    ReadWrite   = Read|Write,
};
//----------------------------------------------------------------------------
enum class BufferUsage : u32 {
    Default     = 0,
    Immutable   ,
    Dynamic     ,
    Staging     ,
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
                                        DeviceResourceBuffer *owner,
                                        const DeviceResource *resource,
                                        const MemoryView<const u8>& optionalData);
    virtual ~DeviceAPIDependantResourceBuffer();

    const DeviceResourceBuffer *Owner() const { return _owner; }
    const DeviceResource *Resource() const { return _resource; }

    virtual void GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) = 0;
    virtual void SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) = 0;

private:
    DeviceResourceBuffer *_owner;
    const DeviceResource *_resource;
};
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
