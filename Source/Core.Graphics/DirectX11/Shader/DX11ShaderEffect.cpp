#include "stdafx.h"

#include "DX11ShaderEffect.h"

#include "DX11ShaderProgram.h"
#include "DirectX11/DX11DeviceAPIEncapsulator.h"
#include "DirectX11/Geometry/DX11VertexDeclaration.h"

#include "Device/DeviceAPI.h"
#include "Device/DeviceEncapsulatorException.h"

#include "Core/Allocator/PoolAllocator-impl.h"
#include "Core/Memory/UniqueView.h"

namespace Core {
namespace Graphics {
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
static HRESULT CreateInputLayout_(
    ::ID3D11Device *device,
    ::ID3DBlob *vertexShaderBlob,
    const DX11VertexDeclaration *vertexDeclaration,
    ComPtr<ID3D11InputLayout>& inputLayout) {
    const MemoryView<const ::D3D11_INPUT_ELEMENT_DESC> layout = vertexDeclaration->Layout();

    return device->CreateInputLayout(
        layout.Pointer(),
        checked_cast<UINT>(layout.size()),
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize(),
        inputLayout.GetAddressOf()
        );
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
DX11ShaderEffect::DX11ShaderEffect(IDeviceAPIEncapsulator *device, ShaderEffect *owner)
:   DeviceAPIDependantShaderEffect(device, owner) {
    const DX11DeviceWrapper *wrapper = DX11GetDeviceWrapper(device);

    const PCShaderProgram& vertexProgram = owner->StageProgram(ShaderProgramType::Vertex);
    if (vertexProgram) {
        Assert(vertexProgram->Available());
        Assert(owner->VertexDeclaration());
        Assert(owner->VertexDeclaration()->Available());

        const DX11VertexDeclaration *dx11VertexDeclaration =
            checked_cast<DX11VertexDeclaration *>(owner->VertexDeclaration()->DeviceAPIDependantDeclaration().get());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(vertexProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            CreateInputLayout_( wrapper->Device(),
                                dx11ShaderProgram->Entity(),
                                dx11VertexDeclaration,
                                _inputLayout)
            ));

        Assert(_inputLayout);
        DX11SetDeviceResourceNameIFP(_inputLayout, owner->VertexDeclaration().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateVertexShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _vertexShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_vertexShader, vertexProgram.get());
    }

    const PCShaderProgram& domainProgram = owner->StageProgram(ShaderProgramType::Domain);
    if (domainProgram) {
        Assert(domainProgram->Available());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(domainProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateDomainShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _domainShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_domainShader, domainProgram.get());
    }

    const PCShaderProgram& hullProgram = owner->StageProgram(ShaderProgramType::Hull);
    if (hullProgram) {
        Assert(hullProgram->Available());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(hullProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateHullShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _hullShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_hullShader, hullProgram.get());
    }

    const PCShaderProgram& geometryProgram = owner->StageProgram(ShaderProgramType::Geometry);
    if (geometryProgram) {
        Assert(geometryProgram->Available());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(geometryProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateGeometryShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _geometryShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_geometryShader, geometryProgram.get());
    }

    const PCShaderProgram& pixelProgram = owner->StageProgram(ShaderProgramType::Pixel);
    if (pixelProgram) {
        Assert(pixelProgram->Available());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(pixelProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreatePixelShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _pixelShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_pixelShader, pixelProgram.get());
    }

    const PCShaderProgram& computeProgram = owner->StageProgram(ShaderProgramType::Compute);
    if (computeProgram) {
        Assert(computeProgram->Available());

        const DX11ShaderProgram *dx11ShaderProgram =
            checked_cast<DX11ShaderProgram *>(computeProgram->DeviceAPIDependantProgram().get());

        DX11_THROW_IF_FAILED(device, owner, (
            wrapper->Device()->CreateComputeShader(
                dx11ShaderProgram->Entity()->GetBufferPointer(),
                dx11ShaderProgram->Entity()->GetBufferSize(),
                NULL,
                _computeShader.GetAddressOf())
            ));

        DX11SetDeviceResourceNameIFP(_computeShader, computeProgram.get());
    }
}
//----------------------------------------------------------------------------
DX11ShaderEffect::~DX11ShaderEffect() {
    ReleaseComRef(_inputLayout);
    ReleaseComRef(_vertexShader);
    ReleaseComRef(_domainShader);
    ReleaseComRef(_hullShader);
    ReleaseComRef(_geometryShader);
    ReleaseComRef(_domainShader);
    ReleaseComRef(_pixelShader);
    ReleaseComRef(_computeShader);
}
//----------------------------------------------------------------------------
SINGLETON_POOL_ALLOCATED_TAGGED_DEF(Graphics, DX11ShaderEffect, );
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
} //!namespace Graphics
} //!namespace Core
