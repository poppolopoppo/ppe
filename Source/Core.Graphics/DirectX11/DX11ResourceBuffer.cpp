#include "stdafx.h"

#include "DX11ResourceBuffer.h"

#include "DX11DeviceAPIEncapsulator.h"
#include "DX11ResourceHelpers.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"
#include "Device/DeviceResource.h"
#include "Device/PresentationParameters.h"
#include "Device/Texture/SurfaceFormat.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Maths/ScalarVector.h"
#include "Core/Maths/ScalarRectangle.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11ResourceBuffer::DX11ResourceBuffer(
    IDeviceAPIEncapsulator *device,
    const DeviceResource *resource,
    const DeviceResourceBuffer *buffer,
    const MemoryView<const u8>& optionalData)
:   DeviceAPIDependantResourceBuffer(device, resource, buffer, optionalData) {
    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    ::D3D11_BUFFER_DESC bd;
    ::SecureZeroMemory(&bd, sizeof(bd));
    bd.ByteWidth = checked_cast<UINT>(SizeInBytes());
    bd.StructureByteStride = checked_cast<UINT>(Stride());
    bd.BindFlags = DeviceResourceTypeToDX11BindFlag(resource->ResourceType());
    bd.Usage = BufferUsageToDX11Usage(Usage());
    bd.CPUAccessFlags = BufferModeToDX11CPUAccessFlags(Mode());
    bd.MiscFlags = 0;

    if (optionalData.empty()) {
        AssertRelease(Usage() != BufferUsage::Immutable);

        DX11_THROW_IF_FAILED(device, resource, (
            wrapper->Device()->CreateBuffer(&bd, NULL, _entity.GetAddressOf())
            ));
    }
    else {
        AssertRelease(bd.ByteWidth == optionalData.SizeInBytes());
        Assert(IS_ALIGNED(16, optionalData.Pointer()));

        ::D3D11_SUBRESOURCE_DATA initData;
        ::SecureZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = optionalData.Pointer();

        DX11_THROW_IF_FAILED(device, resource, (
            wrapper->Device()->CreateBuffer(&bd, &initData, _entity.GetAddressOf())
            ));
    }

    Assert(_entity);
    DX11SetDeviceResourceNameIFP(_entity, resource);
}
//----------------------------------------------------------------------------
DX11ResourceBuffer::~DX11ResourceBuffer() {
    ReleaseComRef(_entity);
}
//----------------------------------------------------------------------------
void DX11ResourceBuffer::GetData(IDeviceAPIEncapsulator *device, size_t offset, void *const dst, size_t stride, size_t count) {
    AssertRelease(Usage() == BufferUsage::Staging);

    DX11ResourceGetData(device, _entity.Get(), 0, offset, dst, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void DX11ResourceBuffer::SetData(IDeviceAPIEncapsulator *device, size_t offset, const void *src, size_t stride, size_t count) {

    DX11ResourceSetData(device, _entity.Get(), 0, offset, src, stride, count, Mode(), Usage());
}
//----------------------------------------------------------------------------
void DX11ResourceBuffer::CopyFrom(IDeviceAPIEncapsulator *device, const DeviceAPIDependantResourceBuffer *psource) {

    const DX11ResourceBuffer *psourceDX11 = checked_cast<const DX11ResourceBuffer *>(psource);

    DX11CopyResource(device, _entity.Get(), psourceDX11->Entity());
}
//----------------------------------------------------------------------------
void DX11ResourceBuffer::CopySubPart(
    IDeviceAPIEncapsulator *device, size_t dstOffset,
    const DeviceAPIDependantResourceBuffer *psource, size_t srcOffset, size_t length ) {

    const DX11ResourceBuffer *psourceDX11 = checked_cast<const DX11ResourceBuffer *>(psource);

    const uint3 dstPos(checked_cast<u32>(dstOffset), 0, 0);
    const AABB3u srcBox(uint3(checked_cast<u32>(srcOffset), 0, 0),
                        uint3(checked_cast<u32>(srcOffset + length), 0, 1) );

    DX11CopyResourceSubRegion(device, _entity.Get(), 0, dstPos, psourceDX11->Entity(), 0, srcBox);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_SEGREGATED_DEF(Graphics, DX11ResourceBuffer, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
D3D11_CPU_ACCESS_FLAG BufferModeToDX11CPUAccessFlags(BufferMode value) {
    switch (value)
    {
    case Core::Graphics::BufferMode::None:
        return static_cast<D3D11_CPU_ACCESS_FLAG>(0);
    case Core::Graphics::BufferMode::Read:
        return D3D11_CPU_ACCESS_READ;
    case Core::Graphics::BufferMode::Write:
    case Core::Graphics::BufferMode::WriteDiscard:
    case Core::Graphics::BufferMode::WriteDoNotWait:
        return D3D11_CPU_ACCESS_WRITE;
    case Core::Graphics::BufferMode::ReadWrite:
        return static_cast<D3D11_CPU_ACCESS_FLAG>(D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE);
    default:
        break;
    }
    AssertNotImplemented();
    return static_cast<D3D11_CPU_ACCESS_FLAG>(-1);
}
//----------------------------------------------------------------------------
BufferMode DX11CPUAccessFlagsToBufferMode(D3D11_CPU_ACCESS_FLAG value) {
    switch (size_t(value))
    {
    case D3D11_CPU_ACCESS_FLAG(0):
        return Core::Graphics::BufferMode::None;
    case D3D11_CPU_ACCESS_READ:
        return Core::Graphics::BufferMode::Read;
    case D3D11_CPU_ACCESS_WRITE:
        return Core::Graphics::BufferMode::Write;
    case D3D11_CPU_ACCESS_READ|D3D11_CPU_ACCESS_WRITE:
        return Core::Graphics::BufferMode::ReadWrite;
    }
    AssertNotImplemented();
    return static_cast<Graphics::BufferMode>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_USAGE BufferUsageToDX11Usage(BufferUsage value) {
    switch (value)
    {
    case Core::Graphics::BufferUsage::Default:
        return D3D11_USAGE_DEFAULT;
    case Core::Graphics::BufferUsage::Immutable:
        return D3D11_USAGE_IMMUTABLE;
    case Core::Graphics::BufferUsage::Dynamic:
        return D3D11_USAGE_DYNAMIC;
    case Core::Graphics::BufferUsage::Staging:
        return D3D11_USAGE_STAGING;
    }
    AssertNotImplemented();
    return static_cast<::D3D11_USAGE>(-1);
}
//----------------------------------------------------------------------------
BufferUsage DX11UsageToBufferUsage(D3D11_USAGE value) {
    switch (value)
    {
    case D3D11_USAGE_DEFAULT:
        return Core::Graphics::BufferUsage::Default;
    case D3D11_USAGE_IMMUTABLE:
        return Core::Graphics::BufferUsage::Immutable;
    case D3D11_USAGE_DYNAMIC:
        return Core::Graphics::BufferUsage::Dynamic;
    case D3D11_USAGE_STAGING:
        return Core::Graphics::BufferUsage::Staging;
    }
    AssertNotImplemented();
    return static_cast<Graphics::BufferUsage>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
::D3D11_BIND_FLAG DeviceResourceTypeToDX11BindFlag(DeviceResourceType value) {
    switch (value)
    {
    case Core::Graphics::DeviceResourceType::Constants:
        return D3D11_BIND_CONSTANT_BUFFER;
    case Core::Graphics::DeviceResourceType::Indices:
        return D3D11_BIND_INDEX_BUFFER;
    case Core::Graphics::DeviceResourceType::RenderTarget:
        return D3D11_BIND_RENDER_TARGET;
    case Core::Graphics::DeviceResourceType::ShaderProgram:
        return D3D11_BIND_SHADER_RESOURCE;
    case Core::Graphics::DeviceResourceType::Vertices:
        return D3D11_BIND_VERTEX_BUFFER;

    default:
        AssertNotImplemented();
        throw DeviceEncapsulatorException("DX11: unsupported resource type", nullptr);
    }
}
//----------------------------------------------------------------------------
DeviceResourceType DX11BindFlagToDeviceResourceType(::D3D11_BIND_FLAG value) {
    switch (value)
    {
    case D3D11_BIND_CONSTANT_BUFFER:
        return Core::Graphics::DeviceResourceType::Constants;
    case D3D11_BIND_INDEX_BUFFER:
        return Core::Graphics::DeviceResourceType::Indices;
    case D3D11_BIND_RENDER_TARGET:
        return Core::Graphics::DeviceResourceType::RenderTarget;
    case D3D11_BIND_SHADER_RESOURCE:
        return Core::Graphics::DeviceResourceType::ShaderProgram;
    case D3D11_BIND_VERTEX_BUFFER:
        return Core::Graphics::DeviceResourceType::Vertices;
    default:
        AssertNotImplemented();
    }
    return static_cast<Graphics::DeviceResourceType>(-1);
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
